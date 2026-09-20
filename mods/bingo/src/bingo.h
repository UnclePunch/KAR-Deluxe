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

typedef enum BingoActionKind
{
    ACTION_NONE,
    ACTION_HAVE,
    ACTION_USE,
    ACTION_HIT,
    ACTION_KO,
} BingoActionKind;

typedef enum BingoAttackKind
{
    ATTACK_HIT,
    ATTACK_KO,
    ATTACK_NUM,
} BingoAttackKind;

typedef enum BingoTargetKind
{
    TARGET_PLAYER,
    TARGET_MACHINE,
    TARGET_ITEM,
    TARGET_ENEMY,
    TARGET_YAKUMONO,
    TARGET_NUM,
} BingoTargetKind;

typedef enum BingoConditionKind
{
    CONDITION_HIT,
    CONDITION_ITEM,
    CONDITION_POSITION,
    CONDITION_SPEED,
    CONDITION_AIR,
    CONDITION_MACHINE,
    CONDITION_NUM,
} BingoConditionKind;

typedef enum ZoneKind
{
    ZONEKIND_BOOST,
    ZONEKIND_LIFT,
    ZONEKIND_NUM,
} ZoneKind;

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

typedef struct BingoConditionData
{
    int num : 5;                                // number of times to complete this condition
    int is_expires : 1;                         // goal is marked as incomplete if conditions to succeed later become untrue
    BingoConditionKind condition_kind : 3;      // up to 7
    union                                       // 23 bits available for this
    {
        int _size : 23;
        struct
        {
            BingoTargetKind target : 3;    
            union 
            {
                MachineKind machine : 6;
                ItemKind item : 7;
                int enemy : 6;
                int yakumono : 6;
            } target_kind;
            
            BingoAttackKind attack_kind : 2;         // hit or ko

            BingoDamageSource dmg_source : 4;    
            union 
            {
                int player_attack : 7;
                int weapon_kind : 7;
                int map_kind : 7;
            } dmg_source_kind;
        } hit;
        ItemKind item_kind : 7;
        struct
        {
            int is_on_foot : 1;
            int map_area : 22;
        } map;
        struct
        {
            BingoComparisonKind comparison : 2;
            int amt : 21;
        } speed;
        struct
        {
            BingoComparisonKind comparison : 2;
            int frame : 21;
        } air;
        struct
        {
            MachineKind kind : 23;      // VCKIND_NUM can be a stand-in for "any" machine
        } machine;
    };
} BingoConditionData;

typedef struct BingoGoal
{
    u8 condition_num;     // up to 3
    BingoConditionData condition_data[3];
} BingoGoal;

typedef struct BingoCard
{
    BingoGoal goal[BINGO_UI_GRID_SIZE * BINGO_UI_GRID_SIZE];
} BingoCard;

typedef struct BingoGoalProgress
{
    u8 is_goal_complete;
    u8 condition_progress[3];
} BingoGoalProgress;

typedef struct BingoTrackerData
{
    int ply;
    BingoGoalProgress goal[BINGO_UI_GRID_SIZE * BINGO_UI_GRID_SIZE];
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

typedef struct DamageLogObject
{
    HurtKind hurt_kind : 3;
    int ply : 4;
    unsigned int is_airborne : 1;
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
    float progress;
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
#endif