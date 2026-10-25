#ifndef BINGOSCENE_H
#define BINGOSCENE_H

#define DOWNSCALE_WIDTH (640 / 4)
#define DOWNSCALE_HEIGHT (480 / 4)
#define DOWNSCALE_NUM (60)

#include "scene.h"

MajorKind Bingo_InstallScene();
void BingoScene_SetBoot();
void BingoScene_OnReset();
void BingoScene_Init();

#endif