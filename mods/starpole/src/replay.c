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
    if (Starpole_Imm(STARPOLE_CMD_REQMATCH, 0) <= 0)
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
    if (frame_size <= 0)
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

void Playback_OnFrameStart()
{
    bp();

    // request frame
    if (!Replay_ReqFrame(frame_idx))
    {
        // error getting the frame, lets end the game
        Scene_SetDirection(PAD_BUTTON_B);
        Scene_ExitMinor();
        BGM_Stop();
        FGM_StopAll();
        Pad_StopRumbleAll();
        Gm_Pause(1); // avoid having all the gobj proc update after this?

        // OSReport("Replay: No frame received, ending game\n");

        return;
    }
    
    // desync detection
    if (starpole_buf->frame.rng_seed != *hsd_rand_seed)
    {
        OSReport("Replay: ERROR Random seed mismatch on frame %d!\n", frame_idx);
        if (!desync_text)
            Replay_CreateDesyncText(frame_idx);
    }

    // *hsd_rand_seed = starpole_buf->frame.rng_seed;

    return;
}
void Playback_OnRiderInput(RiderData *rd)
{
    if (starpole_buf->frame.frame_idx != frame_idx)
    {
        OSReport("Replay: frame index mismatch. Expected %d, received %d\n", frame_idx, starpole_buf->frame.frame_idx);
        assert("0");
    }

    // find the data for this ply
    for (int i = 0; i < starpole_buf->frame.ply_num; i++)
    {
        int ply = starpole_buf->frame.ply[i].idx;

        if (ply != rd->ply)
            continue;

        // copy inputs
        rd->input.held = (int)starpole_buf->frame.ply[ply].input.held << 4; // game code will update down for us (8018f178)
        rd->input.stickX = starpole_buf->frame.ply[ply].input.stickX;       // game code will convert to float and update the Vec2 (8018f154)
        rd->input.stickY = starpole_buf->frame.ply[ply].input.stickY;
        rd->input.rstick.X = (float)starpole_buf->frame.ply[ply].input.substickX / 80.0f;
        rd->input.rstick.Y = (float)starpole_buf->frame.ply[ply].input.substickY / 80.0f;

        return;
    }

    OSReport("Replay: ERROR no frame data found for ply %d\n", rd->ply);
    assert("0");
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

RiderData *PlyCam_GetRiderData(CamData *cd)
{
    // embark on a journey of epic proportions to find this player cam gobj
    // because the function doesnt pass it in.
    for (int i = 0; i < CM_CAMERA_MAX; i++)
    {
        GOBJ *g = stc_plycam_lookup->cam_gobjs[i];

        if (!g)
            continue;

        PlayerCamData *gp = g->userdata;
        if (gp->cam_data != cd)
            continue;

        // ok we found it
        GOBJ *r = Ply_GetRiderGObj(gp->ply);
        RiderData *rd = r->userdata;

        return rd;
    }

    return 0;
}
float PlyCam_ClampStick(float val)
{
    float min = 0.4;

    if (val < -min)
            val += min; 
    else if (val > min)
        val -= min; 
        
    return val / (1.0 - min);
}
void PlyCam_UseRiderInputsForMachineCameraControl(CamData *cam_data, int controller_idx, float *limits)
{
    if (!cam_data->x94)
    {
        cam_data->x84_80 = 0;
        return;
    }

    cam_data->x84_80 = 1;

    RiderData *rd = PlyCam_GetRiderData(cam_data);
    float rstickX = PlyCam_ClampStick(rd->input.rstick.X);
    float rstickY = PlyCam_ClampStick(rd->input.rstick.Y);

    float *tuning = (float *)stc_plycam_lookup->x234;

    /* ---------- ROTATION (YAW) ---------- */

    float desired = -rstickX * limits[0]; // yaw speed scalar
    float cur = cam_data->rotation_amt;
    float max_step = M_1DEGREE * tuning[0x334/4];
    float delta = (desired - cur) * tuning[0x330/4];

    if (delta > max_step)
        cam_data->rotation_amt = cur + max_step;
    else if (delta < -max_step)
        cam_data->rotation_amt = cur - max_step;
    else
        cam_data->rotation_amt = cur + delta;

    // zoom and pitch

    if (rstickY != 0.0f)
        cam_data->zoom_amt += -rstickY * tuning[0x32c/4];; // inferred

    // clamp zoom
    float z = cam_data->zoom_amt;
    if (z < limits[1])
        z = limits[1];
    else if (z > limits[2])
        z = limits[2];
    cam_data->zoom_amt = z;

    // normalized ratio
    if (z < 0.0f)
    {
        cam_data->x90 = 0.0f;
        return;
    }

    cam_data->x90 = limits[3] * (z / limits[2]);
}

////////////////
// Injections //
////////////////

// Injection to fetch the frame
void Playback_GetFrame()
{
    if (is_active && replay_mode == REPLAY_PLAYBACK)
        Playback_OnFrameStart();
    
    return;
}
CODEPATCH_HOOKCREATE(0x80012e74, "", Playback_GetFrame, "", 0)

// Injection to write inputs directly to rider data
int Playback_RiderInputRestore(RiderData *rd)
{
    if (is_active && replay_mode == REPLAY_PLAYBACK)
    {
        Playback_OnRiderInput(rd);
        return 1;
    }
    
    return 0;
}
CODEPATCH_HOOKCONDITIONALCREATE(0x8018ef34, "mr 3, 31\n\t", Playback_RiderInputRestore, "", 0, 0x8018effc)


// Injection to make the kirby on foot camera use the rider input data (we restore this)
float PlyCam_UseRiderInputsForOnFootCameraControl(CamData *cam_data)
{
    RiderData *rd = PlyCam_GetRiderData(cam_data);

    float rstickX = rd->input.rstick.X;
    float min = 0.4;

    if (rstickX < -min)
            rstickX += min; 
    else if (rstickX > min)
        rstickX -= min; 
        
    return rstickX / (1.0 - min);
}
CODEPATCH_HOOKCREATE(0x800cb4c8, "mr 3, 29\n\t", PlyCam_UseRiderInputsForOnFootCameraControl, "fmr 2, 1\n\t", 0)

// Injection to avoid directly referencing the current cobj's position when dismounting a machine
// (the active cobj should not impact gameplay, this allows us to freecam)
void Dismount_GetCameraPosition(CamData *cd)
{
    memcpy(&cd->xe8.interest, &cd->interest_pos, sizeof(cd->xe8.interest)); // interest pos
    memcpy(&cd->xe8.eye, &cd->eye_pos, sizeof(cd->xe8.eye)); // eye pos
}
CODEPATCH_HOOKCREATE(0x800b7840, "mr 3, 30\n\t", Dismount_GetCameraPosition, "", 0)

// Mod Callbacks
void Replay_OnBoot()
{
    CODEPATCH_HOOKAPPLY(0x800b7840);
    CODEPATCH_HOOKAPPLY(0x8018ef34);
    CODEPATCH_HOOKAPPLY(0x800cb4c8);
    CODEPATCH_HOOKAPPLY(0x80012e74);
    CODEPATCH_REPLACEFUNC(0x800b67cc, PlyCam_UseRiderInputsForMachineCameraControl);
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
        // GObj_AddProc(g, Playback_OnFrameInputs, RDPRI_INPUT);
        GObj_AddProc(g, Playback_OnFrameEnd, RDPRI_15 + 1);

        // use live view camera
        GameData *gd = Gm_GetGameData();
        gd->ply_view_desc[0].flag = PLYCAM_ON;
    }

    Replay_CreateFrameText();

}
void Replay_On3DExit()
{
    if (is_active && replay_mode == REPLAY_RECORD)
        Replay_SendEnd();
}