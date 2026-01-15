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
#include "playback.h"
#include "code_patch/code_patch.h"
#include "text_joint/text_joint.h"

extern StarpoleBuffer *starpole_buf;
int frame_idx;

int replay_frame_size;
ReplayMode replay_mode = REPLAY_NONE;

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
Text *debug_text[4];
void Replay_Debug(GOBJ *g)
{
    for (int ply = 0; ply < 1; ply++)
    {
        GOBJ *plycam_gobj = stc_plycam_lookup->cam_gobjs[ply];
        if (!plycam_gobj)
            continue;

        if (debug_text[ply])
            Text_Destroy(debug_text[ply]);

        PlayerCamData *plycam_data = plycam_gobj->userdata;
        CamData *cd = plycam_data->cam_data;

        // display string
        Text *t = Hoshi_CreateScreenText();
        t->kerning = 1;
        // t->use_aspect = 1;
        t->viewport_scale = (Vec2){0.2, 0.2};
        t->trans = (Vec3){0, (ply * 250) + (15), 0};
        // t->aspect = (Vec2){260, 32};
        t->color = (GXColor){255, 255, 255, 255};
        t->viewport_color = (GXColor){0, 0, 0, 128};
        float y_pos = 0;

        // CameraParam *param = (CameraParam *)&cd->xc0;
        // for (int i = 0; i < 4; i++)
        // {   
        //     Text_AddSubtext(t, 0, y_pos, "0x%x:", 0xc0 + (i * sizeof(CameraParam)));
        //     y_pos += 30;
        //     Text_AddSubtext(t, 30, y_pos, "eye:");
        //     Text_AddSubtext(t, 130, y_pos, "%.2f, %.2f, %.2f", param[i].eye.X, param[i].eye.Y, param[i].eye.Z);
        //     y_pos += 30;
        //     Text_AddSubtext(t, 30, y_pos, "int:");
        //     Text_AddSubtext(t, 130, y_pos, "%.2f, %.2f, %.2f", param[i].interest.X, param[i].interest.Y, param[i].interest.Z);
        //     y_pos += 30;
        // }
        // y_pos += 30;

        GOBJ *r = Ply_GetRiderGObj(ply);
        RiderData *rd = r->userdata;

        Text_AddSubtext(t, 0, y_pos, "inputs:");
        y_pos += 30;
        Text_AddSubtext(t, 30, y_pos, "held:");
        Text_AddSubtext(t, 170, y_pos, "0x%08X", rd->input.held);
        y_pos += 30;
        Text_AddSubtext(t, 30, y_pos, "lstick:");
        Text_AddSubtext(t, 170, y_pos, "%.2f, %.2f", rd->input.lstick.X, rd->input.lstick.Y);
        y_pos += 30;
        Text_AddSubtext(t, 30, y_pos, "rstick:");
        Text_AddSubtext(t, 170, y_pos, "%.2f, %.2f", rd->input.rstick.X, rd->input.rstick.Y);
        y_pos += 30;
        Text_AddSubtext(t, 30, y_pos, "trigger:");
        Text_AddSubtext(t, 170, y_pos, "%.2f", rd->input.trigger);
        y_pos += 30;

        t->aspect = (Vec2){450, y_pos};

        debug_text[ply] = t;
    }
}

s8 denormalize_signed(float val)
{
    return (s8)(val * 80.0f);
}
float normalize_signed(s8 val)
{
    return (float)val / 80.0f;
}
u8 denormalize_unsigned(float val)
{
    return (u8)(val * 140.0f);
}
float normalize_unsigned(u8 val)
{
    return (float)val / 140.0f;
}

