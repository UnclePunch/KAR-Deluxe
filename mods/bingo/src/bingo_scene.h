#ifndef BINGOSCENE_H
#define BINGOSCENE_H

#define DOWNSCALE_WIDTH (640 / 2)
#define DOWNSCALE_HEIGHT (480 / 2)
#define DOWNSCALE_NUM 40

#include "scene.h"

MajorKind Bingo_InstallScene();
void BingoScene_SetBoot();
void BingoScene_OnReset();
void BingoScene_Init();

#endif