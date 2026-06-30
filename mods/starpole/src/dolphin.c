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
#include "playback.h"
#include "dolphin.h"
#include "netsync.h"
#include "code_patch/code_patch.h"
#include "text_joint/text_joint.h"

StarpoleDataDolphin *dolphin_data;
extern StarpoleExport starpole_export; 
extern ReplayMode replay_mode;

HSD_Archive *netplay_archive;

int is_dolphin = 0;

// EXI
int Dolphin_ReqData(StarpoleDataDolphin *data)
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
    if (!Starpole_DMA((StarpoleBuffer *)data, sizeof(*data), EXI_READ))
        goto CLEANUP;

    result = 1;

CLEANUP:
    OSRestoreInterrupts(enable);
    return result;
}

int Dolphin_IsNetplay()
{
    return (is_dolphin && dolphin_data->netplay.is);
}

// Fullscreen
void Netplay_OverridePlayerView(StarpoleDataDolphin *data)
{
    if (!data->netplay.is)
        return;

    GameData *gd = Gm_GetGameData();

    // if (replay_mode == REPLAY_PLAYBACK)
    // {
    //     gd->ply_view_desc[0].flag = PLYCAM_LIVE;
    //     return;
    // }

    if (data->netplay.ply != -1 && Gm_GetGameData()->ply_desc[data->netplay.ply].p_kind == PKIND_HMN)
    {
        for (int i = 0; i < GetElementsIn(gd->ply_view_desc); i++)
            gd->ply_view_desc[i].flag = PLYCAM_OFF;
                    
        // plugged in and not present, give us live cam to spectate with
        int held = stc_engine_pads[data->netplay.ply].held;
        if ((held & (PAD_BUTTON_A | PAD_TRIGGER_L | PAD_TRIGGER_R)) != (PAD_BUTTON_A | PAD_TRIGGER_L | PAD_TRIGGER_R))
            gd->ply_view_desc[dolphin_data->netplay.ply].flag = PLYCAM_ON;
    }
}