int Replay_SendMatch()
{
    int result = 0;
    int enable = OSDisableInterrupts();
    
    // notify EXI of incoming data
    if (Starpole_Imm(STARPOLE_CMD_MATCH, 0) == -1)
    {
        OSReport("Replay: error sending match\n");
        goto CLEANUP;
    }

    // send it
    if (!Starpole_DMA(starpole_buf, sizeof(starpole_buf->match), EXI_WRITE))
        goto CLEANUP;

    result = 1;

CLEANUP:
    OSRestoreInterrupts(enable);
    return result;
}
int Replay_SendFrame(int frame_idx)
{
    int result = 0;
    int enable = OSDisableInterrupts();

    // notify EXI of incoming data
    if (Starpole_Imm(STARPOLE_CMD_FRAME, frame_idx) == -1)
        goto CLEANUP;

    // send it
    if (!Starpole_DMA(starpole_buf, replay_frame_size, EXI_WRITE))
        goto CLEANUP;

    result = 1;

CLEANUP:
    OSRestoreInterrupts(enable);
    return result;
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
    int result = 0;
    int enable = OSDisableInterrupts();

    // request data
    if (Starpole_Imm(STARPOLE_CMD_REQMATCH, 0) <= 0)
    {
        OSReport("Replay: error receiving match\n");
        goto CLEANUP;
    }

    // receive it
    if (!Starpole_DMA(starpole_buf, sizeof(starpole_buf->match), EXI_READ))
        goto CLEANUP;

    result = 1;

CLEANUP:
    OSRestoreInterrupts(enable);
    return result;
}
int Replay_ReqFrame(int frame_idx)
{
    // OSReport("Starpole: Requesting Frame %d\n", frame_idx);

    int result = 0;
    int enable = OSDisableInterrupts();

    // request data
    int frame_size = Starpole_Imm(STARPOLE_CMD_REQFRAME, frame_idx);
    if (frame_size <= 0)
        goto CLEANUP;

    // receive it
    if (!Starpole_DMA(starpole_buf, frame_size, EXI_READ))
        goto CLEANUP;

    result = 1;

CLEANUP:
    OSRestoreInterrupts(enable);
    return result;
}

void Record_OnFrameStart()
{
    starpole_buf->frame.ply_num = 0;
    return;
}
void Record_OnRiderInput(RiderData *rd)
{
    int record_ply_index = starpole_buf->frame.ply_num;

    // index of this player
    starpole_buf->frame.ply[record_ply_index].idx = rd->ply;

    int buttons_to_copy = (PAD_BUTTON_A | PAD_BUTTON_B | PAD_BUTTON_X | PAD_BUTTON_Y |
                            PAD_TRIGGER_L | PAD_TRIGGER_L | PAD_TRIGGER_Z |
                            PAD_BUTTON_DPAD_UP | PAD_BUTTON_DPAD_DOWN | PAD_BUTTON_DPAD_LEFT | PAD_BUTTON_DPAD_RIGHT);

    // copy buttons
    starpole_buf->frame.ply[record_ply_index].input.held = (rd->input.held & buttons_to_copy);
    starpole_buf->frame.ply[record_ply_index].input.stickX = rd->input.stickX;
    starpole_buf->frame.ply[record_ply_index].input.stickY = rd->input.stickY;
    starpole_buf->frame.ply[record_ply_index].input.substickX = denormalize_signed(rd->input.rstick.X);
    starpole_buf->frame.ply[record_ply_index].input.substickY = denormalize_signed(rd->input.rstick.Y);
    starpole_buf->frame.ply[record_ply_index].input.trigger = denormalize_unsigned(rd->input.trigger);

    starpole_buf->frame.ply_num++;
}
void Record_OnFrameEnd(GOBJ *g)
{
    // log amount of players we have data for
    starpole_buf->frame.frame_idx = frame_idx;
    starpole_buf->frame.rng_seed = (*hsd_rand_seed);

    Replay_SendFrame(frame_idx);

    Text_SetText(frame_text, 0, "Frame: %d", frame_idx);
    frame_idx++;
}

