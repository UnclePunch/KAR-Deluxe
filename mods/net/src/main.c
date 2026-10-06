/*---------------------------------------------------------------------------*
    Entrypoint for the module.

 *---------------------------------------------------------------------------*/

#include "hoshi/mod.h"

#include "net.h"


OptionDesc mod_settings = {
    .name = "Starpole",
    .description = "Not sure yet.",
    .pri = MENUPRI_NORMAL,
    .kind = OPTKIND_SCENE,
    .major_idx = -1,
};

void OnBoot()
{
    Net_Init();
    return;
}

void OnSaveLoaded()
{
    Net_OnSaveLoaded();
}

void OnSceneChange()
{
    Net_OnSceneChange();
}

void OnFrameEnd()
{
    Net_OnFrameEnd();
}

void On3DLoadStart()
{
}
void On3DLoadEnd()
{
}

void On3DExit()
{
}

void On3DPause(int pause_ply)
{
}

void On3DUnpause(int pause_ply)
{
}

ModDesc mod_desc = {
    .name = "Net Testing",
    .version.major = 0,
    .version.minor = 0,
    .affects_gameplay = false,
    .OnBoot = OnBoot,
    .OnSaveLoaded = OnSaveLoaded,
    .OnSceneChange = OnSceneChange,
    .On3DLoadStart = On3DLoadStart,
    .On3DLoadEnd = On3DLoadEnd,
    .On3DExit = On3DExit,
    .OnFrameEnd = OnFrameEnd,
    .On3DPauseStart = On3DPause,
    .On3DUnpause = On3DUnpause,
};