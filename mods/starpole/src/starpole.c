#include <stddef.h>
#include <stdarg.h>

#include "text.h"
#include "os.h"
#include "hsd.h"
#include "game.h"
#include "exi.h"
#include "scene.h"
#include "inline.h"
#include "scene.h"
#include "rider.h"

#include "hoshi/func.h"
#include "hoshi/screen_cam.h"

#include "starpole.h"
#include "code_patch/code_patch.h"
#include "text_joint/text_joint.h"

int is_starpole = 0;                // bool indicating whether or not starpole is present (checked on startup)
StarpoleBuffer *starpole_buf;

StarpoleDataTest starpole_data_test;

// EXI Transfer functions
int Starpole_Imm(StarpoleCmd cmd, int args)
{
    // return values:
    //   -1  = EXI failure
    //  else = EXI reponse 

    if (cmd >= STARPOLE_CMD_NUM)
        return -1;

    // OSReport("Starpole: sending imm cmd %d\n", cmd);

    u32 resp = 0;
    u32 buf = ((args & 0xFFFFFF) << 16) | (cmd & 0xFFFF); // shove args into payload

    // ensure a device is inserted before waiting for probe
    volatile EXIChannelReg *exi_reg = (EXIChannelReg *)0xCC006800;
    if (!(exi_reg[STARPOLE_EXI_CHAN].BR & 0x1000))
        return -1;
        
    int is_probed;
    do { is_probed = EXIProbe(STARPOLE_EXI_CHAN);
    } while (!is_probed);

    if (!EXILock(STARPOLE_EXI_CHAN, 0))
        return -1;

    if (!EXISelect(STARPOLE_EXI_CHAN, STARPOLE_DEVICE, STARPOLE_EXI_FREQ))
        goto FAIL;

    // send
    if (!EXIImm(STARPOLE_EXI_CHAN, &buf, 4, EXI_WRITE, NULL))
        goto FAIL;

    // block until DMA completes
    if (!EXISync(STARPOLE_EXI_CHAN))
        goto FAIL;

    // get response
    if (!EXIImm(STARPOLE_EXI_CHAN, &resp, 4, EXI_READ, NULL))
        goto FAIL;

    // block until DMA completes
    if (!EXISync(STARPOLE_EXI_CHAN))
        goto FAIL;

    // if we get here everything succeeded
    goto CLEANUP;

FAIL:
    OSReport("Starpole: EXI Imm Failed!\n");
    resp = -1;

CLEANUP:
    EXIDeselect(STARPOLE_EXI_CHAN);
    EXIUnlock(STARPOLE_EXI_CHAN);

    // OSReport("Starpole: imm response (0x%x)\n", resp);

    return resp;
}
int Starpole_DMA(StarpoleBuffer *buf, int size, EXIMode mode)
{
    // ensure buffer and size is 32 byte aligned
    if ((int)buf % 32 != 0)
    {
        OSReport("Starpole: buffer has alignment error\n");
        assert("0");
    }

    // ensure buffer is large enough to receive this data
    if (size > sizeof(StarpoleBuffer))
    {
        OSReport("Starpole: incoming DMA too large for buffer\n");
        assert("0");
    }

    // static char *exi_mode_names[] = {"reading", "writing"};
    // OSReport("Starpole: %s payload of size 0x%x bytes\n", exi_mode_names[mode], size);

    int result;

    if (mode == EXI_WRITE)
        DCFlushRange(buf, size); // Flush cache so EXI sees correct data

    // ensure a device is inserted before waiting for probe
    volatile EXIChannelReg *exi_reg = (EXIChannelReg *)0xCC006800;
    if (!(exi_reg[STARPOLE_EXI_CHAN].BR & 0x1000))
        return -1;
        
    int is_probed;
    do { is_probed = EXIProbe(STARPOLE_EXI_CHAN);
    } while (!is_probed);
    
    if (!EXILock(STARPOLE_EXI_CHAN, 0))
        return 0;

    if (!EXISelect(STARPOLE_EXI_CHAN, STARPOLE_DEVICE, STARPOLE_EXI_FREQ))
        goto FAIL;

    // start DMA
    if (!EXIDma(STARPOLE_EXI_CHAN, buf, size, mode, NULL))
        goto FAIL;

    // block until DMA completes
    if (!EXISync(STARPOLE_EXI_CHAN))
        goto FAIL;

    if (mode == EXI_READ)
        DCInvalidateRange(buf, size); // Flush cache so cpu sees correct data

    // if we get here everything succeeded
    result = 1;
    goto CLEANUP;

FAIL:
    OSReport("Starpole: EXI DMA Failed!\n");
    result = 0;

CLEANUP:
    EXIDeselect(STARPOLE_EXI_CHAN);
    EXIUnlock(STARPOLE_EXI_CHAN);

    // OSReport("Starpole: transfer result (%d)\n", result);
    return result;

}

