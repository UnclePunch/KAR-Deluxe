#include <stddef.h>

#include "text.h"
#include "os.h"
#include "hsd.h"
#include "game.h"
#include "hud.h"
#include "scene.h"
#include "inline.h"
#include "scene.h"

#include "hoshi/func.h"
#include "hoshi/screen_cam.h"

#include "starpole.h"
#include "replay.h"
#include "dolphin.h"
#include "netsync.h"
#include "code_patch/code_patch.h"
#include "text_joint/text_joint.h"

HSD_Pad *sys_pads = (HSD_Pad *)0x8058b0e4;

void (*HSD_InsertIntoPadQueue)(PADStatus *status, int unk) = (void *)0x80412480;
void (*HSD_PadConsume)() = (void *)0x80062978;

extern int is_netplay;
extern StarpoleDataDolphin *dolphin_data;

int Netplay_SendInputs(PADStatus *status)
{
    int result = 0;
    int enable = OSDisableInterrupts();

    // copy pad data to aligned buffer
    char buffer[sizeof(PADStatus) * 4] __attribute__((aligned(32)));
    memcpy(buffer, status, sizeof(buffer));

    // notify of incoming data
    if (Starpole_Imm(STARPOLE_CMD_NETPADSEND, sizeof(buffer)) <= 0)
    {
        OSReport("Starpole: unable to send pad data.\n");
        goto CLEANUP;
    }

    // send it
    if (!Starpole_DMA((StarpoleBuffer *)buffer, sizeof(buffer), EXI_WRITE))
        goto CLEANUP;

    result = 1;

CLEANUP:
    OSRestoreInterrupts(enable);
    return result;
}
int Netplay_ReceiveInputs()
{
    int result = 0;
    int enable = OSDisableInterrupts();

    // transfer buffer
    PADStatus buffer[4] __attribute__((aligned(32)));

    // request data
    if (Starpole_Imm(STARPOLE_CMD_NETPADRECV, 0) <= 0)
    {
        OSReport("Starpole: inputs not ready.\n");
        goto CLEANUP;
    }

    // receive it
    if (!Starpole_DMA((StarpoleBuffer *)buffer, sizeof(buffer), EXI_READ))
        goto CLEANUP;

    // consume the pad and consume it
    HSD_InsertIntoPadQueue(buffer, 0);
    HSD_PadConsume();

    result = 1;

CLEANUP:
    OSRestoreInterrupts(enable);
    return result;
}

int Netplay_WaitForClients()
{
    void (*HSD_PadAlarmCallback)() = (void *)0x800625cc;
    void (*HSD_DoTasks)() = (void *)0x80059cfc;

    // input update
    PADStatus status[4];
    PADRead(status);
    Netplay_SendInputs(status);           // send inputs to dolphin

    // perform a tick update only when we have all inputs for this frame
    if (Netplay_ReceiveInputs())
    {
        // clients are ready, process the frame
        return 0;
    }
    
    // render a frame and come back
    return 1;
}
CODEPATCH_HOOKCONDITIONALCREATE(0x8000682c, "", Netplay_WaitForClients, "", 0, 0x80006a8c)

void Netsync_AdjustGameLoop()
{
    int *is_alarm_active = (int *)0x80550ca8;
    OSAlarm *alarm_ptr = (OSAlarm *)0x80550d28;

    // cancel any active alarms
    if (*is_alarm_active)
        OSCancelAlarm(alarm_ptr);
    CODEPATCH_REPLACEINSTRUCTION(0x80062660, 0x4e800020);   // disable pad alarm creation

    CODEPATCH_REPLACEINSTRUCTION(0x80006bd4, 0x38600001);   // spoof pad queue as always 1
    // CODEPATCH_REPLACEINSTRUCTION(0x80006b94, 0x60000000);   // 
    CODEPATCH_REPLACECALL(0x80006b94, VIWaitForRetrace);    // replace pad alarm jam with viwaitforretrace

    // netpause at the top of each frame
    CODEPATCH_REPLACEINSTRUCTION(0x8000682c, 0x60000000);   // remove pad consume
    CODEPATCH_HOOKAPPLY(0x8000682c);
}

Text *rng_text;
void Netsync_CreateRNGText()
{
    // display test string
    Text *t = Hoshi_CreateScreenText();
    t->kerning = 1;
    t->use_aspect = 1;
    t->trans = (Vec3){480, 0, 0};
    t->viewport_scale = (Vec2){0.5, 0.5};
    t->aspect = (Vec2){320, 32};
    t->viewport_color = (GXColor){0, 0, 0, 128};
    Text_AddSubtext(t, 0, 0, "RNG Seed: %08X");

    rng_text = t;
}
void Netsync_UpdateRNGText()
{
    Text_SetText(rng_text, 0, "RNG Seed: %08X", **stc_rng_seed);
}

void Netsync_Init()
{
    Netsync_AdjustGameLoop();
}