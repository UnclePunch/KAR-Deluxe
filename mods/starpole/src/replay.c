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
#include "replay.h"
#include "code_patch/code_patch.h"
#include "text_joint/text_joint.h"

extern StarpoleBuffer *starpole_buf;
int frame_idx;
int is_active = 0;

int replay_frame_size;
ReplayMode replay_mode = REPLAY_PLAYBACK;

Text *cam_text = 0;
Text *Replay_CreateCamText()
{
    // display test string
    Text *t = Hoshi_CreateScreenText();
    t->kerning = 1;
    t->use_aspect = 1;
    t->trans = (Vec3){10, 30, 0};
    t->viewport_scale = (Vec2){0.5, 0.5};
    t->aspect = (Vec2){400, 64};
    t->viewport_color = (GXColor){0, 0, 0, 128};

    return t;
}
Text *frame_text;
void Replay_CreateFrameText()
{
    // display test string
    Text *t = Hoshi_CreateScreenText();
    t->kerning = 1;
    t->use_aspect = 1;
    t->trans = (Vec3){450, 30, 0};
    t->viewport_scale = (Vec2){0.5, 0.5};
    t->aspect = (Vec2){260, 32};
    t->viewport_color = (GXColor){0, 0, 0, 128};
    Text_AddSubtext(t, 0, 0, "");

    frame_text = t;
}
Text *desync_text;
void Replay_CreateDesyncText(int frame)
{
    // display test string
    Text *t = Hoshi_CreateScreenText();
    t->kerning = 1;
    t->use_aspect = 1;
    t->trans = (Vec3){0, 0, 0};
    t->viewport_scale = (Vec2){0.5, 0.5};
    t->aspect = (Vec2){260, 32};
    t->color = (GXColor){255, 128, 128, 255};
    t->viewport_color = (GXColor){0, 0, 0, 128};
    Text_AddSubtext(t, 0, 0, "Desynced on frame %d!\n", frame);

    desync_text = t;
}

int Replay_SendMatch()
{
    // notify EXI of incoming data
    if (Starpole_Imm(STARPOLE_CMD_MATCH, 0) == -1)
    {
        OSReport("Replay: error sending match\n");
        return 0;
    }

    GameData *gp = Gm_GetGameData();

    // calculate size of frame
    int hmn_num = 0;
    for (int i = 0; i < 4; i++)
    {
        if (gp->ply_desc[i].p_kind == PKIND_HMN)
            hmn_num++;
    }

    replay_frame_size = sizeof(starpole_buf->frame.rng_seed) +
                        sizeof(starpole_buf->frame.frame_idx) +
                        sizeof(starpole_buf->frame.ply_num) +
                        sizeof(starpole_buf->frame.ply[0]) * hmn_num;

    // initialize outgoing buffer
    starpole_buf->match.rng_seed = *hsd_rand_seed;
    starpole_buf->match.frame_size = replay_frame_size;
    starpole_buf->match.stadium_kind = gp->city.stadium_kind;
    memcpy(starpole_buf->match.misc, &gp->city_kind, sizeof(starpole_buf->match.misc));
    memcpy(starpole_buf->match.ply_desc, gp->ply_desc, sizeof(starpole_buf->match.ply_desc));

    // send it
    if (!Starpole_DMA(starpole_buf, sizeof(starpole_buf->match), EXI_WRITE))
        return 0;

    return 1;
}
int Replay_SendFrame(int frame_idx)
{
    // notify EXI of incoming data
    if (Starpole_Imm(STARPOLE_CMD_FRAME, frame_idx) == -1)
        return 0;

    // send it
    if (!Starpole_DMA(starpole_buf, replay_frame_size, EXI_WRITE))
        return 0;

    return 1;
}
int Replay_SendEnd()
{
    // notify EXI of incoming data
    if (Starpole_Imm(STARPOLE_CMD_END, 0) == -1)
        return 0;

    return 1;
}

