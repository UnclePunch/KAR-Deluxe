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
#include "netplay.h"
#include "code_patch/code_patch.h"
#include "text_joint/text_joint.h"

int is_netplay = 0;
StarpoleDataNetplay *netplay_data;
extern ReplayMode replay_mode;

int Netplay_ReqData()
{
    int result = 0;
    int enable = OSDisableInterrupts();

    // request data
    if (Starpole_Imm(STARPOLE_CMD_NETPLAY, 0) <= 0)
    {
        OSReport("Netplay: not active\n");
        goto CLEANUP;
    }

    // receive it
    if (!Starpole_DMA((StarpoleBuffer *)netplay_data, sizeof(*netplay_data), EXI_READ))
        goto CLEANUP;

    result = 1;

CLEANUP:
    OSRestoreInterrupts(enable);
    return result;
}

void Netplay_Init()
{
    if (!Starpole_IsPresent())
        return;

    // alloc buffer
    netplay_data = HSD_MemAlloc(sizeof(*netplay_data));

    // get data
    if (Netplay_ReqData())
    {
        is_netplay = 1;

        OSReport("Netplay: you are player %d \"%s\"\n", 
            netplay_data->ply, 
            netplay_data->usernames[netplay_data->ply]);
    }
}

void Netplay_OverridePlayerView()
{
    if (!is_netplay || replay_mode == REPLAY_PLAYBACK)
        return;

    GameData *gd = Gm_GetGameData();

    for (int i = 0; i < GetElementsIn(gd->ply_view_desc); i++)
        gd->ply_view_desc[i].flag = PLYCAM_OFF;
    
    // if its a netplay game and we are not plugged in, force p1 cam
    if (netplay_data->ply == -1)
        gd->ply_view_desc[0].flag = PLYCAM_ON;
    else
    {
        // // if we are plugged in and in this game, force our cam on
        // if (Gm_GetGameData()->ply_desc[netplay_data->ply].p_kind == PKIND_HMN)
            gd->ply_view_desc[netplay_data->ply].flag = PLYCAM_ON;

        // // plugged in and not present, give us live cam to spectate with
        // else
        //     gd->ply_view_desc[netplay_data->ply].flag = PLYCAM_LIVE;
    }
}

// Player Tags
void Netplay_CreatePlayerTags()
{
    if (!is_netplay)
        return;

    Game3dData *g3d = Gm_Get3dData();
    int canvas_idx = Text_CreateCanvas(0, 1, 0, 0, 0, GAMEGX_HUD, 1, 0);

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
                        Netplay_PlayerTagGX, GAMEGX_HUD, 1);

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

            // name outline
            t->color = (GXColor){0, 0, 0, 255};
            Text_AddSubtext(t, -2, -2, netplay_data->usernames[ply]);
            Text_AddSubtext(t, -2, 2, netplay_data->usernames[ply]);
            Text_AddSubtext(t, 2, -2, netplay_data->usernames[ply]);
            Text_AddSubtext(t, 2, 2, netplay_data->usernames[ply]);

            // set name color
            GOBJ *plynum_gobj = g3d->plynum_gobj[i][ply];
            if (plynum_gobj)
            {
                DOBJ *plynum_dobj = JObj_GetDObjIndex(plynum_gobj->hsd_object, 0, 0);
                if (plynum_dobj)
                    t->color = plynum_dobj->mobj->mat->diffuse;
            }
            else
                t->color = (GXColor){255, 255, 255, 255};
            Text_AddSubtext(t, 0, 0, netplay_data->usernames[ply]);

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
    if (pass != 2)
        return; 

    Game3dData *g3d = Gm_Get3dData();

    PlayerTagViewData *gp = g->userdata;
    for (int i = 0; i < GetElementsIn(gp->t); i++)
    {
        Text *t = gp->t[i];
        if (!t)
            continue;;

        // get plynum hud gobj
        GOBJ *plynum_gobj = g3d->plynum_gobj[gp->ply][i];
        if (!plynum_gobj)
            continue;

        HudPlyNumData *plynum_data = plynum_gobj->userdata;
        if (!plynum_data->is_visible || !plynum_data->x8_02)
            t->hidden = 1;
        else
        {
            t->hidden = 0;
            JOBJ *plynum_jobj = plynum_gobj->hsd_object;

            // move text to PlyNum
            t->trans.X = plynum_jobj->trans.X;
            t->trans.Y = -(plynum_jobj->trans.Y + 6.3);
        }

        if (Gm_Get3dData()->plyview_num >= 2)
        {
            CamScissor view_scissor;
            PlyCam_GetViewIndexScissor(Ply_GetViewIndex(plynum_data->ply), &view_scissor);
            GXSetScissor(view_scissor.left, 
                        view_scissor.bottom, 
                        view_scissor.right - view_scissor.left,
                        view_scissor.top - view_scissor.bottom);
        }

        // call render func
        Text_GX(t->gobj, pass);
    }

    // restore scissor
    CamScissor full_scissor;
    PlyCam_GetFullscreenScissor(&full_scissor);
    GXSetScissor(full_scissor.left, 
                full_scissor.bottom, 
                full_scissor.right - full_scissor.left,
                full_scissor.top - full_scissor.bottom);

}