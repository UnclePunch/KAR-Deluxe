#include "obj.h"
#include "os.h"
#include "hsd.h"
#include <stdio.h>
#include "code_patch/code_patch.h"

int profiler_enable = 0;
int proc_total;
int time_total;
int pre_tick;

void GOBJProc_Start()
{
    if (!profiler_enable)
        return;

    // OSReport("## GObj Proc Begin ##\n\n");
    proc_total = 0;
    time_total = 0;

    return;
}
CODEPATCH_HOOKCREATE(0x80429ee8, "", GOBJProc_Start, "", 0)

void GOBJProc_Pre(void *cb, GOBJ *g)
{
    if (!profiler_enable)
        return;

    pre_tick = OSGetTick();
    return;
}
CODEPATCH_HOOKCREATE(0x80429fc8, "", GOBJProc_Pre, "", 0)

void GOBJProc_Post(void *cb, GOBJ *g)
{
    if (!profiler_enable)
        return;

    static char *p_link_names[] = {
        "SYS",
        "1",
        "CITYEVENTSPAWN",
        "3",
        "CITYITEMSPAWN",
        "CITYMACHINESPAWN",
        "6", // effect probably
        "STAGE",
        "ENEMY",
        "MACHINE",
        "RIDER",
        "11",
        "ENEMY", // dyna blade, tac, meteor, etc
        "ITEM",       // anything in ItemKind
        "PROJECTILE", // bomb, plasma bullet, flame, firework etc
        "SHADOW",
        "EFFECTMODEL",
        "CAMWORLD",
        "18",
        "CAMHUD",
        "20",
        "CAMDEBUG",
        "AUDIO",
        "23",
        "24",
        "25",
        "HUD",
        "PAUSEHUD",
        "CARDCAM", // renders memcard save prompt
    };

    int time = OSGetTick() - pre_tick;
    time_total += time;
    proc_total++;

    if (!(Pad_GetHeld(20) & PAD_BUTTON_DPAD_DOWN))
        return;

    char buffer[2];
    char *p_link_name;
    if (g->p_link <= sizeof(p_link_names) / sizeof(p_link_names[0]))
        p_link_name = p_link_names[g->p_link];
    else
    {
        sprintf(buffer, "%d", g->p_link);
        p_link_name = buffer; 
    }

    OSReport("Func %p with p_link %s completed in %.4fms\n", cb, p_link_name, OSTicksToMilliseconds(time));
    return;
}
CODEPATCH_HOOKCREATE(0x80429fd8, "lwz	3, 0x0014 (25)\n\t"
                                 "lwz	4, 0x0010 (25)\n\t",
                     GOBJProc_Post, "", 0)

void GOBJProc_End()
{
    if (!profiler_enable)
        return;

    OSReport("%d proc's completed in %.4fms\n", proc_total, OSTicksToMilliseconds(time_total));
    // OSReport("## GObj Proc End ##\n\n");

    return;
}
CODEPATCH_HOOKCREATE(0x8042a090, "", GOBJProc_End, "", 0)

void Profiler_Init()
{
    CODEPATCH_HOOKAPPLY(0x80429ee8);
    CODEPATCH_HOOKAPPLY(0x80429fc8);
    CODEPATCH_HOOKAPPLY(0x80429fd8);
    CODEPATCH_HOOKAPPLY(0x8042a090);
}