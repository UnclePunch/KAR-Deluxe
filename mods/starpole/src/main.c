/*---------------------------------------------------------------------------*
    Entrypoint for the more_machines module.

 *---------------------------------------------------------------------------*/

#include "hoshi/mod.h"

#include "starpole.h"
#include "replay.h"
#include "netplay.h"

OptionDesc mod_settings = {
    .name = "Starpole",
    .description = "Not sure yet.",
    .pri = MENUPRI_NORMAL,
    .kind = OPTKIND_SCENE,
    .major_idx = -1,
};

void OnBoot()
{
    Starpole_Init();
    Replay_OnBoot();
    Netplay_Init();

    return;
}

void OnSceneChange()
{
    TextConsole_Init();

    Test_DisplayString();
    Starpole_DisplayAsset();
}

void On3DLoadStart()
{
    Replay_On3DLoadStart();
    Netplay_OverridePlayerView();
}

void On3DExit()
{
    Replay_On3DExit();
}

ModDesc mod_desc = {
    .name = "Starpole Communication",
    .version.major = STARPOLE_VERSION_MAJOR,
    .version.minor = STARPOLE_VERSION_MINOR,
    .OnBoot = OnBoot,
    .OnSceneChange = OnSceneChange,
    .On3DLoadStart = On3DLoadStart,
    .On3DExit = On3DExit,
};