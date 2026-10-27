#include "hoshi/mod.h"
#include "hoshi/settings.h"

#include "mirror_mode.h"

#define VERSION_MAJOR 1
#define VERSION_MINOR 0

extern int g_is_mirror;

OptionDesc mod_settings = {
    .name = "Mirror Mode",
    .description = "Flip the course visuals.",
    .on_change = 0,
    .pri = MENUPRI_LOW,
    .kind = OPTKIND_VALUE,
    .val = &g_is_mirror,
    .value_num = 2,
    .value_names = (char *[]){
        "Off",
        "On",
    },
};

void OnBoot()
{
    MirrorMode_Init();
    return;
}

ModDesc mod_desc = {
    .name = "Mirror Mode",
    .author = "UnclePunch",
    .version.major = VERSION_MAJOR,
    .version.minor = VERSION_MINOR,
    .affects_gameplay = false,
    .option_desc = &mod_settings,
    .OnBoot = OnBoot,
};