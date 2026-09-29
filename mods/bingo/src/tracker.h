typedef struct StatLog
{
    int glide_frames;
} StatLog;

typedef struct BingoTrackerData
{
    int ply;
    StatLog stats;
    u8 progress[BINGO_UI_GRID_SIZE * BINGO_UI_GRID_SIZE];
} BingoTrackerData;

void BingoTracker_Create();
void BingoTracker_Think(GOBJ *t);
void BingoTracker_GX(GOBJ *t, int pass);
int Bingo_UpdateProgress(BingoTrackerData *tp, int goal_idx);