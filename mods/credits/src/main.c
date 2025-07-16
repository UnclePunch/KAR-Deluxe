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
#include "hoshi/settings.h"
#include "fst/fst.h"

#include "credits.h"

char ModName[] = "KAR Deluxe Credits";
char ModAuthor[] = "UnclePunch";
char ModVersion[] = "v1.0";

OptionDesc ModSettings = {
    .name = "Credits",
    .description = "View the credits for KAR Deluxe.",
    .pri = MENUPRI_VERYLOW,
    .kind = OPTKIND_SCENE,
    .major_idx = -1,
};

void OnBoot(HSD_Archive *archive)
{
    // install the credits scene
    ModSettings.major_idx = Credits_Init(); // install and set index

    return;
}
