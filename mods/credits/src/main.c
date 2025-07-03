/*---------------------------------------------------------------------------*
    Entrypoint for the city_settings module.

 *---------------------------------------------------------------------------*/

#include "text.h"
#include "useful.h"
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
MenuDesc ModMenu = {
    .name = "Credits",
    .option_num = 1,
    .pri = MENUPRI_VERYLOW,
    .options = (OptionDesc[]){
        {
            .name = "View Credits",
            .kind = OPTKIND_SCENE,
            .major_idx = -1,
        },
    },
};

void OnBoot(HSD_Archive *archive)
{
    // install the credits scene
    ModMenu.options[0].major_idx = Credits_Init(); // install and set index

    return;
}
