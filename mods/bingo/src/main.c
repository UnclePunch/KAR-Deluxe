/*---------------------------------------------------------------------------*
    Entrypoint for the Bingo module.

 *---------------------------------------------------------------------------*/

#include "hoshi/mod.h"

#include "bingo.h"

void OnBoot()
{
    Bingo_Init();
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

void On3DPause(int pause_ply)
{
    Bingo_On3DPause(pause_ply);
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
    .author = "UnclePunch, Kim-Lan",
    .version.major = 1,
    .version.minor = 0,
    .affects_gameplay = false,
    .option_desc = 0,
    .OnBoot = OnBoot,
    .On3DLoadStart = On3DLoadStart,
    .On3DLoadEnd = On3DLoadEnd,
    .On3DPause = On3DPause,
    .On3DUnpause = On3DUnpause,
    .OnPlayerSelectLoad = OnPlayerSelectLoad,
};