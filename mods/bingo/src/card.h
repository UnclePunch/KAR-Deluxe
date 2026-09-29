#include "text.h"

#include "bingo.h"

#define BINGO_ASSET_FILENAME "IfBingo"
#define BINGO_SIS_INDEX 0

#define BINGO_UI_SCOREBOARD_JOINT (1)
#define BINGO_UI_SCOREBOARD_TEXT_JOINT (2)
#define BINGO_UI_DESCRIPTION_JOINT (3)
#define BINGO_UI_DESCRIPTION_TEXT_JOINT (4)
#define BINGO_UI_ICONGRID_JOINT (5)

#define BINGO_UI_JOINT_BACKGROUND (1)
#define BINGO_UI_JOINT_BACKGROUND_OUTLINE (2)
#define BINGO_UI_JOINT_BACKGROUND_FILL (3)
#define BINGO_UI_JOINT_SINGLE_ICON (4)
#define BINGO_UI_JOINT_MULTI_ICON (5)
#define BINGO_UI_JOINT_SINGLE_DIGIT (9)
#define BINGO_UI_JOINT_DOUBLE_DIGIT (11)

typedef struct BingoUIData
{
    int ply;
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
    JOBJ *icon_arr[BINGO_UI_GRID_SIZE * BINGO_UI_GRID_SIZE];
} BingoUIData;

typedef struct BingoCursor
{
    s8 x;
    s8 y;
} BingoCursor;

void BingoUI_OnBoot();
void BingoUI_On3DLoadStart();
void BingoUI_Think(GOBJ *g);
void BingoUI_Destroy(BingoUIData *bp);
void BingoUI_DestroyOnPause();

void Bingo_SetIconForGoal(BingoGoal *goal, JOBJ *icon_j, int single_icon_idx, int multi_icon_idx, int single_digit_idx, int double_digit_idx);

void BingoInput_Create();
void BingoInput_Think(GOBJ *r);