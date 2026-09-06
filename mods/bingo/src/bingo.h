#ifndef BINGO_H
#define BINGO_H

#define BINGO_ASSET_FILENAME "IfBingo"
#define BINGO_SIS_INDEX 0
#define BINGO_UI_SCOREBOARD_TEXT_JOINT (3)
#define BINGO_UI_DESCRIPTION_TEXT_JOINT (5)

typedef struct BingoUIData
{
    struct
    {
        struct
        {
            Text *label;
            Text *game_info;
            Text *leaderboard_nums;
            Text *leaderboard_teams;
            Text *leaderboard_scores;
        } scoreboard;

        Text *details;
    } text;
} BingoUIData;

void Bingo_Init();
void Bingo_On3DLoadStart();
void Bingo_On3DLoadEnd();
void Bingo_On3DPause(int pause_ply);
void Bingo_On3DUnpause(int pause_ply);

void BingoCardView_Destroy(BingoUIData *bp);
#endif