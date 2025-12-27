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

#include "starpole.h"
#include "code_patch/code_patch.h"
#include "text_joint/text_joint.h"

extern StarpoleBuffer *starpole_buf;
int frame_idx;

int Replay_SendMatch()
{
    // notify EXI of incoming data
    if (Starpole_Imm(STARPOLE_CMD_MATCH, 0) <= 0)
        return 0;

    // initialize outgoing buffer
    GameData *gp = Gm_GetGameData();
    starpole_buf->match.rng_seed = *hsd_rand_seed;
    starpole_buf->match.gr_kind = gp->stage_kind;
    memcpy(starpole_buf->match.ply_desc, gp->ply_desc, sizeof(starpole_buf->match.ply_desc));

    // send it
    if (!Starpole_DMA(starpole_buf, sizeof(starpole_buf->match), EXI_WRITE))
        return 0;

    return 1;
}
int Replay_SendFrame()
{
    // notify EXI of incoming data
    if (Starpole_Imm(STARPOLE_CMD_FRAME, 0) == -1)
        return 0;

    // send it
    if (!Starpole_DMA(starpole_buf, sizeof(starpole_buf->frame), EXI_WRITE))
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
    gp->stage_kind = starpole_buf->match.gr_kind;
    memcpy(gp->ply_desc, starpole_buf->match.ply_desc, sizeof(starpole_buf->match.ply_desc));

    return 1;
}
int Replay_ReqFrame(int frame_idx)
{
    // request data
    if (Starpole_Imm(STARPOLE_CMD_REQFRAME, frame_idx) == -1)
        return 0;

    // receive it
    if (!Starpole_DMA(starpole_buf, sizeof(starpole_buf->frame), EXI_READ))
        return 0;

    return 1;
}

void Replay_OnFrameStart(GOBJ *g)
{
    starpole_buf->frame.rng_seed = *hsd_rand_seed;
    return;
}
void Replay_OnFrameInputs(GOBJ *g)
{
    // log player inputs
    int ply = 0;
    for (int i = 0; i < 4; i++)
    {
        if (Ply_GetPKind(i) != PKIND_NONE)
        {
            GOBJ *r = Ply_GetRiderGObj(i);
            RiderData *rd = r->userdata;

            // index of this player
            starpole_buf->frame.ply[ply].idx = ply;

            // copy buttons
            starpole_buf->frame.ply[ply].input.buttons = rd->input.down;
            starpole_buf->frame.ply[ply].input.lstick = rd->input.lstick;
            starpole_buf->frame.ply[ply].input.rstick = rd->input.rstick;

            ply++;
        }
    }

    // log amount of players we have data for
    starpole_buf->frame.ply_num = ply;
    starpole_buf->frame.frame_idx = frame_idx;
}
void Replay_OnFrameEnd(GOBJ *g)
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

            // copy state
            starpole_buf->frame.ply[ply].rd_state = rd->state_idx;
            starpole_buf->frame.ply[ply].machine_kind = machine_kind;
            starpole_buf->frame.ply[ply].pos = rd->pos;

            ply++;
        }
    }

    // ship it off
    if (Gm_GetIntroState() == GMINTRO_END)
    {
        // Starpole_SendFrame();
        Replay_ReqFrame(frame_idx);
        frame_idx++;
    }
    
}

void Replay_On3DLoadStart()
{
    if (!Starpole_IsPresent())
        return;

    // send off initial match data
    if (!Replay_SendMatch())
        return;
        
    // if (!Replay_ReqMatch())
    //     return;

    // create a gobj to transmit per frame match data
    GOBJ *g = GOBJ_EZCreator(0, GAMEPLINK_1, 0, 
                    0, 0,
                    0, 0, 
                    0, 0,
                    0, 0, 0);

    frame_idx = 0;
    GObj_AddProc(g, Replay_OnFrameStart, RDPRI_0);
    GObj_AddProc(g, Replay_OnFrameInputs, RDPRI_INPUT + 1);
    GObj_AddProc(g, Replay_OnFrameEnd, RDPRI_15 + 1);
}
void Replay_On3DExit()
{
    Replay_SendEnd();
}