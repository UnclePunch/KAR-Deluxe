/*---------------------------------------------------------------------------*
    Entrypoint for the Bingo module.

 *---------------------------------------------------------------------------*/

#include "hoshi/mod.h"

#include "bingo.h"

#include "wide/wide.h"

WideExport *g_wide_export = 0;

void OnBoot()
{
    Bingo_Init();
    return;
}

void OnSaveLoaded()
{
    g_wide_export = Hoshi_ImportMod("Widescreen", 1, 0);
    return;
}

void On3DLoadStart()
{
    Bingo_On3DLoadStart();
    return;
}

void On3DLoadEnd()
{
    Bingo_On3DLoadEnd();
    return;
}

void On3DPauseStart(int pause_ply)
{
    Bingo_On3DPauseStart(pause_ply);
    return;
}

void On3DUnpause(int pause_ply)
{
    Bingo_On3DUnpause(pause_ply);
    return;
}

void OnPlayerSelectLoad()
{
    Bingo_OnPlayerSelectLoad();
    return;
}

ModDesc mod_desc = {
    .name = "Bingo",
    .author = "UnclePunch",
    .version.major = 1,
    .version.minor = 0,
    .affects_gameplay = false,
    .option_desc = 0,
    .OnBoot = OnBoot,
    .OnSaveLoaded = OnSaveLoaded,
    .On3DLoadStart = On3DLoadStart,
    .On3DLoadEnd = On3DLoadEnd,
    .On3DPauseStart = On3DPauseStart,
    .On3DUnpause = On3DUnpause,
    .OnPlayerSelectLoad = OnPlayerSelectLoad,
};