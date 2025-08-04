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
#include "patches/items.h"
#include "patches/events.h"
#include "patches/machines.h"

char ModName[] = "City Settings";
char ModAuthor[] = "UnclePunch";
char ModVersion[] = "v" STR(VERSION_MAJOR) "." STR(VERSION_MINOR);

CitySettingsSave *ModSave;
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

void OnSaveInit()
{
    CitySettings_SetDefault();
}

void OnSaveLoaded()
{
    CitySettings_SaveLoaded();
}

void OnPlayerSelectLoad()
{
    Machines_DetermineSingleRandom();
}
void On3DLoad()
{
    Box_CheckIfEnabled();
    EventReveal_Do();
}