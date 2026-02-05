#ifndef STARPOLE_H
#define STARPOLE_H

#include "exi.h"
#include "game.h"

#define STARPOLE_VERSION_MAJOR (1)
#define STARPOLE_VERSION_MINOR (0)

#define STARPOLE_EXI_CHAN (EXI_CHANNEL_1)
#define STARPOLE_DEVICE (0)
#define STARPOLE_EXI_FREQ 5
#define STARPOLE_DEVICE_ID 0x0A000000

#define STARPOLE_ASSET_FILE "IfStarpole.dat"
#define STARPOLE_ASSET_SYMBOL "ScInfStarpole_scene_models"

typedef enum
{
    STARPOLE_CMD_ID,
    STARPOLE_CMD_TEST,

    STARPOLE_CMD_MATCH,
    STARPOLE_CMD_FRAME,
    STARPOLE_CMD_END,
    
    STARPOLE_CMD_REQMATCH,
    STARPOLE_CMD_REQFRAME,

    STARPOLE_CMD_CHECKPLAYBACK,

    STARPOLE_CMD_NETPLAY,

    STARPOLE_CMD_NUM,    
} StarpoleCmd;

typedef struct
{
    char str[128];
} StarpoleDataTest;

typedef struct
{
    float aspect_mult;
    struct
    {
        int ply;
        char usernames[4][31];
    } netplay;
} StarpoleDataDolphin;

typedef struct
{
    int rng_seed;
    u16 frame_size;
    u16 stage_kind;
    u8 stadium_kind;
    // u8 city_kind;
    // u16 time_seconds;
    // u8 is_enable_events : 1;
    // u8 padding : 4;
    // u8 tempo : 2;
    u8 misc[0xac4 - 0xa94];
    struct 
    {
        s8 ply_stats[5][9];
        u8 is_bike[5];
        u8 machine_kind[5];
    } stadium;
    PlayerDesc ply_desc[4];
} StarpoleDataMatch;

#pragma pack(push, 1)
typedef struct
{
    int frame_idx;
    u32 rng_seed;
    u32 hash;
    u8 ply_num;
    struct
    {
        u8 idx;
        struct
        {

            u16 held;
            s8 stickX;
            s8 stickY;
            s8 substickX;
            s8 substickY;
            u8 trigger;
        } input;
        // int rd_state;
        // MachineKind machine_kind;
        // Vec3 pos;
    }ply[4];
} StarpoleDataFrame;
#pragma pack(pop)

typedef struct
{
    union 
    {
        StarpoleDataTest test;
        StarpoleDataMatch match;
        StarpoleDataFrame frame;
    };
} StarpoleBuffer;

typedef struct
{
    Text *t;
    float y;
} TextConsole;

int Starpole_Imm(StarpoleCmd cmd, int args);
int Starpole_DMA(StarpoleBuffer *buf, int size, EXIMode mode);

int Starpole_IsPresent();
void Starpole_Init();
void Starpole_LoadAsset();
void Starpole_DisplayAsset();

void TextConsole_Init();
void TextConsole_AddString(float x, float y, char *fmt, ...);

StarpoleDataTest *Test_GetString();
void Test_DisplayString();

#endif
