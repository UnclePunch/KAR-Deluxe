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

int is_netplay = 0;

extern StarpoleExport starpole_export; 
StarpoleDataDolphin *dolphin_data;
extern ReplayMode replay_mode;

// EXI
int Dolphin_ReqData()
{
    int result = 0;
    int enable = OSDisableInterrupts();

    // request data
    if (Starpole_Imm(STARPOLE_CMD_DOLPHIN, 0) <= 0)
    {
        OSReport("Starpole: Dolphin not detected.\n");
        goto CLEANUP;
    }

    // receive it
    if (!Starpole_DMA((StarpoleBuffer *)dolphin_data, sizeof(*dolphin_data), EXI_READ))
        goto CLEANUP;

    result = 1;

CLEANUP:
    OSRestoreInterrupts(enable);
    return result;
}

// Init
void Dolphin_Init()
{
    if (!Starpole_IsPresent() && !DOLPHIN_DEBUG)
        return;

    // alloc buffer
    dolphin_data = HSD_MemAlloc(sizeof(*dolphin_data));

    if (DOLPHIN_DEBUG)
    {
        static char *test_names[] = {
            "UnclePunch",
            "charity",
            "Taco",
            "ThePulsarLegend",
        };

        dolphin_data->aspect_mult = 1;
        dolphin_data->netplay.is = 1;
        dolphin_data->netplay.ply = 0;
        for (int i = 0; i < GetElementsIn(dolphin_data->netplay.usernames); i++)
            strcpy(dolphin_data->netplay.usernames[i], test_names[i]);
    }

    // get data
    if (DOLPHIN_DEBUG || Dolphin_ReqData())
    {
        OSReport("Starpole: Dolphin detected.\n");
        
        // store pointer to export data
        starpole_export.dolphin_data = dolphin_data;

        // init netplay flag
        if (dolphin_data->netplay.is)
            Netplay_Init();
        else
            OSReport("Starpole: Netplay not detected.\n");
    }

}

void Netplay_Init()
{
    is_netplay = 1;

    OSReport("Starpole: Netplay detected.\n");

    if (dolphin_data->netplay.ply != -1)
    {
        OSReport(" You are player %d \"%s\"\n", 
            dolphin_data->netplay.ply, 
            dolphin_data->netplay.usernames[dolphin_data->netplay.ply]);
    }
    else
        OSReport(" You are spectating.\n");

    // init rng seed
    *hsd_rand_seed = dolphin_data->netplay.rng_seed;

    // PadAlarm_Remove();

    Netsync_Init();
}

// Fullscreen
void Netplay_OverridePlayerView()
{
    if (!is_netplay || replay_mode == REPLAY_PLAYBACK)
        return;

    GameData *gd = Gm_GetGameData();

    if (dolphin_data->netplay.ply != -1 && Gm_GetGameData()->ply_desc[dolphin_data->netplay.ply].p_kind == PKIND_HMN)
    {
        for (int i = 0; i < GetElementsIn(gd->ply_view_desc); i++)
            gd->ply_view_desc[i].flag = PLYCAM_OFF;
        
        // plugged in and not present, give us live cam to spectate with
        int held = stc_engine_pads[dolphin_data->netplay.ply].held;
        if ((held & (PAD_BUTTON_A | PAD_TRIGGER_L | PAD_TRIGGER_R)) != (PAD_BUTTON_A | PAD_TRIGGER_L | PAD_TRIGGER_R))
            gd->ply_view_desc[dolphin_data->netplay.ply].flag = PLYCAM_ON;
    }
}

