#ifndef GAMESETTINGS_INTANGAFTERKO_H
#define GAMESETTINGS_INTANGAFTERKO_H

#include "game.h"

#define INTANG_TIME (2 * 60)

void IntangAfterKO_On3DLoad();
void IntangAfterKO_Init();
void Rider_ApplyWalkIntang(GOBJ *r);

#endif