#ifndef BINGO_H
#define BINGO_H

#define BINGO_ASSET_FILENAME "IfBingo"

void Bingo_Init();
void Bingo_On3DLoadStart();
void Bingo_On3DPause(int pause_ply);
void Bingo_On3DUnpause(int pause_ply);
#endif