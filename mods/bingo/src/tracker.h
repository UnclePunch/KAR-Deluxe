typedef struct RideLog
{
    u8 head;
    u8 arr[5];
} RideLog;

typedef struct StatLog
{
    int glide_frames;
    RideLog ride_log[VCKIND_NUM];        // array of the of the machine instances this player has rode
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
void Bingo_UpdateStats(BingoTrackerData *tp);