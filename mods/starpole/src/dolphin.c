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
#include "code_patch/code_patch.h"
#include "text_joint/text_joint.h"

int is_netplay = 0;
StarpoleDataDolphin *dolphin_data;
extern ReplayMode replay_mode;

// EXI
int Dolphin_ReqData()
{
    int result = 0;
    int enable = OSDisableInterrupts();

    // request data
    if (Starpole_Imm(STARPOLE_CMD_NETPLAY, 0) <= 0)
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
        dolphin_data->aspect_mult = 0;

        static char *test_names[] = {
            "UnclePunch",
            "charity",
            "Taco",
            "ThePulsarLegend",
        };

        is_netplay = 1;
        dolphin_data->netplay.ply = 0;
        for (int i = 0; i < GetElementsIn(dolphin_data->netplay.usernames); i++)
            strcpy(dolphin_data->netplay.usernames[i], test_names[i]);
    }

    // get data
    else if (Dolphin_ReqData())
    {
        bp();
        OSReport("Starpole: Dolphin detected.\n");
        
        if (dolphin_data->netplay.ply != -1)
        {
            is_netplay = 1;

            OSReport("Starpole: Netplay detected. You are player %d \"%s\"\n", 
                dolphin_data->netplay.ply, 
                dolphin_data->netplay.usernames[dolphin_data->netplay.ply]);
        }
        else
            OSReport("Starpole: Netplay not detected.\n");
    }

}

// Fullscreen
void Netplay_OverridePlayerView()
{
    if (!is_netplay || replay_mode == REPLAY_PLAYBACK)
        return;

    GameData *gd = Gm_GetGameData();

    for (int i = 0; i < GetElementsIn(gd->ply_view_desc); i++)
        gd->ply_view_desc[i].flag = PLYCAM_OFF;
    
    // if its a netplay game and we are not plugged in, force p1 cam
    if (dolphin_data->netplay.ply == -1)
        gd->ply_view_desc[0].flag = PLYCAM_ON;
    else
    {
        // // if we are plugged in and in this game, force our cam on
        // if (Gm_GetGameData()->ply_desc[dolphin_data->netplay.ply].p_kind == PKIND_HMN)
            gd->ply_view_desc[dolphin_data->netplay.ply].flag = PLYCAM_ON;

        // // plugged in and not present, give us live cam to spectate with
        // else
        //     gd->ply_view_desc[dolphin_data->netplay.ply].flag = PLYCAM_LIVE;
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

            // move text to PlyNum
            t->trans.X = plynum_jobj->trans.X;
            t->trans.Y = -(plynum_jobj->trans.Y + 6.3);
        }

        // splitscreen logic
        if (Gm_Get3dData()->plyview_num >= 2)
        {
            CamScissor view_scissor;
            PlyCam_Get2PScissor(Ply_GetViewIndex(hp->ply), &view_scissor);
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