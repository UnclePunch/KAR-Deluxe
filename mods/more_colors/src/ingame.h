#ifndef MORECOLORS_H_INGAME
#define MORECOLORS_H_INGAME

#include "datatypes.h"
struct RdKirbyOverload
{
    char *override_symbol;
    u8 mat_indices_num;
    u8 mat_indices[9];
};

void Game_Init();

#endif