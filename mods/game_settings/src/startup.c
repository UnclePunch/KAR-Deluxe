
#include "text.h"
#include "os.h"
#include "hsd.h"
#include "preload.h"
#include "scene.h"
#include "inline.h"
#include "audio.h"
#include "obj.h"
#include "game.h"

#include "code_patch/code_patch.h"
#include "startup.h"

// Startup Behavior
StartupKind startup_kind = STARTUP_CITYSELECT;
static u8 startup_scenes[] = {
    MJRKIND_TITLE, // opening movie
    MJRKIND_TITLE, // title screen
    MJRKIND_MENU,  // main menu
    MJRKIND_CITY,  // city trial select screen
    MJRKIND_AIR,   // air ride select screen
    MJRKIND_LAN,   // LAN
};
void Startup_Init()
{
    // change bootup scene to this
    MajorKind major_kind = startup_scenes[startup_kind];

    // disable save minor code from changing scene
    CODEPATCH_REPLACEINSTRUCTION(0x80047754, 0x60000000);
    CODEPATCH_REPLACEINSTRUCTION(0x800477f4, 0x60000000);
    
    // change scene
    Scene_SetNextMajor(major_kind);

    switch (startup_kind)
    {
    case (STARTUP_MOVIE):
    {
        break;
    }
    case (STARTUP_TITLE):
    {
        GameData *gd = Gm_GetGameData();
        // gd->titlescreen_is_skip_op = 1;

        // adjust instruction to skip opening movie upon leaving the memcard scene
        CODEPATCH_REPLACEINSTRUCTION(0x80047748, 0x38600001);
        CODEPATCH_REPLACEINSTRUCTION(0x80047784, 0x38600001);

        break;
    }
    case (STARTUP_MAINMENU):
    {
        // do something to fix the text in top left
        MainMenu_InitAllVariables();
        break;
    }
    case (STARTUP_CITYSELECT):
    {
        MainMenu_InitAllVariables();

        // set city trial flag and skip tutorial video
        GameData *gd = Gm_GetGameData();
        gd->main_menu.is_in_submenu = 1;
        gd->main_menu.top_menu = MAINMENU_TOPMENU_CITY;
        gd->main_menu.submenu_kind = MAINMENU_TOPMENU_CITY;
        gd->main_menu.major_kind = major_kind;
        gd->city.mode = CITYMODE_TRIAL;
        gd->is_showed_tutorial_city = 1;

        // debug code to boot into city trial
        if (0)
        {
            static PlayerDesc hmn_desc = {
                .p_kind = PKIND_HMN,
                .rider_kind = RDKIND_KIRBY,
                .is_bike = 0,
                .machine_kind = VCKIND_COMPACT,
                .color = 0,
                .rumble = 1,
                .ply = 0,
                .x7 = -1,
                .cpu_level = 8,
                .x9 = -1,
                .xa = 0,
                .xb = 0,
                .xc = 0,
                .x10 = 0,
                .x14 = 0,
                .x18 = 0,
                .x1c = 0,
                .x20 = 0,
                .x24 = 0,
                .x28 = 0,
                .x2c = 0,
            };
            memcpy(&gd->ply_desc[0], &hmn_desc, sizeof(PlayerDesc));
            gd->ply_view_desc[0].flag = PLYCAM_ON;
            gd->time_seconds = 5 * 60;
            gd->city.x398 = 1;
        }

        // gd->city.mode = CITYMODE_STADIUM;


        break;

    }
    case (STARTUP_AIRRIDESELECT):
    {
        MainMenu_InitAllVariables();

        // set air ride mode and skip tutorial video
        GameData *gd = Gm_GetGameData();
        gd->main_menu.is_in_submenu = 1;
        gd->main_menu.top_menu = MAINMENU_TOPMENU_AIRRIDE;
        gd->main_menu.submenu_kind = MAINMENU_TOPMENU_AIRRIDE;
        gd->main_menu.major_kind = major_kind;
        gd->airride_mode = AIRRIDEMODE_RACE;
        gd->is_showed_tutorial_airride = 1;
        break;
    }
    case (STARTUP_LAN):
    {
        MainMenu_InitAllVariables();
        GameData *gd = Gm_GetGameData();
        gd->main_menu.top_menu = MAINMENU_TOPMENU_LAN;
        break;
    }
    }
}
