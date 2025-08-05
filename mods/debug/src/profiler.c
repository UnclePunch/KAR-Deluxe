#include "obj.h"
#include "os.h"
#include "code_patch/code_patch.h"

int proc_total;
int time_total;
int pre_tick;

void GOBJProc_Start()
{
    OSReport("## GObj Proc Begin ##\n\n");
    proc_total = 0;
    time_total = 0;

    return;
}
CODEPATCH_HOOKCREATE(0x80429ee8, "", GOBJProc_Start, "", 0)

void GOBJProc_Pre(void *cb, GOBJ *g)
{
    pre_tick = OSGetTick();
    return;
}
CODEPATCH_HOOKCREATE(0x80429fc8, "", GOBJProc_Pre, "", 0)

void GOBJProc_Post(void *cb, GOBJ *g)
{
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
        "EVENTACTOR", // dyna blade, tac, meteor, etc
        "ITEM",       // anything in ItemKind
        "PROJECTILE", // bomb, plasma bullet, flame, firework etc
        "15",
        "16",
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

    OSReport("Func %p with p_link %d completed in %.2fms\n", cb, g->p_link, OSTicksToMilliseconds(time));
    return;
}
CODEPATCH_HOOKCREATE(0x80429fd8, "lwz	3, 0x0014 (25)\n\t"
                                 "lwz	4, 0x0010 (25)\n\t",
                     GOBJProc_Post, "", 0)

void GOBJProc_End()
{
    OSReport("%d proc's completed in %.2fms\n", proc_total, OSTicksToMilliseconds(time_total));
    OSReport("## GObj Proc End ##\n\n");

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