void Playback_OnFrameStart()
{
    // request frame
    if (!Replay_ReqFrame(frame_idx))
    {
        // couldn't get frame, if the game is frozen, its likely at the time up screen so do noth8ing
        if (Gm_GetGameData()->update.pause_kind & ((1 << 0) | (1 << 1) | (1 << 2) | (1 << 3)))
            return;

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
        rd->input.held = (int)starpole_buf->frame.ply[ply].input.held;      // game code will update down for us (8018f178)
        rd->input.stickX = starpole_buf->frame.ply[ply].input.stickX;       // game code will convert to float and update the Vec2 (8018f154)
        rd->input.stickY = starpole_buf->frame.ply[ply].input.stickY;
        rd->input.rstick.X = normalize_signed(starpole_buf->frame.ply[ply].input.substickX);
        rd->input.rstick.Y = normalize_signed(starpole_buf->frame.ply[ply].input.substickY);
        rd->input.trigger = normalize_unsigned(starpole_buf->frame.ply[ply].input.trigger);

        return;
    }

    OSReport("Replay: WARNING no frame data found for ply %d on frame %d (%p)\n", rd->ply, frame_idx, &starpole_buf->frame);
    // assert("0");
}
void Playback_OnFrameEnd(GOBJ *g)
{
    // OSReport("Frame %d:\n", frame_idx);

    // desync detection
    if (starpole_buf->frame.rng_seed != *hsd_rand_seed)
    {
        if (!desync_text)
        {
            OSReport("Replay: ERROR Random seed mismatch on frame %d!\n", frame_idx);
            Replay_CreateDesyncText(frame_idx);
        }
    }

    Text_SetText(frame_text, 0, "Frame: %d", frame_idx);
    frame_idx++;
}

