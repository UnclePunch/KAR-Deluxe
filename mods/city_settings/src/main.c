/*---------------------------------------------------------------------------*
    Entrypoint for the city_settings module.

 *---------------------------------------------------------------------------*/

#include "hoshi/mod.h"

#include "citysettings.h"
#include "patches/around_world.h"
#include "patches/items.h"
#include "patches/events.h"
#include "patches/machines.h"

void OnBoot()
{
    CitySettings_Init(); // overload the city settings scene with our own code

    return;
}
void OnSceneChange()
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

ModDesc mod_desc = {
    .name = "City Settings",
    .author = "UnclePunch",
    .version.major = VERSION_MAJOR,
    .version.minor = VERSION_MINOR,
    .save_size = sizeof(struct CitySettingsSave),
    .OnBoot = OnBoot,
    .OnSceneChange = OnSceneChange,
    .OnSaveInit = OnSaveInit,
    .OnSaveLoaded = OnSaveLoaded,
    .OnPlayerSelectLoad = OnPlayerSelectLoad,
    .On3DLoad = On3DLoad,
};