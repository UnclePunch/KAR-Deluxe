#include "datatypes.h"
#include "hurt.h"

typedef enum ZoneKind
{
    ZONEKIND_BOOST,
    ZONEKIND_LIFT,
    ZONEKIND_NUM,
} ZoneKind;

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

void Log_Clear();
void BingoLog_Init();