int Replay_ReqMatch()
{
    // request data
    if (Starpole_Imm(STARPOLE_CMD_REQMATCH, 0) == -1)
    {
        OSReport("Replay: error receiving match\n");
        return 0;
    }

    // receive it
    if (!Starpole_DMA(starpole_buf, sizeof(starpole_buf->match), EXI_READ))
        return 0;

    // copy to game struct
    GameData *gp = Gm_GetGameData();
    *hsd_rand_seed = starpole_buf->match.rng_seed;
    gp->city.stadium_kind = starpole_buf->match.stadium_kind;
    memcpy(&gp->city_kind, starpole_buf->match.misc, sizeof(starpole_buf->match.misc));
    memcpy(gp->ply_desc, starpole_buf->match.ply_desc, sizeof(starpole_buf->match.ply_desc));

    return 1;
}
int Replay_ReqFrame(int frame_idx)
{
    // request data
    int frame_size = Starpole_Imm(STARPOLE_CMD_REQFRAME, frame_idx);
    if (frame_size == -1)
        return 0;

    // receive it
    if (!Starpole_DMA(starpole_buf, frame_size, EXI_READ))
        return 0;

    return 1;
}

void Record_OnFrameStart(GOBJ *g)
{
    starpole_buf->frame.rng_seed = *hsd_rand_seed;
    return;
}
void Record_OnFrameInputs(GOBJ *g)
{
    // log player inputs
    int ply = 0;
    for (int i = 0; i < 4; i++)
    {
        if (Ply_GetPKind(i) == PKIND_HMN)
        {
            GOBJ *r = Ply_GetRiderGObj(i);
            RiderData *rd = r->userdata;

            // index of this player
            starpole_buf->frame.ply[ply].idx = ply;

            // copy buttons
            starpole_buf->frame.ply[ply].input.held = (stc_engine_pads[i].held & 0x00000FF0) >> 4;
            starpole_buf->frame.ply[ply].input.stickX = stc_engine_pads[i].stickX;
            starpole_buf->frame.ply[ply].input.stickY = stc_engine_pads[i].stickY;
            starpole_buf->frame.ply[ply].input.substickX = stc_engine_pads[i].substickX;
            starpole_buf->frame.ply[ply].input.substickY = stc_engine_pads[i].substickY;

            ply++;
        }
    }

    // log amount of players we have data for
    starpole_buf->frame.ply_num = ply;
    starpole_buf->frame.frame_idx = frame_idx;
}
void Record_OnFrameEnd(GOBJ *g)
{
    // log player state
    int ply = 0;
    for (int i = 0; i < 4; i++)
    {
        if (Ply_GetPKind(i) != PKIND_NONE)
        {
            GOBJ *r = Ply_GetRiderGObj(i);
            RiderData *rd = r->userdata;

            // get machine
            GOBJ *m = rd->machine_gobj;
            int machine_kind;
            if (m)
            {
                MachineData *md = m->userdata;
                machine_kind = md->kind;
            }
            else
                machine_kind = -1;

            // // copy state
            // starpole_buf->frame.ply[ply].rd_state = rd->state_idx;
            // starpole_buf->frame.ply[ply].machine_kind = machine_kind;
            // starpole_buf->frame.ply[ply].pos = rd->pos;

            ply++;
        }
    }

    Replay_SendFrame(frame_idx);

    Text_SetText(frame_text, 0, "Frame: %d", frame_idx);
    frame_idx++;

    int machine_num = 0;
    for (GOBJ *m = (*stc_gobj_lookup)[GAMEPLINK_MACHINE]; m; m = m->next)
    {
        MachineData *md = m->userdata;

        OSReport("#%d: kind: %d  pos: (%.2f, %.2f, %.2f)\n", 
                    machine_num + 1, 
                    md->kind, 
                    md->pos.X, md->pos.Y, md->pos.Z);

        machine_num++;
    }    
}

