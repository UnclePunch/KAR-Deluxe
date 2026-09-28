#ifndef BINGO_H
#define BINGO_H

#include "item.h"
#include "rider.h"
#include "machine.h"

#define BINGO_ASSET_FILENAME "IfBingo"
#define BINGO_SIS_INDEX 0
#define BINGO_UI_SCOREBOARD_TEXT_JOINT (3)
#define BINGO_UI_DESCRIPTION_TEXT_JOINT (5)

#define BINGO_UI_GRID_SIZE (5)

#define BINGO_UI_JOINT_BACKGROUND (1)
#define BINGO_UI_JOINT_BACKGROUND_OUTLINE (2)
#define BINGO_UI_JOINT_BACKGROUND_FILL (3)
#define BINGO_UI_JOINT_SINGLE_ICON (4)
#define BINGO_UI_JOINT_MULTI_ICON (5)
#define BINGO_UI_JOINT_SINGLE_DIGIT (9)
#define BINGO_UI_JOINT_DOUBLE_DIGIT (11)

#define BINGO_NOTIF_JOINT_MOVE (1)
#define BINGO_NOTIF_JOINT_BACKGROUND_FILL (4)
#define BINGO_NOTIF_JOINT_SINGLE_ICON (6)
#define BINGO_NOTIF_JOINT_MULTI_ICON (7)
#define BINGO_NOTIF_JOINT_SINGLE_DIGIT (11)
#define BINGO_NOTIF_JOINT_DOUBLE_DIGIT (13)
#define BINGO_NOTIF_JOINT_TEXT (16)

#define BINGO_NOTIF_PARAM_TIMER (120)

typedef enum BingoUIIconFrame
{
    BINGOICONFRAME_ALLBOXES,
    BINGOICONFRAME_ITEMSTART,
    BINGOICONFRAME_MACHINESTART = 56,
    BINGOICONFRAME_AREASTART = 73,
    BINGOICONFRAME_GRIND = 76,
    BINGOICONFRAME_GLIDE,
    BINGOICONFRAME_BOOSTRING,
    BINGOICONFRAME_HIT,
    BINGOICONFRAME_KO,
    BINGOICONFRAME_PLAYER,
} BingoUIIconFrame;

typedef enum BingoAttackKind
{
    ATTACK_HIT,
    ATTACK_KO,
    ATTACK_NUM,
} BingoAttackKind;

typedef enum BingoGoalKind
{
    GOAL_STATGET,
    GOAL_FOODGET,
    GOAL_ITEMFALLGET,
    GOAL_POSITION,
    GOAL_BREAKBOXKIND,
    GOAL_BREAKBOXWITHATTACK,
    GOAL_BREAKBOXANY,
    GOAL_HITPLAYER,
    GOAL_HITPLAYERWITHATTACK,
    GOAL_KOPLAYER,
    GOAL_DESTROYMACHINE,
    GOAL_RIDEMACHINEKIND,
    GOAL_RAILDISTANCE,
    GOAL_RAILLAND,
    GOAL_GLIDETIME,
    GOAL_BOOSTRING,
    GOAL_NUM,
} BingoGoalKind;

typedef enum ZoneKind
{
    ZONEKIND_BOOST,
    ZONEKIND_LIFT,
    ZONEKIND_NUM,
} ZoneKind;

typedef enum AreaKind
{
    AREAKIND_ISLAND = 7,
    AREAKIND_ROCKFLOWER,
    AREAKIND_CITYHALLFLOWER,
} AreaKind;

typedef enum BingoComparisonKind
{
    COMPARE_LESS,
    COMPARE_EQUAL,
    COMPARE_GREATER,
} BingoComparisonKind;

typedef enum BingoDamageSource
{
    DMGSOURCE_PLAYER,
    DMGSOURCE_WEAPON,
    DMGSOURCE_MAP,
    DMGSOURCE_ANY,
    DMGSOURCE_NUM,
} BingoDamageSource;

