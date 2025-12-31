#include <stddef.h>

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
        return 0;

    // OSReport("Starpole: sending imm cmd %d\n", cmd);

    u32 resp = 0;
    u32 buf = ((args & 0xFFFFFF) << 16) | cmd & 0xFFFF; // shove args into payload

    if (!EXIProbe(STARPOLE_EXI_CHAN))
        return 0;
    
    if (!EXILock(STARPOLE_EXI_CHAN, 0))
        return 0;

    if (!EXISelect(STARPOLE_EXI_CHAN, 0, STARPOLE_EXI_FREQ))
        goto FAIL;

    // send
    if (!EXIImm(STARPOLE_EXI_CHAN, &buf, 4, EXI_WRITE))
        goto FAIL;

    // block until DMA completes
    if (!EXISync(STARPOLE_EXI_CHAN))
        goto FAIL;

    // get response
    if (!EXIImm(STARPOLE_EXI_CHAN, &resp, 4, EXI_READ))
        goto FAIL;

    // block until DMA completes
    if (!EXISync(STARPOLE_EXI_CHAN))
        goto FAIL;

    // if we get here everything succeeded
    goto CLEANUP;

FAIL:
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
    if (size > sizeof(*starpole_buf))
    {
        OSReport("Starpole: incoming DMA too large for buffer\n");
        assert("0");
    }


    // static char *exi_mode_names[] = {"reading", "writing"};
    // OSReport("Starpole: %s payload of size 0x%x bytes\n", exi_mode_names[mode], size);

    int result;

    DCFlushRange(buf, size); // Flush cache so EXI sees correct data

    if (!EXIProbe(STARPOLE_EXI_CHAN))
        return 0;
    
    if (!EXILock(STARPOLE_EXI_CHAN, 0))
        return 0;

    if (!EXISelect(STARPOLE_EXI_CHAN, 0, STARPOLE_EXI_FREQ))
        goto FAIL;

    // start DMA
    if (!EXIDma(STARPOLE_EXI_CHAN, buf, size, mode))
        goto FAIL;

    // block until DMA completes
    if (!EXISync(STARPOLE_EXI_CHAN))
        goto FAIL;

    // if we get here everything succeeded
    result = 1;
    goto CLEANUP;

FAIL:
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
    // check if starpole is present
    if (Starpole_Imm(STARPOLE_CMD_ID, 0) != STARPOLE_DEVICE_ID)
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
}

// test functions
StarpoleDataTest *Test_GetString()
{
    // get size of incoming data
    int recv_size = Starpole_Imm(STARPOLE_CMD_TEST, 0);
    if (recv_size == -1)
        return 0;

    // receive it
    if (!Starpole_DMA(starpole_buf, recv_size, EXI_READ))
        return 0;

    return (StarpoleDataTest *)starpole_buf;
}
void Test_DisplayString()
{
    // if (!is_starpole)
        return;

    // display test string
    int canvas_idx = Hoshi_GetScreenCanvasIndex();
    Text *t = Text_CreateText(1, canvas_idx);
    t->kerning = 1;
    t->use_aspect = 1;
    t->trans = (Vec3){10, 30, 0};
    t->viewport_scale = (Vec2){0.5, 0.5};
    t->aspect = (Vec2){560, 32};
    t->viewport_color = (GXColor){0, 0, 0, 128};

    char buf[256];
    Text_Sanitize(starpole_data_test.str, buf, sizeof(buf));

    Text_AddSubtext(t, 0, 0, buf);
}

// UI
JOBJSet *starpole_jobjset = 0;
void Starpole_LoadAsset()
{
    HSD_Archive *archive = Archive_LoadFile(STARPOLE_ASSET_FILE);
    if (!archive)
        return;
        
    // check if symbol exists
    JOBJSet **set = Archive_GetPublicAddress(archive, STARPOLE_ASSET_SYMBOL);
    if (!set)
        return;

    // save a ptr to the jobjset
    starpole_jobjset = set[0];
    
}
void Starpole_DisplayAsset()
{
    // didnt load
    if (!starpole_jobjset)
        return;

    GOBJ_EZCreator(0, 0, 0,
                    0, 0,
                    HSD_OBJKIND_JOBJ, starpole_jobjset->jobj,
                    GOBJ_Anim, 0,
                    JObj_GX, HOSHI_SCREENCAM_GXLINK, 0);
}