float PlyCam_ClampStick(float val)
{
    const float dead = 0.4f;

    if (val > dead)
        return (val - dead) / (1.0f - dead);
    else if (val < -dead)
        return (val + dead) / (1.0f - dead);
    else
        return 0.0f;
}
void PlyCam_UseRiderInputsForMachineCameraControl(CamData *cam_data, int controller_idx, float *limits)
{
    if (!cam_data->target)
    {
        cam_data->x84_80 = 0;
        return;
    }

    cam_data->x84_80 = 1;

    RiderData *rd = Ply_GetRiderGObj(cam_data->target->ply)->userdata;

    float rstickX = PlyCam_ClampStick(rd->input.rstick.X);
    float rstickY = PlyCam_ClampStick(rd->input.rstick.Y);

    float *tuning = (float *)stc_plycam_lookup->x234;

    /* ---------- ROTATION (YAW) ---------- */

    float desired = -rstickX * limits[0]; // yaw speed scalar
    float cur = cam_data->rotation_amt;
    float max_step = M_1DEGREE * tuning[0x334 / 4];
    float delta = (desired - cur) * tuning[0x330 / 4];

    if (delta > max_step)
        cam_data->rotation_amt = cur + max_step;
    else if (delta < -max_step)
        cam_data->rotation_amt = cur - max_step;
    else
        cam_data->rotation_amt = cur + delta;

    // zoom and pitch

    if (rstickY != 0.0f)
        cam_data->zoom_amt += -rstickY * tuning[0x32c / 4];
    ; // inferred

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
void Replay_OnFrameStart()
{
    // dont run on start pause
    if (Gm_GetGameData()->update.pause_kind & (1 << 1))
        return;

    if (replay_mode == REPLAY_PLAYBACK)
        Playback_OnFrameStart();
    else if (replay_mode == REPLAY_RECORD)
        Record_OnFrameStart();

    return;
}
CODEPATCH_HOOKCREATE(0x80012e74, "", Replay_OnFrameStart, "", 0)

// Injection to read inputs directly from rider data
int Record_RiderInputBackup(RiderData *rd)
{
    if (replay_mode == REPLAY_RECORD)
        Record_OnRiderInput(rd);

    return 0;
}
CODEPATCH_HOOKCREATE(0x8018effc, "mr 3, 31\n\t", Record_RiderInputBackup, "", 0)

// Injection to write inputs directly to rider data
int Playback_RiderInputRestore(RiderData *rd)
{
    if (replay_mode == REPLAY_PLAYBACK)
    {
        Playback_OnRiderInput(rd);
        return 1;
    }

    return 0;
}
CODEPATCH_HOOKCONDITIONALCREATE(0x8018ef34, "mr 3, 31\n\t", Playback_RiderInputRestore, "", 0, 0x8018effc)

// Injection to send the match's data
void Playback_BackupMatch()
{
    if (replay_mode == REPLAY_RECORD)
    {
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
        starpole_buf->match.stage_kind = gp->stage_kind;
        starpole_buf->match.stadium_kind = gp->city.stadium_kind;
        // starpole_buf->match.city_kind = gp->city_kind;
        // starpole_buf->match.time_seconds = gp->time_seconds;
        // starpole_buf->match.tempo = gp->tempo;
        // starpole_buf->match.is_enable_events = gp->is_enable_events;
        for (int i = 0; i < GetElementsIn(starpole_buf->match.stadium.ply_stats); i++)
        {
            starpole_buf->match.stadium.is_bike[i] = gp->city.is_bike[i];
            starpole_buf->match.stadium.machine_kind[i] = gp->city.machine_kind[i];

            for (int j = 0; j < GetElementsIn(starpole_buf->match.stadium.ply_stats[0]); j++)
            {
                starpole_buf->match.stadium.ply_stats[i][j] = (s8)gp->city.ply_stats[i][j];
            }
        }
        memcpy(starpole_buf->match.misc, &gp->city_kind, sizeof(starpole_buf->match.misc));
        memcpy(starpole_buf->match.ply_desc, gp->ply_desc, sizeof(starpole_buf->match.ply_desc));

        Replay_SendMatch();
    }

    return;
}
CODEPATCH_HOOKCREATE(0x800144d4, "mr 3, 31\n\t", Playback_BackupMatch, "", 0)

// Injection to write the replay's match data
int Playback_RestoreMatch()
{
    if (replay_mode == REPLAY_PLAYBACK)
    {
        // copy to game struct
        GameData *gp = Gm_GetGameData();
        *hsd_rand_seed = starpole_buf->match.rng_seed;
        gp->city.stadium_kind = starpole_buf->match.stadium_kind;
        gp->stage_kind = starpole_buf->match.stage_kind;
        // gp->city_kind = starpole_buf->match.city_kind;
        // gp->time_seconds = starpole_buf->match.time_seconds;
        // gp->xaa6_20 = 1;
        // gp->xaa6_10 = 1;
        // gp->tempo = starpole_buf->match.tempo;
        // gp->is_enable_events = starpole_buf->match.is_enable_events;
        for (int i = 0; i < GetElementsIn(gp->city.ply_stats); i++)
        {
            gp->city.is_bike[i] = starpole_buf->match.stadium.is_bike[i];
            gp->city.machine_kind[i] = starpole_buf->match.stadium.machine_kind[i];

            for (int j = 0; j < GetElementsIn(gp->city.ply_stats[0]); j++)
            {
                gp->city.ply_stats[i][j] = (float)starpole_buf->match.stadium.ply_stats[i][j];
            }
        }
        memcpy(&gp->city_kind, starpole_buf->match.misc, sizeof(starpole_buf->match.misc));
        memcpy(gp->ply_desc, starpole_buf->match.ply_desc, sizeof(starpole_buf->match.ply_desc));

        return 1;
    }

    return 0;
}
CODEPATCH_HOOKCONDITIONALCREATE(0x800144cc, "mr 3, 31\n\t", Playback_RestoreMatch, "", 0, 0x800144d4)

// Injection to make the kirby on foot camera use the rider input data (we restore this)
float PlyCam_UseRiderInputsForOnFootCameraControl(CamData *cam_data)
{
    if (!cam_data->target)
        return 0;

    RiderData *rd = Ply_GetRiderGObj(cam_data->target->ply)->userdata;

    float x = rd->input.rstick.X;
    const float dead = 0.4f;

    if (x > dead)
        x = (x - dead) / (1.0f - dead);
    else if (x < -dead)
        x = (x + dead) / (1.0f - dead);
    else
        x = 0.0f;

    return x;
}
CODEPATCH_HOOKCREATE(0x800cb4c8, "mr 3, 29\n\t", PlyCam_UseRiderInputsForOnFootCameraControl, "fmr 2, 1\n\t", 0)

// Injection to avoid directly referencing the current cobj's position when dismounting a machine
// (the active cobj should not impact gameplay, this allows us to freecam)
void Dismount_GetCameraPosition(CamData *cd)
{
    memcpy(&cd->xe8.interest, &cd->interest_pos, sizeof(cd->xe8.interest)); // interest pos
    memcpy(&cd->xe8.eye, &cd->eye_pos, sizeof(cd->xe8.eye));                // eye pos
}
CODEPATCH_HOOKCREATE(0x800b7840, "mr 3, 30\n\t", Dismount_GetCameraPosition, "", 0)

// Mod Callbacks
void Replay_OnBoot()
{
    CODEPATCH_HOOKAPPLY(0x800b7840);
    CODEPATCH_HOOKAPPLY(0x8018ef34);
    CODEPATCH_HOOKAPPLY(0x800cb4c8);
    CODEPATCH_HOOKAPPLY(0x80012e74);
    CODEPATCH_HOOKAPPLY(0x8018effc);

    CODEPATCH_HOOKAPPLY(0x800144cc);
    CODEPATCH_HOOKAPPLY(0x800144d4);

    CODEPATCH_REPLACEFUNC(0x800b67cc, PlyCam_UseRiderInputsForMachineCameraControl);
}
void Replay_OnSceneChange()
{
    replay_mode = REPLAY_NONE;
}
void Replay_On3DLoadStart()
{
    // ensure starpole is active
    if (!Starpole_IsPresent())
        return;
    
    // decide replay mode
    if (Scene_GetCurrentMajor() == MJRKIND_CITY || Scene_GetCurrentMajor() == MJRKIND_AIR)
        replay_mode = REPLAY_RECORD;
    else if (Playback_IsMajor())
    {
        replay_mode = REPLAY_PLAYBACK;
        Gm_GetGameData()->major_cur = MJRKIND_CITY; // need to spoof this??
    }
    else
    {
        replay_mode = REPLAY_NONE;
        return;
    }

    // send/receive initial match data
    int result;
    if (replay_mode == REPLAY_PLAYBACK)
        result = Replay_ReqMatch();
    else
        result = Replay_SendMatch();

    if (!result)
    {
        replay_mode = REPLAY_NONE;
        return;
    }

    // create a gobj to transmit per frame match data
    GOBJ *g = GOBJ_EZCreator(0, GAMEPLINK_1, 0,
                             0, 0,
                             0, 0,
                             0, 0,
                             0, 0, 0);

    frame_idx = 0;

    if (replay_mode == REPLAY_RECORD)
    {
        GObj_AddProc(g, Record_OnFrameEnd, stc_gobj_init_data->proc_pri_max - 1);
    }
    else if (replay_mode == REPLAY_PLAYBACK)
    {
        GObj_AddProc(g, Playback_OnFrameEnd, stc_gobj_init_data->proc_pri_max - 1);

        // use live view camera
        GameData *gd = Gm_GetGameData();
        for (int i = 0; i < GetElementsIn(gd->ply_view_desc); i++)
            gd->ply_view_desc[i].flag = PLYCAM_OFF;

        gd->ply_view_desc[0].flag = PLYCAM_ON;
    }

    // debug display
    if (1)
    {
        for (int i = 0; i < GetElementsIn(debug_text); i++)
            debug_text[i] = 0;

        GObj_AddProc(g, Replay_Debug, 20);
    }

    desync_text = 0;
    Replay_CreateFrameText();
}
void Replay_On3DExit()
{
    if (replay_mode == REPLAY_RECORD)
        Replay_SendEnd();
}