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

ModCredit *mod_credits[] = {
    &(ModCredit) {
        .role = "Project Lead",
        .names = (char *[]){ "UnclePunch", NULL },
    },
    &(ModCredit) {
        .role = "Logo",
        .names = (char *[]){ "TacoTikiTaco", NULL },
    },
    &(ModCredit) {
        .role = "Playtester",
        .names = (char *[]){ "heynow", "Jas5250", "TacoTikiTaco", NULL },
    },
    &(ModCredit) {
        .role = "HSDRaw, MexTK",
        .names = (char *[]){ "Ploaj", NULL },
    },
    &(ModCredit) {
        .role = "Special Thanks",
        .names = (char *[]){ "Eternalll", "Cheetaboy", NULL },
    },
    NULL,
};

ModDesc mod_desc = {
    .name = "Kirby Air Ride Deluxe",
    .author = "UnclePunch",
    .version.major = 1,
    .version.minor = 0,
    .credits = mod_credits,
    .OnBoot = OnBoot,
};
