#ifndef BINGO_H
#define BINGO_H

#include "item.h"
#include "rider.h"
#include "machine.h"

#define BINGO_UI_GRID_SIZE (5)

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

typedef enum BingoGoalSFXKind
{
    GOALSFX_DOWN,
    GOALSFX_UP,
    GOALSFX_COMPLETE,
    GOALSFX_NONE,
} BingoGoalSFXKind;

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

void Bingo_GetDescriptionForGoal(BingoGoal *gd, char *out);
void Bingo_UpdateIconProgress(BingoGoal *goal, int progress, JOBJ *icon_j, int progres_joint_idx);

void Bingo_Init();
void Bingo_OnPlayerSelectLoad();
void Bingo_On3DLoadStart();
void Bingo_On3DLoadEnd();
void Bingo_On3DPause(int pause_ply);
void Bingo_On3DUnpause(int pause_ply);
void BingoInput_Think(GOBJ *r);
void BingoTracker_Think(GOBJ *t);
void BingoTracker_GX(GOBJ *t, int pass);



#endif