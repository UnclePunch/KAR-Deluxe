#include "hoshi/mod.h"
#include "hoshi/settings.h"

#include "alt_courses.h"

#define VERSION_MAJOR 1
#define VERSION_MINOR 0

extern ModDesc mod_desc;

void OnBoot()
{
    AltCourses_Init();
    mod_desc.option_desc = AltCourses_CreateMenu();
    return;
}
void OnSceneChange()
{
    AltCourses_ApplyAltStages();

    return;
}
void OnSaveLoaded()
{

    return;
}

ModDesc mod_desc = {
    .name = "Alt Courses",
    .author = "UnclePunch",
    .version.major = VERSION_MAJOR,
    .version.minor = VERSION_MINOR,
    .affects_gameplay = true,
    .option_desc = 0,
    .OnBoot = OnBoot,
    .OnSceneChange = OnSceneChange,
    .OnSaveLoaded = OnSaveLoaded,
};