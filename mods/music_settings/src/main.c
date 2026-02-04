/*---------------------------------------------------------------------------*
    Entrypoint for the music_settings module.

 *---------------------------------------------------------------------------*/

#include "hoshi/settings.h"
#include "hoshi/mod.h"

#include "musicsettings.h"
#include "music_change.h"

#include "../../wide/src/wide_export.h"

WideExport *wide_export;

void OnBoot()
{
    MusicSettings_Init();
    MusicChange_Init();
    return;
}
void OnSceneChange()
{
    return;
}
void OnSaveInit()
{
    MusicSettings_SaveSetDefault();

    // get exports from widescreen mod
    wide_export = (WideExport *)Hoshi_ImportMod("Widescreen", WIDE_VERSION_MAJOR, WIDE_VERSION_MINOR);
    return;
}
void OnSaveLoaded()
{
    MusicSettings_OnSaveLoaded();
    return;
}
void OnMainMenuLoad()
{
    MainMenu_LoadMusicPrompt();
}
void On3DLoad()
{
    MusicChange_On3DLoad();
}
void On3DPause(int pauser_ply)
{
    MusicChange_Create();
}
void On3DUnpause(int pauser_ply)
{
    MusicChange_Destroy();
}

ModDesc mod_desc = {
    .name = "Music Settings",
    .author = "UnclePunch",
    .version.major = VERSION_MAJOR,
    .version.minor = VERSION_MINOR,
    .save_size = sizeof(struct MusicSettingsSave),
    .OnBoot = OnBoot,
    .OnSceneChange = OnSceneChange,
    .OnSaveInit = OnSaveInit,
    .OnSaveLoaded = OnSaveLoaded,
    .OnMainMenuLoad = OnMainMenuLoad,
    .On3DLoadEnd = On3DLoad,
    .On3DPause = On3DPause,
    .On3DUnpause = On3DUnpause,
};