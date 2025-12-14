/*---------------------------------------------------------------------------*
    Entrypoint for the title screen module.

 *---------------------------------------------------------------------------*/

#include "hoshi/mod.h"
#include "hoshi/settings.h"

#include "title.h"

void OnBoot()
{
    // apply patches
    Title_ApplyPatches();

    return;
}

ModDesc mod_desc = {
    .name = "KAR Deluxe Title",
    .author = "UnclePunch",
    .version.major = 1,
    .version.minor = 0,
    .OnBoot = OnBoot,
};
