/*---------------------------------------------------------------------------*
    Entrypoint for the more_machines module.

 *---------------------------------------------------------------------------*/

#include "hoshi/mod.h"

#include "starpole.h"
#include "replay.h"
#include "dolphin.h"
#include "netsync.h"
#include "playback.h"

#include "../../music_settings/src/musicsettings.h"

StarpoleExport starpole_export = {0};
MusicExport *music_export;

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

    Hoshi_ExportMod((void *)&starpole_export);

    return;
}

void OnSaveLoaded()
{
    music_export = Hoshi_ImportMod("Music Settings", 1, 0);
}

void OnSceneChange()
{
    TextConsole_Init();

    Dolphin_OnSceneChange();
    Replay_OnSceneChange();
    Test_DisplayString();
    Starpole_DisplayAsset();

    Netsync_OnSceneChange();
}

void OnFrameEnd()
{
    Playback_Listen();
    Netsync_OnFrameEnd();

    // Netsync_UpdateRNGText();
}

void On3DLoadStart()
{
    Replay_On3DLoadStart();
    Netplay_On3DLoadStart();
    Netsync_On3DLoadStart();
}
void On3DLoadEnd()
{
    Audio_InitLog();
    Netplay_On3DLoadEnd();
    Replay_On3DLoadEnd();
    // StressTest_Create();
}

void On3DExit()
{
    Replay_On3DExit();
    Netsync_On3DExit();
}

void On3DPause(int pause_ply)
{
    // Hash_CreateText();
}

void On3DUnpause(int pause_ply)
{
    // Hash_DestroyText();
}

ModDesc mod_desc = {
    .name = "Starpole Communication",
    .version.major = STARPOLE_VERSION_MAJOR,
    .version.minor = STARPOLE_VERSION_MINOR,
    .affects_gameplay = false,
    .OnBoot = OnBoot,
    .OnSaveLoaded = OnSaveLoaded,
    .OnSceneChange = OnSceneChange,
    .On3DLoadStart = On3DLoadStart,
    .On3DLoadEnd = On3DLoadEnd,
    .On3DExit = On3DExit,
    .OnFrameEnd = OnFrameEnd,
    .On3DPause = On3DPause,
    .On3DUnpause = On3DUnpause,
};