void Playback_OnFrameStart(GOBJ *g)
{
    // request frame
    Replay_ReqFrame(frame_idx);
    
    if (starpole_buf->frame.rng_seed != *hsd_rand_seed)
    {
        OSReport("Replay: ERROR Random seed mismatch on frame %d!\n", frame_idx);
        if (!desync_text)
            Replay_CreateDesyncText(frame_idx);
    }

    // *hsd_rand_seed = starpole_buf->frame.rng_seed;

    return;
}
void Playback_OnFrameInputs(GOBJ *g)
{
    if (starpole_buf->frame.frame_idx != frame_idx)
    {
        OSReport("Replay: frame index mismatch\n");
        assert("0");
    }

    // update player inputs
    int ply = 0;
    for (int i = 0; i < 4; i++)
    {
        if (Ply_GetPKind(i) == PKIND_HMN)
        {
            // copy buttons
            stc_engine_pads[i].held = (int)starpole_buf->frame.ply[ply].input.held << 4;
            stc_engine_pads[i].stickX = starpole_buf->frame.ply[ply].input.stickX;
            stc_engine_pads[i].stickY = starpole_buf->frame.ply[ply].input.stickY;
            stc_engine_pads[i].substickX = starpole_buf->frame.ply[ply].input.substickX;
            stc_engine_pads[i].substickY = starpole_buf->frame.ply[ply].input.substickY;

            // update float value too because rider input callback reads that instead of the byte...
            stc_engine_pads[i].fsubstickX = (float)stc_engine_pads[i].substickX / 80.0f;
            stc_engine_pads[i].fsubstickY = (float)stc_engine_pads[i].substickY / 80.0f;

            ply++;
        }
    }
}
void Playback_OnFrameEnd(GOBJ *g)
{ 

    OSReport("Frame %d:\n", frame_idx);

    // update player inputs
    int ply = 0;
    for (int i = 0; i < 4; i++)
    {   
        if (Ply_GetPKind(i) == PKIND_HMN)
        {
            GOBJ *r = Ply_GetRiderGObj(i);
            RiderData *rd = r->userdata;

            OSReport(" Ply %d:\n", i + 1);
            OSReport("  State:   %d (%d)\n", rd->state_idx, rd->state_idx);
            OSReport("  Pos:     (%.2f, %.2f, %.2f)\n", rd->pos.X, rd->pos.Y, rd->pos.Z);
            OSReport("  Machine: %p\n", rd->machine_gobj);

            ply++;
        }
    }

    Text_SetText(frame_text, 0, "Frame: %d", frame_idx);
    frame_idx++;

}

void Dismount_GetCameraPosition(CamData *cd)
{
    memcpy(&cd->xe8.interest, &cd->interest_pos, sizeof(cd->xe8.interest)); // interest pos
    memcpy(&cd->xe8.eye, &cd->eye_pos, sizeof(cd->xe8.eye)); // eye pos
}
CODEPATCH_HOOKCREATE(0x800b7840, "mr 3, 30\n\t", Dismount_GetCameraPosition, "", 0)

void Replay_OnBoot()
{
    CODEPATCH_HOOKAPPLY(0x800b7840);
}
void Replay_On3DLoadStart()
{
    if (!Starpole_IsPresent())
        return;

    // debug
    if (stc_engine_pads[0].held & PAD_BUTTON_A)
        replay_mode = REPLAY_RECORD;
    else 
        replay_mode = REPLAY_PLAYBACK;

    // send/receive initial match data
    int result;
    if (replay_mode == REPLAY_PLAYBACK)
        result = Replay_ReqMatch();
    else
        result = Replay_SendMatch();

    if (!result)
    {
        is_active = 0;
        return;
    }

    // proceed with EXI comms throughout this match
    is_active = 1;

    // create a gobj to transmit per frame match data
    GOBJ *g = GOBJ_EZCreator(0, GAMEPLINK_1, 0, 
                    0, 0,
                    0, 0, 
                    0, 0,
                    0, 0, 0);

    frame_idx = 0;

    if (replay_mode == REPLAY_RECORD)
    {
        GObj_AddProc(g, Record_OnFrameStart, RDPRI_0);
        GObj_AddProc(g, Record_OnFrameInputs, RDPRI_INPUT + 1);
        GObj_AddProc(g, Record_OnFrameEnd, RDPRI_15 + 1);
    }
    else if (replay_mode == REPLAY_PLAYBACK)
    {
        GObj_AddProc(g, Playback_OnFrameStart, RDPRI_0);
        GObj_AddProc(g, Playback_OnFrameInputs, RDPRI_INPUT);
        GObj_AddProc(g, Playback_OnFrameEnd, RDPRI_15 + 1);
    }

    Replay_CreateFrameText();

}
void Replay_On3DExit()
{
    if (is_active && replay_mode == REPLAY_RECORD)
        Replay_SendEnd();
}