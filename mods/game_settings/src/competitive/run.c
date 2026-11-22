
#include "text.h"
#include "os.h"
#include "hsd.h"
#include "preload.h"
#include "scene.h"
#include "inline.h"
#include "audio.h"
#include "obj.h"
#include "game.h"

#include "code_patch/code_patch.h"
#include "hoshi/settings.h"

int is_run_enabled = 1;
float run_speed_mult = 3;
float swim_speed_mult = 6;

void Hook_Run(RiderData *rd, float *run_speed)
{
    if (!is_run_enabled)
        return;

    if (rd->input.held & PAD_BUTTON_B)
        *run_speed = *run_speed * run_speed_mult;
}
CODEPATCH_HOOKCREATE(0x801b8f64, "stwu	1, -40 (1)\n\t"
                                    "mflr 0\n\t"
                                    "stw 0, 44 (1)\n\t"
                                    "stw 3, 8 (1)\n\t"
                                    "stfs 0, 12 (1)\n\t"
                                    "stfs 1, 16 (1)\n\t"
                                    "stfs 2, 20 (1)\n\t"
                                    "stfs 3, 24 (1)\n\t"
                                    "mr 3, 31\n\t"
                                    "addi 4, 1, 12\n\t",
                                    Hook_Run, 
                                    "lwz 3, 8 (1)\n\t"
                                    "lfs 0, 12 (1)\n\t"
                                    "lfs 1, 16 (1)\n\t"
                                    "lfs 2, 20 (1)\n\t"
                                    "lfs 3, 24 (1)\n\t"
                                    "lwz 0, 44 (1)\n\t"
                                    "mtlr 0\n\t"
                                    "addi	1, 1, 40 \n\t", 
                                    0)

void Hook_Swim(RiderData *rd, float *swim_speed)
{
    if (!is_run_enabled)
        return;

    if (rd->input.held & PAD_BUTTON_B)
        *swim_speed = *swim_speed * swim_speed_mult;
}
CODEPATCH_HOOKCREATE(0x801b9408, "stwu	1, -40 (1)\n\t"
                                    "mflr 0\n\t"
                                    "stw 0, 44 (1)\n\t"
                                    "stw 3, 8 (1)\n\t"
                                    "stfs 0, 12 (1)\n\t"
                                    "stfs 1, 16 (1)\n\t"
                                    "stfs 2, 20 (1)\n\t"
                                    "stfs 3, 24 (1)\n\t"
                                    "mr 3, 31\n\t"
                                    "addi 4, 1, 12\n\t",
                                    Hook_Swim, 
                                    "lwz 3, 8 (1)\n\t"
                                    "lfs 0, 12 (1)\n\t"
                                    "lfs 1, 16 (1)\n\t"
                                    "lfs 2, 20 (1)\n\t"
                                    "lfs 3, 24 (1)\n\t"
                                    "lwz 0, 44 (1)\n\t"
                                    "mtlr 0\n\t"
                                    "addi	1, 1, 40 \n\t", 
                                    0)

void Run_Init()
{
    CODEPATCH_HOOKAPPLY(0x801b8f64);
    CODEPATCH_HOOKAPPLY(0x801b9408);
}