int Starpole_IsPresent()
{
    return is_starpole;
}
void Starpole_Init()
{
    // request device to id itself
    int id = Starpole_Imm(STARPOLE_CMD_ID, 0);

    // check if starpole is present
    if (id != STARPOLE_DEVICE_ID)
        return;

    is_starpole = 1;

    OSReport("Starpole detected!\n");

    // allocate starpole transfer uffer
    starpole_buf = HSD_MemAlloc(sizeof(*starpole_buf));

    // load asset
    Starpole_LoadAsset();

    // receive test data
    StarpoleDataTest *data = Test_GetString();
    if (data)
    {
        strncpy(starpole_data_test.str, data->str, sizeof(starpole_data_test.str));
        OSReport("Received test data: %s\n", &starpole_data_test.str);
    }
    
    return;
}

// test functions
StarpoleDataTest *Test_GetString()
{
    int enable = OSDisableInterrupts();
    StarpoleDataTest* result = NULL;

    // get size of incoming data
    int recv_size = Starpole_Imm(STARPOLE_CMD_TEST, 0);
    if (recv_size == -1)
        goto CLEANUP;

    // receive it
    if (!Starpole_DMA(starpole_buf, recv_size, EXI_READ))
        goto CLEANUP;

    result = (StarpoleDataTest *)starpole_buf;

CLEANUP:
    OSRestoreInterrupts(enable);
    return result;
}

// onscreen console
TextConsole console;
void TextConsole_Init()
{
    // create test string
    Text *t = Hoshi_CreateScreenText();
    t->kerning = 1;
    t->use_aspect = 1;
    t->trans = (Vec3){10, 30, 0};
    t->viewport_scale = (Vec2){0.3, 0.3};
    t->aspect = (Vec2){300, 0};
    t->viewport_color = (GXColor){0, 0, 0, 128};

    console.t = t;
    console.y = 0;
}
void TextConsole_AddString(float x, float y, char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);

    char buf[256];
    int ret = vsnprintf(buf, sizeof(buf), fmt, args);

    Text_AddSubtext(console.t, x, console.y + y, buf);

    console.y += y + 30;
    console.t->aspect.Y = console.y * console.t->scale.Y;
}

void Test_DisplayString()
{
    // if (!is_starpole)
        return;

    // display test string
    Text *t = Hoshi_CreateScreenText();
    t->kerning = 1;
    t->use_aspect = 1;
    t->trans = (Vec3){10, 30, 0};
    t->viewport_scale = (Vec2){0.5, 0.5};
    t->aspect = (Vec2){560, 32};
    t->viewport_color = (GXColor){0, 0, 0, 128};

    char buf[256];
    Text_Sanitize(starpole_data_test.str, buf, sizeof(buf));

    TextConsole_AddString(0, 0, buf);
}

// UI
JOBJSet *starpole_jobjset = 0;
void Starpole_LoadAsset()
{
    HSD_Archive *archive = Archive_LoadFile(STARPOLE_ASSET_FILE);
    if (!archive)
        return;
        
    // ensure symbol exists
    JOBJSet **set = Archive_GetPublicAddress(archive, STARPOLE_ASSET_SYMBOL);
    if (!set)
        return;

    // save a ptr to the jobjset
    starpole_jobjset = set[0];
}
void Starpole_DisplayAsset()
{
    // ensure it loaded
    if (!starpole_jobjset)
        return;

    // create the UI gobj
    GOBJ *g = GOBJ_EZCreator(0, 0, 0,
                    0, 0,
                    HSD_OBJKIND_JOBJ, starpole_jobjset->jobj,
                    GOBJ_Anim, 0,                               // add a proc to animate it each frame
                    JObj_GX, HOSHI_SCREENCAM_GXLINK, 0);

    // add its animations
    JObj_AddSetAnim(g->hsd_object, 0, starpole_jobjset, 0, 1.0f);
}