typedef enum BingoDifficultyKind
{
    DFCLT_EASY,
    DFCLT_MEDIUM,
    DFCLT_HARD,
    DFCLT_NUM,
} BingoDifficultyKind;

typedef struct BingoGoalDesc
{
    BingoGoalKind kind : 8;
    int num : 8;
    union
    {
        struct
        {
            BingoDifficultyKind difficulty : 8;
        } common[DFCLT_NUM];                        // this is insanely dog-shit lol
        struct
        {
            BingoDifficultyKind difficulty : 8;
            unsigned int num_min : 8;
            unsigned int num_max : 8;
            unsigned int items;
        } stat_get[DFCLT_NUM];
        struct
        {
            BingoDifficultyKind difficulty : 8;
            unsigned int num_min : 8;
            unsigned int num_max : 8;
        } food_get[DFCLT_NUM];
        struct
        {
            BingoDifficultyKind difficulty : 8;
            unsigned int num_min : 8;
            unsigned int num_max : 8;
        } item_fall_get[DFCLT_NUM];
        struct
        {
            BingoDifficultyKind difficulty : 8;
            unsigned int positions;
        } position[DFCLT_NUM];
        struct
        {
            BingoDifficultyKind difficulty : 8;
            unsigned int num_min : 8;
            unsigned int num_max : 8;
        } box_any[DFCLT_NUM];
        struct
        {
            BingoDifficultyKind difficulty : 8;
            unsigned int num_min : 8;
            unsigned int num_max : 8;
        } box_kind[DFCLT_NUM];
        struct
        {
            BingoDifficultyKind difficulty : 8;
            unsigned int num_min : 8;
            unsigned int num_max : 8;
            unsigned int attacks;
        } box_attack[DFCLT_NUM];
        struct
        {
            BingoDifficultyKind difficulty : 8;
            unsigned int num_min : 8;
            unsigned int num_max : 8;
        } hit_ply[DFCLT_NUM];
        struct
        {
            BingoDifficultyKind difficulty : 8;
            unsigned int num_min : 8;
            unsigned int num_max : 8;
            unsigned int attacks;
        } hit_ply_attack[DFCLT_NUM];
        struct
        {
            BingoDifficultyKind difficulty : 8;
            unsigned int num_min : 8;
            unsigned int num_max : 8;
        } ko_ply[DFCLT_NUM];
        struct
        {
            BingoDifficultyKind difficulty : 8;
            unsigned int num_min : 8;
            unsigned int num_max : 8;
        } ko_machine[DFCLT_NUM];
        struct
        {
            BingoDifficultyKind difficulty : 8;
            unsigned int num_min : 8;
            unsigned int num_max : 8;
            unsigned int machines;
        } ride_machine_kind[DFCLT_NUM];
        struct
        {
            BingoDifficultyKind difficulty : 8;
            unsigned int dist : 24;
        } rail_dist[DFCLT_NUM];
        struct
        {
            BingoDifficultyKind difficulty : 8;
            unsigned int num_min : 8;
            unsigned int num_max : 8;
        } rail_land[DFCLT_NUM];
        struct
        {
            BingoDifficultyKind difficulty : 8;
            unsigned int num_min : 8;
            unsigned int num_max : 8;
        } glide_time[DFCLT_NUM];
        struct
        {
            BingoDifficultyKind difficulty : 8;
            unsigned int num_min : 8;
            unsigned int num_max : 8;
        } boost_ring[DFCLT_NUM];
    } param;
    
} BingoGoalDesc;

