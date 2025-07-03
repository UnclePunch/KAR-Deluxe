/*---------------------------------------------------------------------------*
    Entrypoint for the video_settings module.

 *---------------------------------------------------------------------------*/

#include "text.h"
#include "useful.h"
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
int ModSaveSize = sizeof(struct MusicSettingsSave);

void OnBoot(HSD_Archive *archive)
{
    MusicSettings_Init();
    return;
}
void OnSceneChange(HSD_Archive *archive)
{
    return;
}
void OnSaveInit(MusicSettingsSave *save, int req_init)
{
    MusicSettings_SaveInit(save, req_init);
    return;
}
void OnMainMenuLoad()
{
    MainMenu_LoadMusicPrompt();
}
void On3DPause()
{
    MusicChange_Create();
}
void On3DUnpause()
{
    MusicChange_Destroy();
}