#ifndef GAMESETTINGS_STARTUP_H
#define GAMESETTINGS_STARTUP_H

#include "game.h"

typedef enum StartupKind
{
    STARTUP_MOVIE,
    STARTUP_TITLE,
    STARTUP_MAINMENU,
    STARTUP_CITYSELECT,
    STARTUP_AIRRIDESELECT,
    STARTUP_LAN,
} StartupKind;

void Startup_Init();

#endif