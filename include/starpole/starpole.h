#ifndef STARPOLE_H
#define STARPOLE_H

#include "exi.h"
#include "game.h"
#include "hoshi/mod.h"

#define STARPOLE_VERSION_MAJOR (1)
#define STARPOLE_VERSION_MINOR (0)

typedef struct
{
    float aspect_mult;
    struct
    {
        int is;
        int ply;
        int rng_seed;
        char usernames[4][31];
    } netplay;
} StarpoleDataDolphin;

typedef struct StarpoleExport
{
    StarpoleDataDolphin *dolphin_data;
} StarpoleExport;

#endif