// Player Tags
void Netplay_CreatePlayerTags(StarpoleDataDolphin *data)
{
    if (!data->netplay.is || 
        !netplay_archive)       // needs the edited PlyNum jobj
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
            strncpy(name, data->netplay.usernames[ply], NETPLAY_TAGMAX);
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
                {
                    text_color = &plynum_dobj->mobj->mat->diffuse;
                    plynum_dobj->flags |= DOBJ_HIDDEN;  // hide pX indicator
                }
            }

            // get outline color
            GXColor *outline_color;
            outline_color = UI_GetTextOutlineColor(text_color);

            Text_AddOutline(t, outline_color, name);

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

            // static float tag_offsets[] = {0, 6.3, 5.8, 5, 5};
            static float tag_offsets[] = {0, 4, 3.8, 3.1, 3.1};

            // move text to PlyNum
            t->trans.X = plynum_jobj->trans.X;
            t->trans.Y = -(plynum_jobj->trans.Y + tag_offsets[g3d->plyview_num]);
        }

        // splitscreen logic
        if (g3d->plyview_num >= 2)
        {
            CamScissor view_scissor;
            if (g3d->plyview_num == 2)
                PlyCam_Get2PScissor(Ply_GetViewIndex(hp->ply), &view_scissor);
            else
                PlyCam_Get4PScissor(Ply_GetViewIndex(hp->ply), &view_scissor);
            
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

// Alt PlyNum - Netplay player tags require a custom jobj to hide the P1 texture...
void Netplay_LoadAltPlyNumFile()
{
    // load netplay assets
    if (!netplay_archive)
        netplay_archive = Archive_LoadFile("IfNetplay");
}
JOBJSet ***Netplay_GetPlyNumSet()
{
    JOBJSet ***plynum_set = (JOBJSet ***)(&((u8 *)Gm_Get3dData())[0xc0]);

    // if we've loaded the alt plynum file, use it in place of the original
    if (netplay_archive)
    {
        JOBJSet **set = Archive_GetPublicAddress(netplay_archive, "ScInfPlynum_scene_models");
        if (set)
            *plynum_set = set;
    }
        
    return plynum_set;
}
CODEPATCH_HOOKCREATE(0x8011fd4c, "", Netplay_GetPlyNumSet, 
                    "mr 30, 3\n\t"
                    "cmpwi	27, 1\n\t"
                    "b 0x8\n\t", 0)
void Netplay_AdjustPlyNum(JOBJ *j)
{
    if (!netplay_archive)
        return;
        
    // the more_colors mod only edits the color dobj 0, copy that to the newly added dobj 1
    JObj_GetDObjIndex(j, 0, 1)->mobj->mat->diffuse = JObj_GetDObjIndex(j, 0, 0)->mobj->mat->diffuse;

}
CODEPATCH_HOOKCREATE(0x8011fe94, "mr 3,28\n\t",
                     Netplay_AdjustPlyNum, "mulli	3, 26, 20\n\t", 0)


// Player Tags
void Netplay_CreateViewportTag(StarpoleDataDolphin *data)
{
    if (!data->netplay.is)
        return;

    if (Gm_GetPlyViewNum() == 1)
        return;

    Game3dData *g3d = Gm_Get3dData();
    int canvas_idx = Text_CreateCanvas(0, 1, 0, 0, 0, GAMEGX_HUD, 1, 0);

    // loop through all player views
    for (int i = 0; i < 4; i++)
    {
        if (Ply_GetPKind(i) == PKIND_NONE || !Ply_IsViewOn(i))
            continue;

        GOBJ *plynm_gobj = g3d->plynm_gobj[i];
        if (!plynm_gobj)
            continue;

        // create a gobj to manage the tag for this viewport
        GOBJ *g = GOBJ_EZCreator(27, GAMEPLINK_HUD, 0,
                        sizeof(PlayerTagViewData), Netplay_DestroyViewportTagViewGObj,
                        0, 0,
                        0, 0, 
                        Netplay_ViewportTagGX, GAMEGX_HUD, 0);

        // init data
        ViewportTagViewData *gp = g->userdata;
        gp->ply = i;
        gp->t = 0;

        // create text
        char name[NETPLAY_TAGMAX + 1];
        strncpy(name, data->netplay.usernames[i], NETPLAY_TAGMAX);
        name[NETPLAY_TAGMAX] = '\0';

        // create a name for them
        Text *t = Text_CreateText(0, canvas_idx);
        t->gobj->gx_cb = 0;
        t->viewport_scale = (Vec2){0.05, 0.05};
        t->trans = (Vec3){0, 0, 0};
        t->viewport_color = (GXColor){0,0,0,0};
        t->aspect = (Vec2){150,32};
        t->use_aspect = 1;
        t->align = 1;
        t->kerning = 1;

        // get text color
        GXColor *text_color;
        DOBJ *plynum_dobj = JObj_GetDObjIndex(plynm_gobj->hsd_object, 2, 0);
        if (plynum_dobj)
            text_color = &plynum_dobj->mobj->mat->diffuse;

        // get outline color
        GXColor *outline_color;
        outline_color = UI_GetTextOutlineColor(text_color);

        Text_AddOutline(t, outline_color, name);

        t->color = *text_color;
        Text_AddSubtext(t, 0, 0, name);

        gp->t = t;
    }
}
void Netplay_DestroyViewportTagViewGObj(ViewportTagViewData *gp)
{
    for (int i = 0; i < GetElementsIn(gp->t); i++)
    {
        if (gp->t)
            Text_Destroy(gp->t);
    }
}
void Netplay_ViewportTagGX(GOBJ *g, int pass)
{
    // only on transparency pass
    if (pass != 2)
        return; 

    Game3dData *g3d = Gm_Get3dData();
    ViewportTagViewData *gp = g->userdata;
    Text* t = gp->t;

    // get plynum hud gobj
    GOBJ* plynm_gobj = g3d->plynm_gobj[gp->ply];
    if (!plynm_gobj)
        return;

    HUDElementData* hp = plynm_gobj->userdata;

    // check if plynum was rendered
    if (!hp->is_visible)
        t->hidden = 1;
    else
    {
        t->hidden = 0;
        static ViewportTagParam tag_param_2p = {.offset = {0, -1.9}, .scale = 0.9, .width = 140};
        static ViewportTagParam tag_param_4p = {.offset = {0.2, -1.2}, .scale = 0.7, .width = 120};

        ViewportTagParam *param = (Gm_GetPlyViewNum() == 2) ? &tag_param_2p : &tag_param_4p;

        // move text to PlyNum
        Vec3 plynm_pos;
        JObj_GetChildPosition(plynm_gobj->hsd_object, 2, &plynm_pos);
        t->trans.X = plynm_pos.X + param->offset.X;
        t->trans.Y = -(plynm_pos.Y + param->offset.Y);
        t->viewport_scale.X = 0.05 * param->scale;
        t->viewport_scale.Y = 0.05 * param->scale;
        t->aspect.X = param->width;
    }

    // splitscreen logic
    if (g3d->plyview_num >= 2)
    {
        CamScissor view_scissor;
        if (g3d->plyview_num == 2)
            PlyCam_Get2PScissor(Ply_GetViewIndex(hp->ply), &view_scissor);
        else
            PlyCam_Get4PScissor(Ply_GetViewIndex(hp->ply), &view_scissor);
        
        GXSetScissor(view_scissor.left, view_scissor.top, view_scissor.right - view_scissor.left,
                    view_scissor.bottom - view_scissor.top);
    }

    // call render func
    Text_GX(t->gobj, pass);

    // restore scissor
    CamScissor full_scissor;
    PlyCam_GetFullscreenScissor(&full_scissor);
    GXSetScissor(full_scissor.left, 
                full_scissor.top, 
                full_scissor.right - full_scissor.left,
                full_scissor.bottom - full_scissor.top);
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

GXColor *UI_GetTextOutlineColor(GXColor *color)
{
    // name outline
    int y = (color->r * 299 + color->g * 587 + color->b * 114) / 1000;
    if (y < 80)
    {
        static GXColor white = {255, 255, 255, 255};
        return &white;
    }
    else
    {
        static GXColor black = {0, 0, 0, 255};
        return &black;
    }
}
void Text_AddOutline(Text *t, GXColor *color, char *s)
{
    static float offsets[4][2] = {
        {-2, -2},
        {-2, 2},
        {2, 2},
        {2, -2},
    };

    t->color = *color;

    for (int i = 0; i < GetElementsIn(offsets); i++)
        Text_AddSubtext(t, offsets[i][0], offsets[i][1], s);
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

// Init
void Dolphin_Init()
{
    if (!Starpole_IsPresent() && !DOLPHIN_DEBUG)
        return;

    // alloc buffer
    dolphin_data = HSD_MemAlloc(sizeof(*dolphin_data));
    is_dolphin = 0;
    
    if (DOLPHIN_DEBUG)
    {
        static char *test_names[] = {
            "UnclePunch",
            "charity",
            "Taco",
            "ThePulsarLegend",
        };

        is_dolphin = 1;
        dolphin_data->aspect_mult = 1;
        dolphin_data->netplay.is = 1;
        dolphin_data->netplay.ply = 0;
        dolphin_data->netplay.rng_seed = 0;
        for (int i = 0; i < GetElementsIn(dolphin_data->netplay.usernames); i++)
            strcpy(dolphin_data->netplay.usernames[i], test_names[i]);
    }

    // get data
    if (DOLPHIN_DEBUG || Dolphin_ReqData(dolphin_data))
    {
        is_dolphin = 1;
        OSReport("Starpole: Dolphin detected.\n");
        
        // store pointer to export data
        starpole_export.dolphin_data = dolphin_data;

        // init netplay flag
        if (dolphin_data->netplay.is)
            Netplay_Init();
        else
            OSReport("Starpole: Netplay not detected.\n");

        // hijack code that creates PlyNum model
        CODEPATCH_HOOKAPPLY(0x8011fd4c);
        CODEPATCH_HOOKAPPLY(0x8011fe94);
    }
}
void Netplay_Init()
{
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
}

void Dolphin_OnSceneChange()
{
    netplay_archive = 0;
}
void Netplay_On3DLoadStart()
{
    if (!Dolphin_IsNetplay())
        return;
    
    Netplay_LoadAltPlyNumFile();
    Netplay_OverridePlayerView(dolphin_data);
}
void Netplay_On3DLoadEnd()
{
    if (!Dolphin_IsNetplay())
        return;

    Netplay_CreatePlayerTags(dolphin_data);

    // show onscreen names if someone is unplugged (spectating)
    if (dolphin_data->netplay.ply == -1)
        Netplay_CreateViewportTag(dolphin_data);

    if (music_export && dolphin_data->netplay.ply != -1)
    {
        GOBJ_EZCreator(0, 0, 0,
                    0, 0,
                    0, 0, 
                    Netplay_MusicChange, stc_gobj_init_data->proc_pri_max - 1,
                    0, 0, 0);
    }
}