/*---------------------------------------------------------------------------*
    Entrypoint for the KAR Deluxe Credits module.

 *---------------------------------------------------------------------------*/

#include "hoshi/mod.h"

#include "credits.h"

OptionDesc mod_settings = {
    .name = "Credits",
    .description = "View the credits for KAR Deluxe.",
    .pri = MENUPRI_VERYLOW,
    .kind = OPTKIND_SCENE,
    .major_idx = -1,
};

void OnBoot()
{
    // install the credits scene
    mod_settings.major_idx = Credits_Init(); // install and set index

    return;
}

ModDesc mod_desc = {
    .name = "KAR Deluxe Credits",
    .author = "UnclePunch",
    .version.major = 1,
    .version.minor = 0,
    .option_desc = &mod_settings,
    .OnBoot = OnBoot,
};