/*---------------------------------------------------------------------------*
    Entrypoint for the video_settings module.

 *---------------------------------------------------------------------------*/

#include "text.h"
#include "os.h"
#include "hsd.h"
#include "preload.h"
#include "scene.h"
#include "inline.h"
#include "audio.h"
#include "hoshi/settings.h"

#include "musicsettings.h"
#include "music_change.h"

char ModName[] = "Music Settings";
char ModAuthor[] = "UnclePunch";
char ModVersion[] = "v" STR(VERSION_MAJOR) "." STR(VERSION_MINOR);

MusicSettingsSave *ModSave = 0;
int ModSaveSize = sizeof(struct MusicSettingsSave);

void OnBoot(HSD_Archive *archive)
{
    MusicSettings_Init();
    MusicChange_Init();
    return;
}
void OnSceneChange(HSD_Archive *archive)
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
void On3DPause()
{
    MusicChange_Create();
}
void On3DUnpause()
{
    MusicChange_Destroy();
}