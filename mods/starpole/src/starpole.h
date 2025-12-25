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
    GroundKind gr_kind;
    PlayerDesc ply_desc[4];
} StarpoleDataMatch;

typedef struct
{
    int frame_idx;
    int rng_seed;
    int ply_num;
    struct
    {
        int idx;
        struct
        {
            Vec2 lstick;
            Vec2 rstick;
            int buttons;
        } input;
        int rd_state;
        MachineKind machine_kind;
        Vec3 pos;
    }ply[4];
} StarpoleDataFrame;

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
