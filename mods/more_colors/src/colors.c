
#include "text.h"
#include "os.h"
#include "hsd.h"
#include "preload.h"
#include "scene.h"
#include "inline.h"
#include "audio.h"
#include "game.h"
#include <string.h>

#include "colors.h"
#include "airride.h"
#include "city.h"
#include "menus.h"
#include "ingame.h"
#include "code_patch/code_patch.h"

UIColor *stc_ui_colors = 0;
int stc_ui_colors_num = -1;

// Shared Select Functions
void Select_SetUIColor(JOBJ *j, int joint_idx, int dobj_idx, GXColor color) // function to manually set the color of a UI element
{
    DOBJ *d = JObj_GetDObjIndex(j, joint_idx, dobj_idx);
    d->mobj->mat->diffuse = color;

    AOBJ *a = d->mobj->aobj;
    if (a)
        a->flags |= AOBJ_NO_ANIM;
}
int Select_ColorToAnim(int color_idx)
{
    int *dol_color_indices = (int *)0x804af51c;

    if (color_idx <= 7) // for vanilla colors, use the lookup table in the dol
        return dol_color_indices[color_idx];
    else
        return color_idx;
}
void Select_UpdatePlyBoard(JOBJ *j, int p_kind, int anim_frame, ElementColorDesc *desc)
{
    // set correct ply anim frame
    JObj_SetFrameAndRate(j, anim_frame, 0);

    // check to override with CPU colors
    if (p_kind == 1) // when adjusting a CPU, always use grey for some elements
    {
        static GXColor cpu_col_fill = {178, 178, 178, 255};
        static GXColor cpu_col_border = {168, 168, 168, 255};
        static GXColor cpu_col_under = {188, 188, 188, 255};
        static GXColor cpu_col_top = {188, 188, 188, 255};
        static GXColor cpu_col_bot = {178, 178, 178, 255};

        desc[0].col = &cpu_col_fill;
        desc[1].col = &cpu_col_border;
        desc[2].col = &cpu_col_under;
        desc[3].col = &cpu_col_top;
        desc[4].col = &cpu_col_bot;
    }

    // apply all colors
    while (desc->joint_idx != -1)
    {
        Select_SetUIColor(j, desc->joint_idx, desc->dobj_idx, *desc->col);
        desc++;
    }
}

void Colors_Init(char *file_name)
{
    
    HSD_Archive archive;
    int entrynum = DVDConvertPathToEntrynum(file_name);

    if (entrynum == -1)
    {
        OSReport("MoreColors: %s not found on disc, aborting.\n", file_name);
        return;
    }

    DVDFileInfo dvdinfo;
    DVDFastOpen(entrynum, &dvdinfo);
    DVDClose(&dvdinfo);

    void *buffer = HSD_MemAlloc(dvdinfo.length);
    int out_size;
    File_LoadSync(file_name, buffer, &out_size);   

    Archive_Init(&archive, buffer, dvdinfo.length);

    RdKirbyColors *kirby_colors = (RdKirbyColors *)Archive_GetPublicAddress(&archive, COLORDATA_SYMBOLNAME);
    if (!kirby_colors)
    {
        OSReport("MoreColors: %s does not contain symbol %s, aborting.\n", file_name, COLORDATA_SYMBOLNAME);
        return;
    }

    stc_ui_colors = kirby_colors->colors;
    stc_ui_colors_num = kirby_colors->num;

    Colors_ApplyPatches();
}

// Apply Patches
void Colors_ApplyPatches()
{
    // Generic Select Function
    CODEPATCH_REPLACEFUNC(0x80009630, Select_ColorToAnim); // color index to anim frame

    AirRideSelect_Init();
    CitySelect_Init();
    Game_Init();
    Menus_Init();
}