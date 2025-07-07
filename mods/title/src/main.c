/*---------------------------------------------------------------------------*
    Entrypoint for the title screen module.

 *---------------------------------------------------------------------------*/

#include "useful.h"
#include "hsd.h"
#include "preload.h"
#include "scene.h"
#include "inline.h"
#include "audio.h"
#include "hoshi/settings.h"

#include "title.h"

char ModName[] = "KAR Deluxe Title";
char ModAuthor[] = "UnclePunch";
char ModVersion[] = "v1.0";

void OnBoot(HSD_Archive *archive)
{
    // apply patches
    Title_ApplyPatches();

    return;
}
