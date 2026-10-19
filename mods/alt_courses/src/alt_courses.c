
#include "text.h"
#include "os.h"
#include "camera.h"
#include "hsd.h"
#include "preload.h"
#include "scene.h"
#include "inline.h"
#include "audio.h"
#include "obj.h"
#include "game.h"

#include "alt_courses.h"

#include "fst/fst.h"
#include "code_patch/code_patch.h"
#include "hoshi/settings.h"

AltCourse *g_alt_courses = 0;
int g_alt_course_num;

OptionDesc mod_settings = {
    // Menu
    .name = "Alternate Courses",
    .description = "Change course aesthetics.",
    .pri = MENUPRI_NORMAL,
    .kind = OPTKIND_MENU,
    .menu_ptr = 0,
};

char *AltCourses_FolderNameFromStageKind(StageKind kind)
{
    static char buf[32];

    char *file_name = stc_stagedesc[kind].data_file;
    
    buf[0] = '/';

    // remove extension
    char *extension_ptr = strchr(file_name, '.');
    strncpy(&buf[1], file_name, extension_ptr - file_name);
    buf[1 + (extension_ptr - file_name)] = '\0';

    return buf;
}

int g_char_total;
void AltCourses_LogCourseFileCallback(int entrynum, void *args)
{
    // log its entrynum
    AltCourse *this_alt = args;
    this_alt->alt_entrynums[this_alt->alt_num++] = entrynum;

    // also grab the length of the filename
    char *file_name = FST_GetFilenameFromEntrynum(entrynum);
    char *extension_ptr = strchr(file_name, '.');
    g_char_total += (extension_ptr - file_name) + 1;

}
void AltCourses_CountCourseFileCallback(int entrynum, void *args)
{
    int *num = args;
    (*num)++;
}

