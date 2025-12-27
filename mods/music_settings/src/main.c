/*---------------------------------------------------------------------------*
    Entrypoint for the music_settings module.

 *---------------------------------------------------------------------------*/

#include "hoshi/settings.h"
#include "hoshi/mod.h"

#include "../../more_machines/src/machines.h"

#include "musicsettings.h"
#include "music_change.h"

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