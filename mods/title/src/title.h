#ifndef TITLE_H
#define TITLE_H

#include "game.h"

typedef struct TitleData
{
    TitleScreenData title_data;
    JOBJSet **set;
} TitleData;

void Title_ApplyPatches();

#endif