// Player Tags
void Netplay_CreatePlayerTags()
{
    if (!is_netplay)
        return;

    Game3dData *g3d = Gm_Get3dData();
    int canvas_idx = Text_CreateCanvas(0, 1, 0, 0, 0, GAMEGX_HUDORTHO, 1, 0);

    // loop through all player views
    for (int i = 0; i < 4; i++)
    {
        if (Ply_GetPKind(i) == PKIND_NONE || !Ply_IsViewOn(i))
            continue;

        // create a gobj to render the tags for this viewport
        GOBJ *g = GOBJ_EZCreator(27, GAMEPLINK_HUD, 0,
                        sizeof(PlayerTagViewData), Netplay_DestroyPlayerTagViewGObj,
                        0, 0,
                        0, 0, 
                        Netplay_PlayerTagGX, GAMEGX_HUDORTHO, 1);

        // init data
        PlayerTagViewData *gp = g->userdata;
        gp->ply = i;
        for (int i = 0; i < GetElementsIn(gp->t); i++)
            gp->t[i] = 0;

        // loop through all other players
        for (int ply = 0; ply < 4; ply++)
        {
            if (Ply_GetPKind(ply) != PKIND_HMN || ply == i)
                continue;

            char name[NETPLAY_TAGMAX + 1];
            strncpy(name, dolphin_data->netplay.usernames[ply], NETPLAY_TAGMAX);
            name[NETPLAY_TAGMAX] = '\0';

            // create a name for them
            Text *t = Text_CreateText(0, canvas_idx);
            t->gobj->gx_cb = 0;
            t->viewport_scale = (Vec2){0.05, 0.05};
            t->trans = (Vec3){320 * t->viewport_scale.X, 240 * t->viewport_scale.Y, 0};
            t->viewport_color = (GXColor){0,0,0,0};
            t->aspect = (Vec2){100,32};
            t->use_aspect = 1;
            t->align = 1;
            t->kerning = 1;

            // get text color
            GXColor *text_color;
            GOBJ *plynum_gobj = g3d->plynum_gobj[i][ply];
            if (plynum_gobj)
            {
                DOBJ *plynum_dobj = JObj_GetDObjIndex(plynum_gobj->hsd_object, 0, 0);
                if (plynum_dobj)
                    text_color = &plynum_dobj->mobj->mat->diffuse;
            }

            // name outline
            int y = (text_color->r * 299 + text_color->g * 587 + text_color->b * 114) / 1000;
            if (y < 80)
                t->color = (GXColor){255, 255, 255, 255};
            else
                t->color = (GXColor){0, 0, 0, 255};
            Text_AddSubtext(t, -2, -2, name);
            Text_AddSubtext(t, -2, 2, name);
            Text_AddSubtext(t, 2, -2, name);
            Text_AddSubtext(t, 2, 2, name);

            t->color = *text_color;
            Text_AddSubtext(t, 0, 0, name);

            gp->t[ply] = t;
        }
    
    }
}
void Netplay_DestroyPlayerTagViewGObj(PlayerTagViewData *gp)
{
    for (int i = 0; i < GetElementsIn(gp->t); i++)
    {
        if (gp->t[i])
            Text_Destroy(gp->t[i]);
    }
}
void Netplay_PlayerTagGX(GOBJ *g, int pass)
{
    // only on transparency pass
    if (pass != 2)
        return; 

    Game3dData *g3d = Gm_Get3dData();
    PlayerTagViewData *gp = g->userdata;

    // loop through tags for this viewport
    for (int i = 0; i < GetElementsIn(gp->t); i++)
    {
        Text *t = gp->t[i];
        if (!t)
            continue;;

        // get plynum hud gobj
        GOBJ *plynum_gobj = g3d->plynum_gobj[gp->ply][i];
        if (!plynum_gobj)
            continue;

        HUDElementData *hp = plynum_gobj->userdata;

        // check if plynum was rendered
        if (!hp->ply_num.is_visible || !hp->is_visible)
            t->hidden = 1;
        else
        {
            t->hidden = 0;
            JOBJ *plynum_jobj = plynum_gobj->hsd_object;

            static float tag_offsets[] = {0, 6.3, 5.8, 5, 5};

            // move text to PlyNum
            t->trans.X = plynum_jobj->trans.X;
            t->trans.Y = -(plynum_jobj->trans.Y + tag_offsets[g3d->plyview_num]);
        }

        // splitscreen logic
        if (g3d->plyview_num >= 2)
        {
            CamScissor view_scissor;
            PlyCam_Get2PScissor(Ply_GetViewIndex(hp->ply), &view_scissor);
            GXSetScissor(view_scissor.left, 
                        view_scissor.top, 
                        view_scissor.right - view_scissor.left,
                        view_scissor.bottom - view_scissor.top);
        }

        // call render func
        Text_GX(t->gobj, pass);
    }

    // restore scissor
    CamScissor full_scissor;
    PlyCam_GetFullscreenScissor(&full_scissor);
    GXSetScissor(full_scissor.left, 
                full_scissor.top, 
                full_scissor.right - full_scissor.left,
                full_scissor.bottom - full_scissor.top);
}

// Hash Display
Text *hash_text;
void Hash_CreateText()
{
    // display test string
    Text *t = Hoshi_CreateScreenText();
    t->kerning = 1;
    t->use_aspect = 1;
    t->trans = (Vec3){0, 0, 0};
    t->viewport_scale = (Vec2){0.5, 0.5};
    t->aspect = (Vec2){320, 32};
    t->viewport_color = (GXColor){0, 0, 0, 128};
    Text_AddSubtext(t, 0, 0, "Game State Hash: %08X", Replay_HashGameState());

    hash_text = t;
}
void Hash_DestroyText()
{
    Text_Destroy(hash_text);
}

// frame budget test
void StressTest_Think()
{
    if (dolphin_data->netplay.ply != -1)
    {
        int held = stc_engine_pads[dolphin_data->netplay.ply].held;
        int down = stc_engine_pads[dolphin_data->netplay.ply].down;
        if ((held & (PAD_TRIGGER_Z | PAD_TRIGGER_L | PAD_TRIGGER_R)) == (PAD_TRIGGER_Z | PAD_TRIGGER_L | PAD_TRIGGER_R))
        {
            for (int i = 0; i < 3; i++)
            {
                for (int j = 0; j < 1000000; j++)
                    ;
            }
        }
    }
}
void StressTest_Create()
{
    GOBJ *g = GOBJ_EZCreator(27, GAMEPLINK_HUD, 0,
                    0, 0,
                    0, 0,
                    StressTest_Think, 20, 
                    0, 0, 0);
}

// Pad stuff
void PadAlarm_Remove()
{
    int *is_alarm_active = (int *)0x80550ca8;
    OSAlarm *alarm_ptr = (OSAlarm *)0x80550d28;

    // cancel any active alarms
    if (*is_alarm_active)
        OSCancelAlarm(alarm_ptr);
    
    // disable pad alarm creation
    CODEPATCH_REPLACEINSTRUCTION(0x80062660, 0x4e800020);
    
    // execute padread on VI retrace callback
    CODEPATCH_REPLACEFUNC(0x80005894, 0x800625cc);

    // // replace pad alarm jam with viwaitforretrace
    // CODEPATCH_REPLACECALL(0x80006b94, VIWaitForRetrace);

    // // move padread to beginning of new frame
    // CODEPATCH_HOOKAPPLY(0x80006b98);
}
