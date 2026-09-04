#ifndef BINGO_H
#define BINGO_H

#define BINGO_ASSET_FILENAME "IfBingo"
#define BINGO_SIS_INDEX 0

void Bingo_Init();
void Bingo_On3DLoadStart();
void Bingo_On3DLoadEnd();
void Bingo_On3DPause(int pause_ply);
void Bingo_On3DUnpause(int pause_ply);

void BingoCardView_Destroy(void *data);
#endif