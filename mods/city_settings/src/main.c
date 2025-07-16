/*---------------------------------------------------------------------------*
    Entrypoint for the city_settings module.

 *---------------------------------------------------------------------------*/

#include "text.h"
#include "os.h"
#include "hsd.h"
#include "preload.h"
#include "scene.h"
#include "inline.h"
#include "audio.h"

#include "citysettings.h"
#include "patches/around_world.h"

char ModName[] = "City Settings";
char ModAuthor[] = "UnclePunch";
char ModVersion[] = "v" STR(VERSION_MAJOR) "." STR(VERSION_MINOR);
int ModSaveSize = sizeof(struct CitySettingsSave);

void OnBoot(HSD_Archive *archive)
{
    CitySettings_Init(); // overload the city settings scene with our own code

    return;
}
void OnSceneChange(HSD_Archive *archive)
{
    return;
}
void OnSaveInit(CitySettingsSave *save, int req_init)
{
    CitySettings_SaveInit(save, req_init);
}

void OnPlayerSelectLoad()
{
}
void On3DLoad()
{
}