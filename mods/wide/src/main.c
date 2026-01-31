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
void On3DLoadStart()
{
    return;
}
void On3DLoadEnd()
{
    Wide_CreateDebugHUDGObj();
}


ModDesc mod_desc = {
    .name = "Widescreen",
    .author = "UnclePunch",
    .version.major = VERSION_MAJOR,
    .version.minor = VERSION_MINOR,
    .OnBoot = OnBoot,
    .On3DLoadStart = On3DLoadStart,
    .On3DLoadEnd = On3DLoadEnd,
};