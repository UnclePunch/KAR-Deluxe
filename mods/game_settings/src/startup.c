
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
StartupKind startup_kind = STARTUP_MOVIE;
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
    CODEPATCH_REPLACEINSTRUCTION(0x80047750, 0x38600000 | major_kind);
    CODEPATCH_REPLACEINSTRUCTION(0x800477f0, 0x38600000 | major_kind);

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
        FGM_LoadInGameBanks(); // title screen usually calls this, it holds up loading other modes if you dont do it beforehand
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
