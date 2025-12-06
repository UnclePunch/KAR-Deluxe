
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

#include "hoshi/settings.h"

char ModName[] = "More Colors";
char ModAuthor[] = "UnclePunch";
char ModVersion[] = "v1.1";

// Callbacks
void OnBoot(HSD_Archive *archive)
{
    Colors_Init("RdKirbyColors.dat");

    return;
}
void OnSceneChange(HSD_Archive *archive)
{
    return;
}

// Data
// UIColor stc_ui_colors[] = {
//     {
//         // pink
//         {255, 82, 168, 255},  // main
//         {191, 62, 126, 255},  // dark
//         {255, 190, 230, 255}, // bright
//         {255, 170, 201, 255}, // in-game hud
//     },
//     {
//         // yellow
//         {255, 203, 50, 255},  // main
//         {191, 152, 38, 255},  // dark
//         {255, 241, 228, 255}, // bright
//         {255, 237, 74, 255},  // in-game hud
//     },
//     {
//         // blue
//         {101, 101, 242, 255}, // main
//         {76, 76, 181, 255},   // dark
//         {221, 230, 239, 255}, // bright
//         {77, 197, 247, 255},  // in-game hud
//     },
//     {
//         // green
//         {0, 198, 0, 255},     // main
//         {0, 143, 0, 255},     // dark
//         {226, 238, 222, 255}, // bright
//         {158, 232, 104, 255}, // in-game hud
//     },
//     {
//         // white
//         {255, 255, 255, 255}, // main
//         {191, 191, 191, 255}, // dark
//         {203, 203, 203, 255}, // bright
//         {232, 232, 232, 255}, // in-game hud
//     },
//     {
//         // purple
//         {178, 76, 203, 255},  // main
//         {133, 57, 152, 255},  // dark
//         {255, 228, 228, 255}, // bright
//         {232, 109, 255, 255}, // in-game hud
//     },
//     {
//         // brown
//         {157, 97, 53, 255},   // main
//         {117, 73, 39, 255},   // dark
//         {255, 243, 241, 255}, // bright
//         {150, 96, 76, 255},   // in-game hud
//     },
//     {
//         // red
//         {216, 63, 63, 255},   // main
//         {163, 48, 48, 255},   // dark
//         {255, 228, 228, 255}, // bright
//         {255, 91, 99, 255},   // in-game hud
//     },
//     {
//         // carbon
//         {60, 60, 60, 255},    // main
//         {37, 37, 37, 255},    // dark
//         {128, 128, 128, 255}, // bright
//         {90, 90, 90, 255},    // in-game hud
//     },
//     {
//         // cherry
//         {248, 150, 112, 255}, // main
//         {184, 107, 83, 255},  // dark
//         {255, 228, 225, 255}, // bright
//         {255, 167, 140, 255}, // in-game hud
//     },
//     {
//         // orange
//         {239, 92, 0, 255},    // main
//         {176, 67, 0, 255},    // dark
//         {255, 200, 110, 255}, // bright
//         {239, 121, 0, 255},   // in-game hud
//     },
//     {
//         // sapphire
//         {0, 81, 247, 255},    // main
//         {0, 60, 184, 255},    // dark
//         {110, 173, 255, 255}, // bright
//         {0, 81, 247, 255},    // in-game hud
//     },
//     {
//         // citrus
//         {248, 248, 80, 255},  // main
//         {166, 166, 53, 255},  // dark
//         {240, 240, 188, 255}, // bright
//         {250, 250, 147, 255}, // in-game hud
//     },
//     {
//         // lavender
//         {140, 24, 181, 255},  // main
//         {91, 15, 117, 255},   // dark
//         {212, 143, 255, 255}, // bright
//         {140, 24, 181, 255},  // in-game hud
//     },
//     {
//         // ivory
//         {76, 43, 29, 255},  // main
//         {12, 7, 4, 255},    // dark
//         {102, 81, 75, 255}, // bright
//         {76, 23, 29, 255},  // in-game hud
//     },
//     {
//         // Light Blue
//         {157, 252, 231, 255}, // main
//         {119, 191, 175, 255}, // dark
//         {229, 248, 244, 255}, // bright
//         {157, 252, 231, 255}, // in-game hud
//     },
//     {
//         // Emerald
//         {103, 252, 149, 255}, // main
//         {78, 191, 114, 255},  // dark
//         {199, 247, 214, 255}, // bright
//         {103, 252, 149, 255}, // in-game hud
//     },
// };
UIColor *stc_ui_colors = 0;
int stc_ui_colors_num = -1; // GetElementsIn(stc_ui_colors);

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

    RdKirbyColors *kirby_colors = (RdKirbyColors *)Archive_GetPublicAddress(&archive, "rdKirbyColors");
    if (!kirby_colors)
    {
        OSReport("MoreColors: %s does not contain symbol %s, aborting.\n", file_name, "rdKirbyColors");
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