typedef struct BingoGoal
{
    int ply_completed : 8;
    BingoGoalKind kind : 8;
    BingoDifficultyKind difficulty : 8;
    unsigned int num : 8;
    union
    {
        struct
        {
            int var;
        } common;
        struct
        {
            ItemKind kind : 16;
        } stat_get;
        struct
        {
            ItemKind kind : 16;
        } food_get;
        struct
        {
            AreaKind kind : 8;
        } position;
        struct
        {
            BoxKind kind : 8;
        } box_kind;
        struct
        {
            AttackKind attack : 8;
        } box_attack;
        struct
        {
            AttackKind attack : 8;
        } hit_ply_attack;
        struct
        {
            MachineKind kind;
        } ride_machine_kind;
        struct
        {
            unsigned int dist : 24;
        } rail_dist;
    } param;
} BingoGoal;

typedef struct BingoCard
{
    BingoGoal goal[BINGO_UI_GRID_SIZE * BINGO_UI_GRID_SIZE];
} BingoCard;

typedef struct BingoTrackerData
{
    int ply;
    u8 progress[BINGO_UI_GRID_SIZE * BINGO_UI_GRID_SIZE];
} BingoTrackerData;

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

typedef struct BingoNotifData
{
    int ply;
    BingoGoal *goal;
    int timer;
    Text *t;
} BingoNotifData;

typedef struct DamageLogObject
{
    HurtKind hurt_kind : 3;
    int ply : 4;
    unsigned int is_airborne : 1;
    AttackData attack_data;      
    int kind : 16;               // yakumono kind, item kind, weapon kind, etc
    int state : 8;
    int state2 : 8;
} DamageLogObject;

typedef struct DamageLog
{
    DamageLogObject victim;
    DamageLogObject attacker;
    float kb_mag;
    float dmg;
    int is_ko;
} DamageLog;

typedef struct ZoneLog
{
    int ply : 8;
    int idx : 16;
    ZoneKind kind : 8;
} ZoneLog;

typedef struct RailLog
{
    s8 ply;
    u8 idx;
    u8 is_airborne;
    float progress;
    u16 status;
    u16 status2;
} RailLog;

typedef struct AreaBound
{
    Vec3 pos;
    Vec3 size;
} AreaBound;

#define MAKE_AREA_BOUND(x_min, x_max, y_min, y_max, z_min, z_max)    \
    (AreaBound){                                                     \
        .pos = {                                                     \
            ((x_min) + (x_max)) / 2.0f,                              \
            ((y_min) + (y_max)) / 2.0f,                              \
            ((z_min) + (z_max)) / 2.0f                               \
        },                                                           \
        .size = {                                                    \
            (x_max) - (x_min),                                       \
            (y_max) - (y_min),                                       \
            (z_max) - (z_min)                                        \
        }                                                            \
    }

void BingoMode_GenerateGoal(BingoGoal *gd, BingoDifficultyKind difficulty);
void BingoMode_InitGoal(BingoGoal *gd, BingoGoalDesc *desc, int difficulty_idx);
int BingoMode_CheckDuplicateGoal(BingoGoal *this, BingoGoal *that);
int Bingo_UpdateProgress(int ply, BingoGoal *gd, u8 progress);

void Bingo_GetDescriptionForGoal(BingoGoal *gd, char *out);

void Bingo_Init();
void Bingo_OnPlayerSelectLoad();
void Bingo_On3DLoadStart();
void Bingo_On3DLoadEnd();
void Bingo_On3DPause(int pause_ply);
void Bingo_On3DUnpause(int pause_ply);
void BingoInput_Think(GOBJ *r);
void BingoTracker_Think(GOBJ *t);
void BingoTracker_GX(GOBJ *t, int pass);

void BingoUI_Think(GOBJ *g);
void BingoUI_Destroy(BingoUIData *bp);

void Bingo_SetIconForGoal(BingoGoal *goal, JOBJ *icon_j, int single_icon_idx, int multi_icon_idx, int single_digit_idx, int double_digit_idx);

GOBJ *BingoNotif_Create(BingoGoal *goal, int progress, int ply);
void BingoNotif_Destroy(BingoNotifData *gp);
void BingoNotif_Think(GOBJ *g);
void BingoNotif_GX(GOBJ *g, int pass);

void Log_Clear();

#endif