// this code is INSANELY overengineered because i really like the idea of not
// having a limit on the amount of alt courses...
OptionDesc *AltCourses_CreateMenu()
{
    // first find how many courses have alt models
    int course_num = 0;
    int dat_total = 0;
    g_char_total = 0;
    for (int i = 0; i < COURSE_NUM; i++)
    {
        char *folder_name = AltCourses_FolderNameFromStageKind(i);

        // count
        int dat_num = 0;
        FST_ForEachInFolder(folder_name, ".dat", false, AltCourses_CountCourseFileCallback, &dat_num);
        if (dat_num == 0)
            continue;

        g_char_total += strlen(folder_name) + 1;
        dat_total += dat_num;
        course_num++;
        
        OSReport("[%d] %d dats found for %s\n", i, dat_num, folder_name);
    }

    g_alt_course_num = course_num;

    if (course_num == 0)
        return 0;
    
    // alloc'ing up front to minimize memory footprint while maintaining no limit on the amount of alt courses
    int course_data_size = (sizeof(AltCourse) * course_num);
    int entrynum_data_size = + (sizeof(u16) * dat_total);
    u8 *course_alloc = HSD_MemAlloc(course_data_size + entrynum_data_size);

    // get pointers to data in this alloc
    g_alt_courses = (AltCourse *)course_alloc;
    course_alloc += course_data_size;
    u16 *entrynum_arr = (u16 *)course_alloc;
    course_alloc += entrynum_data_size;

    OSReport("g_alt_courses: %p\n", g_alt_courses);

    // populate
    course_num = 0;
    for (int i = 0; i < COURSE_NUM; i++)
    {
        char *folder_name = AltCourses_FolderNameFromStageKind(i);

        // count
        int dat_num = 0;
        FST_ForEachInFolder(folder_name, ".dat", false, AltCourses_CountCourseFileCallback, &dat_num);
        if (dat_num == 0)
            continue;

        AltCourse *this_alt = &g_alt_courses[course_num];

        // write stage kind
        this_alt->kind = i;
        this_alt->alt_num = 0;
        this_alt->alt_entrynums = entrynum_arr;
        this_alt->orig_file_path = stc_stagedesc[i].model_file;

        // write alt entrynums
        FST_ForEachInFolder(folder_name, ".dat", false, AltCourses_LogCourseFileCallback, this_alt);

        entrynum_arr += this_alt->alt_num;
        course_num++;
    }

    // alloc menu
    MenuDesc *menu_desc = HSD_MemAlloc(sizeof(MenuDesc) + (course_num * sizeof(OptionDesc *)));
    menu_desc->cursor = 0;
    menu_desc->scroll = 0;
    menu_desc->option_num = course_num;
    menu_desc->prev = 0;

    int option_desc_size = sizeof(OptionDesc) * course_num;               // options desc with an int *val for each
    int option_val_size =  sizeof(int) * course_num;
    int option_name_arr_size = (dat_total + (course_num * 2)) * sizeof(char *); // char **value_names, adding an additional entry for each course for the Original value

    int menu_buf_size = (option_desc_size + option_val_size + option_name_arr_size + g_char_total);   

    // alloc single buffer containing all allocations needed for this menu
    // OSReport("alloc'ing menu buffer of size %d\n", menu_buf_size);
    u8 *menu_buf = HSD_MemAlloc(menu_buf_size);

    // get pointers to all data in this newly alloc'd buffer
    OptionDesc *option_desc = (OptionDesc *)menu_buf;
    menu_buf += option_desc_size;
    
    int *val_ptr = (int *)menu_buf;
    menu_buf += option_val_size;

    char **value_names = (char **)menu_buf;
    menu_buf += option_name_arr_size;

    u8 *string_buf = (u8 *)menu_buf;
    menu_buf += (g_char_total);

    // populate option desc
    for (int i = 0; i < course_num; i++)
    {
        AltCourse *this_alt = &g_alt_courses[i];
        OptionDesc *this_opt_desc = &option_desc[i];

        // write option to menu
        menu_desc->options[i] = this_opt_desc;

        // get stage kind
        StageKind kind = this_alt->kind;

        // write stage name to string buffer
        char *folder_name = AltCourses_FolderNameFromStageKind(kind);
        char *display_name = &folder_name[1];
        strcpy(string_buf, display_name);           // takes null terminator
        this_opt_desc->name = string_buf;           // save to option
        string_buf += strlen(display_name) + 1;     // +1 for null terminator

        // OSReport("writing option name %s\n", display_name);

        // option value pointer
        this_opt_desc->val = val_ptr;
        val_ptr++;

        // option value name array pointer
        this_opt_desc->value_names = value_names;

        // write original as first value
        static char random_text[] = "Random";
        this_opt_desc->value_names[0] = random_text;
        static char original_text[] = "Original";
        this_opt_desc->value_names[1] = original_text;

        // each stage alt
        for (int alt_idx = 0; alt_idx < this_alt->alt_num; alt_idx++)
        {
            // write alt name to string buffer
            char *file_name = FST_GetFilenameFromEntrynum(this_alt->alt_entrynums[alt_idx]);
            char *extension_ptr = strchr(file_name, '.');
            strncpy(string_buf, file_name, extension_ptr - file_name);
            string_buf[extension_ptr - file_name] = '\0';
            this_opt_desc->value_names[alt_idx + 2] = string_buf;
            // OSReport("writing option %s\n", string_buf);
            string_buf += (extension_ptr - file_name) + 1;
        }

        this_opt_desc->value_num = this_alt->alt_num + 2;
        // OSReport("writing option num %d (%p)\n", this_opt_desc->value_num, &this_opt_desc->value_num);

        value_names += this_alt->alt_num + 2;

        // init option valus
        this_opt_desc->kind = OPTKIND_VALUE;
        this_opt_desc->pri = MENUPRI_NORMAL;
        this_opt_desc->min = 0;
        *this_opt_desc->val = 1;        // set it to original
        this_opt_desc->description = 0;
        this_opt_desc->on_change = 0;
    }
    
    // save 
    mod_settings.menu_ptr = menu_desc;
    
    return &mod_settings;
}

void Motion_ParamAdjust_Hook(float fov)
{

    return;
}
CODEPATCH_HOOKCREATE(0, "", Motion_ParamAdjust_Hook, "", 0)

void AltCourses_ApplyAltStages()
{
    MinorKind minor_kind = Scene_GetCurrentMinor();

    if (!(minor_kind == MNRKIND_CITYPLYSELECT || minor_kind == MNRKIND_AIRRIDEPLYSELECT))
        return;

    for (int i = 0; i < g_alt_course_num; i++)
    {
        OptionDesc *option_desc = mod_settings.menu_ptr->options[i];
        AltCourse *alt_course = &g_alt_courses[i];

        int alt_index;

        // handle random
        if ((*option_desc->val) == 0)
            alt_index = HSD_Randi(alt_course->alt_num + 1); // + 1 to include original
        // handle alt
        else
            alt_index = (*option_desc->val) - 1;

        char *file_path;

        // use original
        if (alt_index == 0)
            file_path = alt_course->orig_file_path;
        else
        {
            strcpy(alt_course->alt_file_path, FST_GetFilePathFromEntrynum(alt_course->alt_entrynums[alt_index - 1]));
            file_path = alt_course->alt_file_path;
        }

        // overwrite pointer to this file
        stc_stagedesc[alt_course->kind].model_file = file_path;
    }
}

void AltCourses_Init()
{
    // CODEPATCH_HOOKAPPLY(0);
}
