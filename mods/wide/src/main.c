/*---------------------------------------------------------------------------*
    Entrypoint for the music_settings module.

 *---------------------------------------------------------------------------*/

#include "hoshi/settings.h"
#include "hoshi/mod.h"

#include "wide.h"
#include "gamehud.h"

void OnBoot()
{
    Wide_Init();
    HUDAdjust_Init();
    return;
}
void OnSceneChange()
{
    Wide_CreateTestGObj();
}
void On3DLoadStart()
{
    Wide_AdjustConstants();
    return;
}
void On3DLoadEnd()
{
    Wide_CreateDebugHUDGObj();
}

extern WideKind wide_kind;

OptionDesc mod_settings = {
    .name = "Aspect Ratio",
    .description = "Adjust the width of the screen.",
    .pri = MENUPRI_LOW,
    .kind = OPTKIND_VALUE,
    .val = (int *)&wide_kind,
    .value_num = 4,
    .value_names = (char *[]){"4:3", "16:9", "16:10", "32:9",},
    .on_change = Wide_OnOptionChange,
};


ModDesc mod_desc = {
    .name = "Widescreen",
    .author = "UnclePunch",
    .version.major = VERSION_MAJOR,
    .version.minor = VERSION_MINOR,
    .option_desc = &mod_settings,
    .OnBoot = OnBoot,
    .OnSceneChange = OnSceneChange,
    .On3DLoadStart = On3DLoadStart,
    .On3DLoadEnd = On3DLoadEnd,
};