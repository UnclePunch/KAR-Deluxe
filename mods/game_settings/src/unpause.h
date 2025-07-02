#ifndef GAMESETTINGS_UNPAUSE_H
#define GAMESETTINGS_UNPAUSE_H

#include "game.h"

#define SIGNALGO_ANIMRATE (0.7)
#define SIGNALGO_STARTDELAY 15

typedef struct SignalGoData
{
    HUDElementData hud_data;
    int timer;
    int state;
} SignalGoData;

GOBJ *SignalGo_Create(int ply);
void SignalGo_Anim(GOBJ *g);
void UnpauseDelay_Init();
void UnpauseDelay_On3DStart();

#endif