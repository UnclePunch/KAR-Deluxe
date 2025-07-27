
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

#include "unpause.h"

// SignalGo
JOBJSet **signalgo_set = 0;
GOBJ *SignalGo_Create(int ply)
{
    Game3dData *g3d = Gm_Get3dData();

    GOBJ *g = GOBJ_EZCreator(27, GAMEPLINK_PAUSEHUD, 0,
                             0, 0,
                             HSD_OBJKIND_JOBJ, signalgo_set[0]->jobj,
                             SignalGo_Anim, 20,
                             HUD_GXLink, 21, 1);
    HUD_AddElementData(g, sizeof(SignalGoData), ply, ply);

    SignalGoData *gp = g->userdata;
    gp->timer = 0;
    gp->state = 0;

    // adjust position for multiple viewports
    JOBJ *j = g->hsd_object;
    PlyViewPosData *plyview_data = g3d->plyview_pos_gobj->userdata;
    int anim_id;
    if (g3d->plyview_num == 1)
        anim_id = 0;
    if (g3d->plyview_num > 1)
    {
        j->trans = plyview_data->plyview_center_pos[Ply_GetViewIndex(ply)];
        JObj_SetMtxDirtySub(j);

        if (g3d->plyview_num == 2)
            anim_id = 1;
        else
            anim_id = 2;
    }

    JObj_AddSetAnim(g->hsd_object, anim_id, signalgo_set[0], 0, 0);

    return g;
}
void SignalGo_Anim(GOBJ *g)
{
    SignalGoData *gp = g->userdata;

    JObj_AnimAll(g->hsd_object);

    switch (gp->state)
    {
    case (0):
    {
        // start delay
        gp->timer++;
        if (gp->timer >= SIGNALGO_STARTDELAY)
        {
            // init vars
            gp->timer = 0;
            gp->state = 1;

            // begin animation
            JObj_SetFrameAndRate(g->hsd_object, 0, SIGNALGO_ANIMRATE);
        }

        break;
    }
    case (1):
    {
        gp->timer++;
        if (gp->timer > (int)(80 / SIGNALGO_ANIMRATE))
            GObj_Destroy(g);

        break;
    }
    }

    // if (!JObj_CheckAObjPlaying(g->hsd_object))
    //     GObj_Destroy(g);

    return;
}

// Unpause Delay
int unpause_delay_enabled = 1;
int is_unpause_delay = 0;
int unpause_timer = 0;

int UnpauseDelay_Wait_Hook()
{
    if (is_unpause_delay)
    {
        unpause_timer++;

        // ugly hardcoded frame indices. forget you saw this.
        if (unpause_timer == (int)((4 + SIGNALGO_STARTDELAY) / SIGNALGO_ANIMRATE) ||
            unpause_timer == (int)((24 + SIGNALGO_STARTDELAY) / SIGNALGO_ANIMRATE) ||
            unpause_timer == (int)((44 + SIGNALGO_STARTDELAY) / SIGNALGO_ANIMRATE))
            SFX_Play(FGMMAIN_INFO_TIMECOUNT1);
        else if (unpause_timer == (int)((64 + SIGNALGO_STARTDELAY) / SIGNALGO_ANIMRATE))
        {
            SFX_Play(FGMMAIN_INFO_TIMECOUNT2);

            GameData *gd = Gm_GetGameData();

            Gm_ResumeAllSFX();
            BGM_RaiseVolume();
            Gm_ShowHUD();
            gd->pause_cursor = -1;
            gd->pause_delay = 16;

            // there is a condition where pause_kind is not 1... @ 80041234.
            int pause_kind = 1; // (Scene_GetCurrentMajor() == MJRKIND_AIR && something) ? 2 : 1;
            Gm_Resume(pause_kind);

            is_unpause_delay = 0;
        }

        return 1; // exit game's pause function
    }
    else
    {
        return 0; // continue running game's pause function
    }
}
CODEPATCH_HOOKCONDITIONALCREATE(0x80041188, "", UnpauseDelay_Wait_Hook, "", 0, 0x800415d8)

int UnpauseDelay_Enter_Hook(HSD_Pad *pad)
{
    if (!unpause_delay_enabled)
        return 0;

    // count humans present
    int hmn_num = 0;
    for (int i = 0; i < 5; i++)
        (Ply_GetPKind(i) == PKIND_HMN) ? (hmn_num++) : 0;

    if (hmn_num < 2)
        return 0;

    is_unpause_delay = 1;
    unpause_timer = 0;

    // remove hud, restore camera, play sfx
    Gm_PlayPauseSFX();

    BGM_LowerVolume();
    Gm_HidePauseHUD();
    Gm_SetCameraNormal();

    Game3dData *g3d = Gm_Get3dData();

    for (int i = 0; i < 4; i++)
    {
        if (g3d->plyview_lookup[i] != -1)
            SignalGo_Create(i);
    }

    return 1;
}
CODEPATCH_HOOKCONDITIONALCREATE(0x800413b0, "", UnpauseDelay_Enter_Hook, "", 0, 0x800415d8)
CODEPATCH_HOOKCONDITIONALCREATE(0x800411f8, "", UnpauseDelay_Enter_Hook, "", 0, 0x800415d8)
CODEPATCH_HOOKCONDITIONALCREATE(0x800411d4, "", UnpauseDelay_Enter_Hook, "", 0, 0x800415d8)

void UnpauseDelay_Init()
{
    // apply our code patches
    CODEPATCH_HOOKAPPLY(0x80041188);
    CODEPATCH_HOOKAPPLY(0x800413b0);
    CODEPATCH_HOOKAPPLY(0x800411f8);
    CODEPATCH_HOOKAPPLY(0x800411d4);
}
void UnpauseDelay_On3DStart()
{
    // init our variable
    is_unpause_delay = 0;

    if (!unpause_delay_enabled)
        return;

    // get our file
    HSD_Archive *archive;
    Gm_LoadGameFile(&archive, "IfAllCountdown");
    signalgo_set = Archive_GetPublicAddress(archive, "ScInfSignalGo1_scene_models");
}
