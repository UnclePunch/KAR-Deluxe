#ifndef STARPOLE_HASH_H
#define STARPOLE_HASH_H

#include "datatypes.h"

#define HASH_SYS    (1 << GAMEPLINK_SYS)
#define HASH_3D     ((1 << GAMEPLINK_RIDER) | (1 << GAMEPLINK_MACHINE) | (1 << GAMEPLINK_ENEMY) | (1 << GAMEPLINK_ITEM))
#define HASH_ALL    (HASH_SYS | HASH_3D)

u32 Hash_GameState(u32 kind);

#endif