#include "os.h"
#include "hsd.h"
#include "preload.h"
#include "scene.h"
#include "inline.h"
#include "audio.h"
#include "obj.h"
#include "game.h"

#include "hoshi/mod.h"

#include "code_patch/code_patch.h"

void Startup_Init()
{
    // change bootup scene to this
    MajorKind major_kind = MJRKIND_CITY;
    
    // change scene
    Scene_SetNextMajor(major_kind);

    MainMenu_InitAllVariables();

    // set city trial flag and skip tutorial video
    GameData *gd = Gm_GetGameData();
    gd->main_menu.is_in_submenu = 1;
    gd->main_menu.top_menu = MAINMENU_TOPMENU_CITY;
    gd->main_menu.submenu_kind = MAINMENU_TOPMENU_CITY;
    gd->main_menu.major_kind = major_kind;
    gd->city.mode = CITYMODE_TRIAL;
    gd->is_showed_tutorial_city = 1;

}

void OnBoot()
{

    // prevent returning to main menu from city select ply
    CODEPATCH_REPLACEINSTRUCTION(0x80038914, 0x48000040);

    // return to city select after city trial
    CODEPATCH_REPLACEINSTRUCTION(0x8003fe10, 0x38000000 | MJRKIND_CITY);

    // return to city select on reboot
    CODEPATCH_REPLACECALL(0x800064c8, Startup_Init);

    return;
}

void OnSaveLoaded()
{
    // disable save minor code from changing scene
    CODEPATCH_REPLACEINSTRUCTION(0x80047754, 0x60000000);
    CODEPATCH_REPLACEINSTRUCTION(0x800477f4, 0x60000000);

    Startup_Init();
}

void OnSceneChange()
{

}

void OnFrameEnd()
{

}

void On3DLoadStart()
{

}
void On3DLoadEnd()
{

}

void On3DExit()
{

}

void On3DPause(int pause_ply)
{

}

void On3DUnpause(int pause_ply)
{

}

ModDesc mod_desc = {
    .name = "KAR Lite",
    .version.major = 1,
    .version.minor = 0,
    .affects_gameplay = false,
    .OnBoot = OnBoot,
    .OnSaveLoaded = OnSaveLoaded,
    .OnSceneChange = 0,
    .On3DLoadStart = 0,
    .On3DLoadEnd = 0,
    .On3DExit = 0,
    .OnFrameEnd = 0,
    .On3DPause = 0,
    .On3DUnpause = 0,
};