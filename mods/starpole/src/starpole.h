#ifndef STARPOLE_H
#define STARPOLE_H

#include "exi.h"
#include "game.h"

#define STARPOLE_VERSION_MAJOR (1)
#define STARPOLE_VERSION_MINOR (0)

#define STARPOLE_EXI_CHAN 1
#define STARPOLE_EXI_FREQ 5
#define STARPOLE_DEVICE_ID 0x0A000000

typedef enum
{
    STARPOLE_CMD_ID,
    STARPOLE_CMD_TEST,
    STARPOLE_CMD_MATCH,
    STARPOLE_CMD_FRAME,
    STARPOLE_CMD_END,
    
    STARPOLE_CMD_REQMATCH,
    STARPOLE_CMD_REQFRAME,
    STARPOLE_CMD_NUM,
} StarpoleCmd;

typedef struct
{
    char str[128];
} StarpoleDataTest;

typedef struct
{
    int rng_seed;
    u16 frame_size;
    u16 stadium_kind;
    u8 misc[0xac4 - 0xa94];
    PlayerDesc ply_desc[4];
} StarpoleDataMatch;

#pragma pack(push, 1)
typedef struct
{
    int frame_idx;
    int rng_seed;
    u8 ply_num;
    struct
    {
        u8 idx;
        struct
        {
            u8 held;
            s8 stickX;
            s8 stickY;
            s8 substickX;
            s8 substickY;
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

int Starpole_Imm(StarpoleCmd cmd, int args);
int Starpole_DMA(StarpoleBuffer *buf, int size, EXIMode mode);

int Starpole_IsPresent();
void Starpole_Init();

StarpoleDataTest *Test_GetString();
void Test_DisplayString();

#endif
