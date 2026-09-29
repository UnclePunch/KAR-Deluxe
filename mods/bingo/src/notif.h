#include "text.h"

#define BINGO_NOTIF_JOINT_MOVE (1)
#define BINGO_NOTIF_JOINT_BACKGROUND_FILL (4)
#define BINGO_NOTIF_JOINT_SINGLE_ICON (6)
#define BINGO_NOTIF_JOINT_MULTI_ICON (7)
#define BINGO_NOTIF_JOINT_SINGLE_DIGIT (11)
#define BINGO_NOTIF_JOINT_DOUBLE_DIGIT (13)
#define BINGO_NOTIF_JOINT_TEXT (16)

#define BINGO_NOTIF_PARAM_TIMER (120)

typedef struct BingoNotifData
{
    s16 ply;
    s16 progress;
    BingoGoal *goal;
    int timer;
    Text *t;
} BingoNotifData;

GOBJ *BingoNotif_Create(BingoGoal *goal, int progress, int ply);
void BingoNotif_Destroy(BingoNotifData *gp);
void BingoNotif_Think(GOBJ *g);
void BingoNotif_GX(GOBJ *g, int pass);