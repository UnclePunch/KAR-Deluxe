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
#include "hash.h"
#include "code_patch/code_patch.h"
#include "text_joint/text_joint.h"

void *(*OSGetCaller)(int level) = (void *)0x80428618;

HSD_Pad *sys_pads = (HSD_Pad *)0x8058b0e4;

void (*HSD_InsertIntoPadQueue)(PADStatus *status, int unk) = (void *)0x80412480;
void (*HSD_PadConsume)() = (void *)0x80062978;

extern StarpoleDataDolphin *dolphin_data;

DebugPadData g_debug_pad = {.head = 0};

int g_state_hash = 0;

AudioLog g_audio_log;
RollbackLog g_rollback;

PADStatus g_local_status[4] = {0};
PADStatus g_remote_status[MAX_SIM_FRAMES + 1][4] = {0};
PreserveMemRegion g_preserve_regions[] = {
    {(void *)0, 0, false},                                     // stay
    {(void *)0, 0, false},                                     // AllM
    {(void *)0x00000000, 0x96000, false},                      // XFB buffer 1 80589a48
    {(void *)0x00000000, 0x96000, false},                      // XFB buffer 2 80589a4c
    {(void *)0x00000000, 0x80000, false},                      // gx init alloc in arena lo, performed at 8040fc3c
    {(void *)0, 0, true},                                      // audio heap
    {(void *)0, 0, true},                                      // audio track status
    
    {(void *)0x80003100, 0x2500, false},                       // dol text section 1
    {(void *)0x80005800, 0x483C40, false},                     // dol text section 2
    {(void *)(0x805f6390 - (32 * 1024)), 32 * 1024, false},    // stack, address derived from 80005410. im inferring 32kb stack

    {&g_audio_log, sizeof(g_audio_log), true},                 // audio log
    {&g_rollback, sizeof(g_rollback), false},                  // rollback specific data
    {&g_debug_pad, sizeof(g_debug_pad), false},                // debug pad data
    {&g_remote_status, sizeof(g_remote_status), false},        // pad data to use for game frames

    {(void *)0x80550f68, 0x1008, false},                       // file preload table
    {(void *)0x80508bc8, 0x4 * 3, true},                       // BGM PID's. needed to stop a song from playing
    
    {(void *)(0x805dd0e0 + 0xF20), 0xF68 - 0xF20, true},      // ARQ and hsd audio sbss
    {(void *)(0x805dd0e0 + 0xAC), 0x8, true},                 // 64 bitfield that is raised when the corresponding sg has its volume changed, 0x8044c450
    {(void *)0x80599c60, 0x8059a818 - 0x80599c60, true},      // more audio stuff. sg indexed volume data in here @ 8059a178 and 8059a160?
    {(void *)(0x805dd0e0 + 0x1358), 0x1470 - 0x1358, true},   // hsd audio sbss
    {(void *)0x8056ccb4, 0x24, true},                         // DVD Waiting Queue
    {(void *)0x8056cb40, 0xE0, true},                         // DVD Interrupt stuff @ 803c40b4. includes alarm
    {(void *)0x8056cc20, 0x94, true},                         // DVD state stuff @ 803c67f0. another alarm at 0x70 of this?

    {(void *)(0x805dd0e0 + 0xC60), 0xCF4 - 0xC60, true},      // disc read variables
    {(void *)(0x805dd0e0 + 0xDC8), 0xDD0 - 0xDC8, true},      // OSAlarm variables
    {(void *)(0x805dd0e0 + 0x4C8), 0x4, true},                // file async load flag

    {(void *)0x8056e9e8, 0x80587A60 - 0x8056e9e8, true},      // all the AX data i know of, AXStack head -> end of __AXVPB
    {(void *)(0x805dd0e0 + 0xF30), 0x1054 - 0xF30, true},     // AX region sbss

    {(void *)0x8058e298, 64 * 0x4, true},                     // array of VPB pointers? indexed by FGMInstance index
    {(void *)0x8058E398, 0x8F8, true},                        // unknown in between chunks, part of this is the fgm_kind struct, referenced @ 80442a24
    {(void *)0x8058ec90, 0x90, true},                         // hps stream unk struct @ 804464bc
    {(void *)0x8058ed20, 2 * 0x4000, true},                   // hps double buffer?
    {(void *)0x80596d20, 0x40, true},                         // hps streaming @ 80446a74
    {(void *)0x80596d60, 0x50, true},                         // hps streaming stuff
    {(void *)0x80596da0, 160 + (512*3), true},                // FGM region, multiple offsets of this loaded around 80447ee4. also includes some HPS streaming stuff
    {(void *)0x80597440, 0x220, true},                        // unknown in between chunks

    {(void *)0x80597660, 64 * 0x98, true},                    // AXLive voice array. (8044ccf0)
    {(void *)0x80597F20, 64 * 152, true},                     // static audio lookup 0X8c0 (8044ccf0)

    {(void *)0x8059a880, 0x618, false},                        // memcard thread data? referenced by the function 8045b848 in the thread func
    {(void *)0x805b4698, 0x35C, false},                        // memcard thread data
};

void Netplay_GetPreserveRegions(PreserveMemRegion *regions_out)
{
    memcpy(regions_out, g_preserve_regions, sizeof(g_preserve_regions));

    // Heaps
    regions_out[0].addr = stc_preload_heaps_lookup->heap_arr[PRELOADHEAPKIND_STAY].addr_start;
    regions_out[0].size = stc_preload_heaps_lookup->heap_arr[PRELOADHEAPKIND_STAY].size;
    regions_out[1].addr = stc_preload_heaps_lookup->heap_arr[PRELOADHEAPKIND_ALLM].addr_start;
    regions_out[1].size = stc_preload_heaps_lookup->heap_arr[PRELOADHEAPKIND_ALLM].size;
    // XFB Buffers
    regions_out[2].addr = *(void **)0x80589a48;
    regions_out[3].addr = *(void **)0x80589a4c;
    // GX FIFO
    regions_out[4].addr = *(void **)0x8056d240;
    // audio heap
    regions_out[5].addr = *(void **)0x804bdb2c;
    regions_out[5].size = *(u32 *)0x804bdb30;
    // audio track status
    regions_out[6].addr = (*stc_audio_track_unk);
    regions_out[6].size = sizeof(u16) * (*stc_audio_track_num);
    

    return;
}

int Netplay_StartRollback()
{
    int result = 0;
    int level = OSDisableInterrupts();

    g_rollback.resim_idx = 0;

    // create buffer of memory regions to preserve
    char buffer[sizeof(g_preserve_regions)] __attribute__((aligned(32)));
    Netplay_GetPreserveRegions((PreserveMemRegion *)&buffer);

    // notify of incoming data
    if (Starpole_Imm(STARPOLE_CMD_NETSTART, GetElementsIn(g_preserve_regions)) <= 0)
    {
        NetLog("Starpole: unable to start rollback.\n");
        goto CLEANUP;
    }

    // send it
    if (!Starpole_DMA((StarpoleBuffer *)buffer, sizeof(buffer), EXI_WRITE))
        goto CLEANUP;

    result = 1;

CLEANUP:
    OSRestoreInterrupts(level);
    return result;
}
int Netplay_EndRollback()
{
    int result = 0;
    int level = OSDisableInterrupts();

    int frame_idx = Gm_GetGameData()->update.engine_frames;

    // notify of incoming data
    if (Starpole_Imm(STARPOLE_CMD_NETEND, frame_idx) <= 0)
    {
        NetLog("Starpole: unable to end rollback.\n");
        goto CLEANUP;
    }

    result = 1;

CLEANUP:
    OSRestoreInterrupts(level);
    return result;
}
u32 Netplay_RequestSave(u32 frame_idx)
{
    int result = 0;
    int enable = OSDisableInterrupts();

    NetLog("requesting save/load on frame %d\n", frame_idx);

    u32 confirm_frame = Starpole_Imm(STARPOLE_CMD_NETSAVE, frame_idx);
    result = 1;

CLEANUP:
    OSRestoreInterrupts(enable);
    return confirm_frame;
}
int Netplay_SendInputs(PADStatus *status)
{
    int result = 0;
    int enable = OSDisableInterrupts();
    u32 this_frame_idx = Gm_GetGameData()->update.engine_frames;

    // copy pad data to aligned buffer
    StarpoleDataInputs buffer __attribute__((aligned(32)));
    memcpy(buffer.status, status, sizeof(buffer.status));

    // notify of incoming data
    if (Starpole_Imm(STARPOLE_CMD_NETPADSEND, this_frame_idx) <= 0)
    {
        NetLog("Starpole: unable to send pad data.\n");
        goto CLEANUP;
    }

    // send it
    if (!Starpole_DMA((StarpoleBuffer *)&buffer, sizeof(buffer), EXI_WRITE))
        goto CLEANUP;

    result = 1;

CLEANUP:
    OSRestoreInterrupts(enable);
    return result;
}
int Netplay_ReceiveInputs()
{
    int result;
    u32 this_frame_idx = Gm_GetGameData()->update.engine_frames;
    int enable = OSDisableInterrupts();

    // request pad data and receive the number of frames will receive pad data for
    result = Starpole_Imm(STARPOLE_CMD_NETPADRECV, this_frame_idx);

    int sim_num = result & 0x7FFFFFF;

    if (sim_num > MAX_SIM_FRAMES + 1)
    {
        OSReport("sim_num (%d) over MAX_SIM_FRAMES on game frame %d.\n", sim_num, this_frame_idx);
        assert("starpole");
    }

    if (NETPLAY_DEBUG)
    {
        // store all inputs to buffer
        memcpy(g_debug_pad.history[g_debug_pad.head], g_local_status, sizeof(g_local_status));
        g_debug_pad.head = (g_debug_pad.head + 1) % 10;
    }
    else
    {
        if (sim_num <= 0)
        {
            // NetLog("Starpole: inputs not ready.\n");
            goto CLEANUP;
        }

        // alloc stack buffer to receive to
        PADStatus buffer[MAX_SIM_FRAMES][4] __attribute__((aligned(32)));
        
        memset(buffer, 0, sizeof(buffer));

        // receive inputs
        if (!Starpole_DMA((StarpoleBuffer *)buffer, sizeof(buffer), EXI_READ))
            goto CLEANUP;

        // copy to remote inputs
        memcpy(g_remote_status, buffer, sizeof(buffer));
    }

    // // duplicate input for now
    // for (int i = 1; i < GetElementsIn(g_remote_status[0]) - 1; i++)
    //     memcpy(&g_remote_status[i], &g_remote_status[0], (sizeof(g_remote_status[0])));


CLEANUP:
    OSRestoreInterrupts(enable);
    return result;
}
u32 Netplay_ReceiveConfirmFrame()
{
    u32 confirm_frame;
    u32 this_frame_idx = Gm_GetGameData()->update.engine_frames;
    int enable = OSDisableInterrupts();

    confirm_frame = Starpole_Imm(STARPOLE_CMD_NETGETCONFIRM, 0);

CLEANUP:
    OSRestoreInterrupts(enable);
    return confirm_frame;
}
int Netplay_SendGameState(u32 hash)
{
    int result = 0;
    int enable = OSDisableInterrupts();
    u32 this_frame_idx = Gm_GetGameData()->update.engine_frames;

    // copy pad data to aligned buffer
    StarpoleDataGameState buffer __attribute__((aligned(32)));
    buffer.frame = this_frame_idx;
    buffer.hash = hash;

    // notify of incoming data
    if (Starpole_Imm(STARPOLE_CMD_NETGAMESTATE, 0) <= 0)
    {
        NetLog("Starpole: unable to send game state.\n");
        goto CLEANUP;
    }

    // send it
    if (!Starpole_DMA((StarpoleBuffer *)&buffer, sizeof(buffer), EXI_WRITE))
        goto CLEANUP;

    result = 1;

CLEANUP:
    OSRestoreInterrupts(enable);
    return result;
}

int Netplay_SkipFrameCheck()
{
    return Gm_GetGameData()->update.is_req_exit_minor;
}
CODEPATCH_HOOKCONDITIONALCREATE(0x80006828, "", Netplay_SkipFrameCheck, "", 0, 0x80006a80)

int Netplay_WaitForClients()
{
    PADRead(g_local_status);                      // poll inputs this frame
    Netplay_SendInputs(g_local_status);           // send inputs to dolphin

    // update game sim only when we have all inputs for this frame
    int result = Netplay_ReceiveInputs();
    g_rollback.is_rollback = (result & 0x80000000) >> 31;
    g_rollback.is_render = (result & 0x40000000) >> 30;
    g_rollback.sim_frames = (result & 0x7FFFFFFF);
    g_rollback.confirm_frame = Netplay_ReceiveConfirmFrame();

    if (g_rollback.sim_frames > 0)
    {
        NetLog("\nFRAME START\n");
        NetLog("confirm_frame: %d\n", g_rollback.confirm_frame);
    }

    // if we simulate more than 1 frame we can assume we rolled back, so increment resim index
    if (g_rollback.is_rollback)
    {
        g_rollback.resim_idx++;
        
        // reset is_replayed bool on sounds
        for (int i = 0; i < GetElementsIn(g_audio_log.sfx_start); i++)
            g_audio_log.sfx_start[i].is_replayed = (u32)0;
            
        NetLog("resim detected!! resim_idx: %d\n", g_rollback.resim_idx);
    }

    // NetLog("\nwill sim %d frames\n", g_rollback.sim_frames);

    return g_rollback.sim_frames;
}
CODEPATCH_HOOKCREATE(0x80006bd4, "", Netplay_WaitForClients, "", 0)

void Netplay_OnFrameStart(int loop_num)
{
    u32 frame = Gm_GetGameData()->update.engine_frames;

    // request save/load
    Netplay_RequestSave(frame);

    g_rollback.this_sim_idx = loop_num;
    g_rollback.is_resim_frame = (g_rollback.is_rollback && g_rollback.this_sim_idx < (g_rollback.sim_frames - 1));

    NetLog("now simulating frame %d!\n", Gm_GetGameData()->update.engine_frames);

    // insert pad into queue
    if (NETPLAY_DEBUG)
    {
        int frames_ago = (g_rollback.sim_frames - 1) - loop_num;
        int idx = (g_debug_pad.head - 1 - frames_ago + 10) % 10;
        HSD_InsertIntoPadQueue(g_debug_pad.history[idx], 0);
    }
    else
    {
        // use this frames inputs for non-netplay
        PADStatus *status = (Dolphin_IsNetplay()) ? g_remote_status[loop_num] : g_local_status;

        // insert the pad and consume it
        HSD_InsertIntoPadQueue(status, 0);
    }
}
CODEPATCH_HOOKCREATE(0x8000682c, "mr 3, 29\t\n", Netplay_OnFrameStart, "", 0)

void Netplay_CheckVIWaitForRetrace()
{
    // the idea here is to not sleep the main thread every frame waiting for
    // VI interrupt when we not rendering anything. avoiding this allows us to
    // process game frames faster. we take care to render a black quad to screen 
    // for the first frame so there isnt a lingering image while we completely 
    // ignore VI

    // if (g_rollback.is_render)
        VIWaitForRetrace();
}
CODEPATCH_HOOKCREATE(0x80006b94, "", Netplay_CheckVIWaitForRetrace, "b 0x8\n\t", 0)

int Netplay_CheckRender()
{
    void (*BGM_AdjustVolume)(int volume) = (void *)0x8005fb64;
    void (*FGM_AdjustVolume)(int volume) = (void *)0x8005fae4;
    int *AX_is_updating = (int *)0x805ddfd4;
    void (*AXOutAiCallback)() = (void *)0x803ed268;

    int result;

    if (!g_rollback.is_render)
    {
        // draw a black quad for the first frame we skip rendering
        if (g_rollback.is_render_prev)
        {
            // mute audio
            BGM_AdjustVolume(0);
            FGM_AdjustVolume(0);

            // raise AI interrupt in progress flag so the interrupt never runs
            // int level = OSDisableInterrupts();
            // *AX_is_updating = 1;
            // OSRestoreInterrupts(level);

            GOBJ *developtextcam_gobj = (*stc_gobj_lookup)[61];

            if (developtextcam_gobj && 
                developtextcam_gobj->obj_kind == HSD_OBJKIND_COBJ)
            {
                HSD_StartRender(0);

                // draw black quad
                CObj_SetEraseColor(0, 0, 0, 0);
                CObj_EraseScreen(developtextcam_gobj->hsd_object, 1, 1, 1);

                HSD_VICopyXFBASync(0);
            }
        }

        // update AX 3 times per game tick
        // (normally runs approx every 5ms, so 3 times is about 1 frames worth)
        // int level = OSDisableInterrupts();
        // for (int i = 0; i < 3; i++)
        //     AXOutAiCallback();
        // OSRestoreInterrupts(level);


        result = 1;
    }
    else
    {
        if (!g_rollback.is_render_prev)
        {
            // lower AI interrupt in progress flag so the interrupt runs
            // int level = OSDisableInterrupts();
            // *AX_is_updating = 0;
            // OSRestoreInterrupts(level);

            // unmute audio
            BGM_AdjustVolume(178);
            FGM_AdjustVolume(178);
        }
       
        result = 0;
    }
    
    // update previous render state
    g_rollback.is_render_prev = g_rollback.is_render;

    return result;

}
CODEPATCH_HOOKCONDITIONALCREATE(0x80006a8c, "", Netplay_CheckRender, "", 0, 0x80006b28)

void Netsync_AdjustGameLoop()
{
    int *is_alarm_active = (int *)0x80550ca8;
    OSAlarm *alarm_ptr = (OSAlarm *)0x80550d28;

    g_rollback.is_render_prev = 1;
    g_rollback.is_render = 1;

    // cancel any active alarms
    if (*is_alarm_active)
        OSCancelAlarm(alarm_ptr);
    CODEPATCH_REPLACEINSTRUCTION(0x80062660, 0x4e800020);       // disable pad alarm creation

    CODEPATCH_HOOKAPPLY(0x80006b94);                            // replace pad alarm jam with viwaitforretrace
    // CODEPATCH_REPLACECALL(0x80006b94, VIWaitForRetrace);        // replace pad alarm jam with viwaitforretrace

    // netpause at the top of each frame
    // CODEPATCH_REPLACEINSTRUCTION(0x8000682c, 0x60000000);    // remove pad consume
    CODEPATCH_REPLACEINSTRUCTION(0x80006bd4, 0x60000000);       // remove GetPadQueue Call
    CODEPATCH_REPLACEINSTRUCTION(0x80006bdc, 0x60000000);       // render on 0 ticks
    CODEPATCH_HOOKAPPLY(0x80006828);                            // skip frame if request to leave already
    CODEPATCH_HOOKAPPLY(0x80006bd4);                            // wait for inputs
    CODEPATCH_HOOKAPPLY(0x8000682c);                            // consume pad
    CODEPATCH_HOOKAPPLY(0x80006a8c);                            // gate render
}

// here we will attempt to optimize catchup frames by not processing
// things that dont impact game state
int Netsync_IsFinalSimFrame()
{
    return (((g_rollback.sim_frames - 1) - g_rollback.this_sim_idx) == 0);
}
// audio position updates
CODEPATCH_HOOKCONDITIONALCREATE(0x800614bc, "", Netsync_IsFinalSimFrame, "cmpwi 3, 1\n\t"
                                                                        "lis 3, 0x8054\n\t" 
                                                                        "subi 3, 3, 32632\n\t"
                                                                        "b 0x8\n\t", 0, 0x80061540)
// rider and machine shadow position updates
CODEPATCH_HOOKCONDITIONALCREATE(0x8018fd6c, "", Netsync_IsFinalSimFrame, "xori 3, 3, 1\n\t", 0, 0x8018fd74)
CODEPATCH_HOOKCONDITIONALCREATE(0x801c6b20, "", Netsync_IsFinalSimFrame, "xori 3, 3, 1\n\t", 0, 0x801c6b28)

// SFX
void Audio_Debug()
{
    // get riders machine's emitter
    GOBJ *r = Ply_GetRiderGObj(0);
    if (!r) 
        return;

    RiderData *rd = r->userdata;
    if (!rd->machine_gobj)
        return;
    
    MachineData *md = rd->machine_gobj->userdata;
    AudioEmitter emitter_idx = md->audio.emitter;

    AudioEmitterData *emitter_data = &audio_3d_data->emitters[emitter_idx];
    int sg = emitter_data->sg;
    int fgm_num = Audio_GetFGMNumUsingSoundGenerator(sg);

    if (emitter_data->state > 1 && sg != -1)
    {
        for (int i = 0; i < GetElementsIn(audio_3d_data->emitters); i++)
        {
            if (i != emitter_idx)
            {
                static char *sg_names[] = {"ig", "sg"};
                int sg_kind = -1;

                if (audio_3d_data->emitters[i].sg == sg)
                    sg_kind = 0;
                else if (audio_3d_data->emitters[i].ig == sg)
                    sg_kind = 1;

                if (sg_kind != -1)
                {
                    NetLog("emitter %d (%p) has a matching %s\n", 
                        i, 
                        &audio_3d_data->emitters[i],
                        sg_names[sg_kind]);
                }
            }
        }

        NetLog("emitter %d (%p) has sg %d with %d active sounds\n", 
            emitter_idx, 
            emitter_data,
            sg, 
            fgm_num);
        NetLog("volume %d, pitch %d, pan %d\n", 
            emitter_data->volume, 
            emitter_data->pitch,
            emitter_data->pan); 

        if (fgm_num > 0)
        {
            FGMDebugLog *fgm_log = HSD_MemAlloc(sizeof(FGMDebugLog) * fgm_num);
            int fgm_idx = 0;

            // populate log
            int level = OSDisableInterrupts();
            int this_fgm_idx = 0;
            for (FGMInstanceData *fgm = (*stc_fgm_data_start); fgm; fgm = fgm->next)
            {
                if (fgm->sg == sg)
                {
                    if (this_fgm_idx++ == fgm_idx)
                    {
                        fgm_log[fgm_idx].instance = fgm->instance;
                        fgm_log[fgm_idx].pid = fgm->pid;
                        fgm_log[fgm_idx].sfx_id = fgm->sfx_id;
                        fgm_log[fgm_idx].priority = fgm->priority;
                        fgm_log[fgm_idx].audio_track = fgm->audio_track;

                        // get vpb
                        VPB *vpb = &ax_live->voice_data[fgm->pid & AXDRIVER_PIDMASK];
                        AXVPB *axvpb = vpb->axvpb[0];
                        fgm_log[fgm_idx].vpb.addr = vpb;
                        fgm_log[fgm_idx].vpb.current_vol = vpb->user_vol[1].current;
                        fgm_log[fgm_idx].vpb.target_vol = vpb->user_vol[1].target;
                        
                        if (axvpb)
                        {
                            fgm_log[fgm_idx].axvpb.addr = axvpb;
                            fgm_log[fgm_idx].axvpb.state = axvpb->pb.state;
                            fgm_log[fgm_idx].axvpb.vol = axvpb->pb.ve.currentVolume;
                            fgm_log[fgm_idx].axvpb.vol_l = axvpb->pb.mix.vL;
                            fgm_log[fgm_idx].axvpb.vol_r = axvpb->pb.mix.vR;
                            fgm_log[fgm_idx].axvpb.pitch = axvpb->pb.src.ratioHi;
                            fgm_log[fgm_idx].axvpb.currentAddress = *(void **)&axvpb->pb.addr.currentAddressHi;
                        }

                        if (++fgm_idx == fgm_num)
                            break;
                    }
                }
            }
                
            OSRestoreInterrupts(level);

            // output to log
            for (int i = 0; i < fgm_num; i++)
            {
                NetLog("sound %d/%d:\n", i + 1, fgm_num);
                NetLog(" fgm:\n");
                NetLog("  sfx_id %d:%d\n  instance %08X\n  pid %x\n  priority %d\n  audio_track %08X\n  vpb %p\n", 
                    (fgm_log[i].sfx_id & 0xFFFF0000) >> 16, fgm_log[i].sfx_id & 0xFFFF,
                    fgm_log[i].instance,
                    fgm_log[i].pid,
                    fgm_log[i].priority,
                    fgm_log[i].audio_track,
                    fgm_log[i].vpb
                    ); 

                NetLog(" vpb %p:\n",
                    fgm_log[i].vpb.addr);
                NetLog("  current_vol: %.2f\n  target_vol: %.2f\n",
                    fgm_log[i].vpb.current_vol,
                    fgm_log[i].vpb.target_vol);

                NetLog(" axvpb %p:\n",
                    fgm_log[i].axvpb.addr);
                if (fgm_log[i].axvpb.addr)
                {
                    NetLog("  state: %d\n  vol: %d\n  vol_l: %d\n  vol_r: %d\n  pitch: %d\n  currAddress: %p\n",
                        fgm_log[i].axvpb.state,
                        fgm_log[i].axvpb.vol,
                        fgm_log[i].axvpb.vol_l,
                        fgm_log[i].axvpb.vol_r,
                        fgm_log[i].axvpb.pitch,
                        fgm_log[i].axvpb.currentAddress);
                }
                

            }

            HSD_Free(fgm_log);
        }
    }
    
}
void Audio_InitLog()
{
    // raise audio log flag after initializing the 3D scene.
    // our first savestate occurs soon after this executes.
    g_audio_log.enable = 1;

    memset(g_audio_log.sfx_start, -1, sizeof(g_audio_log.sfx_start));
    memset(g_audio_log.sfx_stop, -1, sizeof(g_audio_log.sfx_stop));
    memset(g_audio_log.bgm, -1, sizeof(g_audio_log.bgm));

    // // audio debug gobj
    // GOBJ_EZCreator(0, 0, 0,
    //                 0, 0, 
    //                 0, 0, 
    //                 Audio_Debug, 23,
    //                 0, 0, 0);
}
void Audio_Cleanup()
{
    if (!(Scene_GetCurrentMinor() == MNRKIND_3D && g_audio_log.enable))
        return;

    // on the final sim
    if (g_rollback.this_sim_idx == (g_rollback.sim_frames - 1))
    {
        // if this is the end of a resim, cleanup AX state if sounds that shouldn't be playing are
        if (g_rollback.is_rollback)
            Audio_ValidateAX((Gm_GetGameData()->update.engine_frames + 1) - g_rollback.sim_frames); // need to +1 because engine frames increments after this hook

        // expire audio logs for frames we wont be rolling back to anymore
        Audio_ExpireLogs();
    }
}

void Audio_ValidateAX(u32 rollback_start_frame)
{
    // this function only gets called after a resim
    NetLog("SFX: validating sfx played between frame %d and %d\n", rollback_start_frame, Gm_GetGameData()->update.engine_frames);

    for (int i = 0; i < GetElementsIn(g_audio_log.sfx_start); i++)
    {
        // stop sounds that weren't replayed 
        // we can probably assume they shouldn't be playing anymore
        if (g_audio_log.sfx_start[i].frame != -1)
        {
            NetLog("SFX: validating sfx %08X with fgm instance %08X from frame %d from resim_idx %d with is_replayed %d\n", 
                    g_audio_log.sfx_start[i].sfx_id,
                    g_audio_log.sfx_start[i].fgm_instance,
                    g_audio_log.sfx_start[i].frame,
                    g_audio_log.sfx_start[i].resim_idx,
                    g_audio_log.sfx_start[i].is_replayed);

            if (g_audio_log.sfx_start[i].frame >= rollback_start_frame &&       // sound exists in a prediction branch
                g_audio_log.sfx_start[i].resim_idx != g_rollback.resim_idx &&   // sound was not just played in this prediction branch
                !g_audio_log.sfx_start[i].is_replayed &&                        // sound did not replay on this resim
                !Audio_CheckStopLog(g_audio_log.sfx_start[i].fgm_instance))
            {
                NetLog("SFX: stopping sfx %08X with fgm instance %08X from frame %d due to not being replayed on resim\n", 
                        g_audio_log.sfx_start[i].sfx_id,
                        g_audio_log.sfx_start[i].fgm_instance,
                        g_audio_log.sfx_start[i].frame);

                FGM_Stop(g_audio_log.sfx_start[i].fgm_instance);

                g_audio_log.sfx_start[i].frame = (u32)-1;
            }
        }
    }
}
void Audio_ExpireLogs()
{
    // audio log enabled + wait for confirm frame to be >= 0
    if (!(g_audio_log.enable && g_rollback.confirm_frame != (u32)-1)) 
        return;
    
    u32 this_frame = Gm_GetGameData()->update.engine_frames;
    NetLog("SFX: checking to expire logs on engine frame %d with confirm frame %d\n", this_frame, g_rollback.confirm_frame); 

    // remove start events
    for (int i = 0; i < GetElementsIn(g_audio_log.sfx_start); i++)
    {
        if (g_rollback.confirm_frame >= g_audio_log.sfx_start[i].frame)
        {
            NetLog("SFX: expiring sfx PLAY %08X for sg (%d) with instance %08X on frame %d\n", 
                g_audio_log.sfx_start[i].sfx_id, 
                g_audio_log.sfx_start[i].sg,
                g_audio_log.sfx_start[i].fgm_instance,
                g_audio_log.sfx_start[i].frame);
                
            g_audio_log.sfx_start[i].frame = (u32)-1;
        }
    }

    // remove stop events
    for (int i = 0; i < GetElementsIn(g_audio_log.sfx_stop); i++)
    {
        if (g_rollback.confirm_frame >= g_audio_log.sfx_stop[i].frame)
        {
            NetLog("SFX: expiring sfx STOP instance %08X for sg (%d) on frame %d\n", 
                g_audio_log.sfx_stop[i].fgm_instance, 
                g_audio_log.sfx_stop[i].sg,
                g_audio_log.sfx_stop[i].frame);
                
            g_audio_log.sfx_stop[i].frame = (u32)-1;
        }
    }

    // remove bgm events
    for (int i = 0; i < GetElementsIn(g_audio_log.bgm); i++)
    {
        if (g_rollback.confirm_frame >= g_audio_log.bgm[i].frame)
        {
            NetLog("SFX: expiring bgm kind %d for entrynum %08X on frame %d\n", 
                g_audio_log.bgm[i].event_kind, 
                g_audio_log.bgm[i].entrynum,
                g_audio_log.bgm[i].frame);

            g_audio_log.bgm[i].frame = (u32)-1;
        }
    }
}

int Audio_CheckStopLog(FGMInstance fgm)
{
    // lets make sure it wasn't stopped    
    for (int i = 0; i < GetElementsIn(g_audio_log.sfx_stop); i++)
    {
        if (g_audio_log.sfx_stop[i].fgm_instance == fgm)
            return true;
    }

    return false;
}
int Audio_RemoveFromSFXLog(FGMInstance fgm_instance)
{
    // stopping an invalid SFX
    if (fgm_instance == (u32)-1)
        return 0;

    // stopping an SFX before the log is initialized
    if (!(Scene_GetCurrentMinor() == MNRKIND_3D && g_audio_log.enable))
        return 0;

    u32 this_frame = Gm_GetGameData()->update.engine_frames;

    int next_free_idx = -1;

    // check if we stopped the sound already
    for (int i = 0; i < GetElementsIn(g_audio_log.sfx_stop); i++)
    {
        // remember next free slot so we dont have to iterate again
        if (next_free_idx == -1 && g_audio_log.sfx_stop[i].frame == -1)
            next_free_idx = i;

        if (this_frame == g_audio_log.sfx_stop[i].frame && 
            g_audio_log.sfx_stop[i].fgm_instance == fgm_instance)
        {
            NetLog("SFX:  skipping sfx END on frame %d. matches instance %08X from frame %d\n", this_frame, g_audio_log.sfx_stop[i].fgm_instance, g_audio_log.sfx_stop[i].frame);
            return 1;
        }
    }

    // log it for the future
    if (next_free_idx != -1)
    {
        SFXLog *next_free = &g_audio_log.sfx_stop[next_free_idx];
        next_free->frame = this_frame;
        next_free->fgm_instance = fgm_instance;

        NetLog("SFX: stopped sfx with instance %08X on frame %d\n", fgm_instance, this_frame);
    }
    else
        NetLog("audio_log END over!!\n");

    return 0;
}
CODEPATCH_HOOKCREATE(0x80440844, "stwu	1, -0x0014 (1)\n\t"
                                 "mflr 0\n\t"
                                 "stw 0, 0x18(1)\n\t"
                                 "stw 3, 0x10(1)\n\t",
                                 Audio_RemoveFromSFXLog, 
                                 "lwz    4, 0x10(1)\n\t"
                                 "lwz    0, 0x0018 (1)\n\t" 
                                 "mtlr	 0\n\t" 
                                 "addi	 1, 1, 0x0014\n\t"
                                 "cmpwi  3,0\n\t"
                                 "beq    0x8\n\t"
                                 "blr\n\t"
                                 "mr 3, 4\n\t", 0)

FGMInstance SFXLog_OnSFXPlay(int sfx_id, int volume, int pan, int r6, int r7, u8 r8, u8 r9, u32 audio_track, int sg)
{
    FGMInstance (*_SFX_Play)(int sfx_id, int volume, int pan, int r6, int r7, u8 r8, u8 r9, u32 audio_track, int sg) = (void *)0x80442674;

    if (!(Scene_GetCurrentMinor() == MNRKIND_3D && g_audio_log.enable))
        return _SFX_Play(sfx_id, volume, pan, r6, r7, r8, r9, audio_track, sg);
        
    u32 this_frame = Gm_GetGameData()->update.engine_frames;

    int next_free_idx = -1;

    // check if we played the sound already
    for (int i = 0; i < GetElementsIn(g_audio_log.sfx_start); i++)
    {
        // remember next free slot so we dont have to iterate again
        if (next_free_idx == -1 && g_audio_log.sfx_start[i].frame == -1)
            next_free_idx = i;

        if (this_frame == g_audio_log.sfx_start[i].frame && 
            sfx_id == g_audio_log.sfx_start[i].sfx_id)
        {
            NetLog("SFX: skipping PLAY %08X on frame %d. matches instance %08X from frame %d\n", 
                sfx_id, 
                this_frame, 
                g_audio_log.sfx_start[i].fgm_instance, 
                g_audio_log.sfx_start[i].frame);

            // lets assume we played this sound already
            // lets update the sound with the params passed in in case they are
            // different
            int level = OSDisableInterrupts();
            FGMInstanceData *fgm = &(*stc_fgm_data_unk)[g_audio_log.sfx_start[i].fgm_instance & 0xFF];
            if (g_rollback.is_resim_frame && 
                g_audio_log.sfx_start[i].fgm_instance == fgm->instance && // if the sound was short, it may have stopped playing and these wont match
                (fgm->audio_track != audio_track || fgm->sg != sg))
            {          
                NetLog(" id %d:%d with instance %08X is using track %d and sg %d. requested to play with track %d and sg %d\n",
                (fgm->sfx_id & 0xFFFF0000) >> 16, fgm->sfx_id & 0xFFFF,
                fgm->instance,
                fgm->audio_track,
                fgm->sg,
                audio_track,
                sg);

                NetLog(" FGM:\n  sfx: %d:%d\n  instance: %08X\n  track: %d\n  sg: %d\n",
                (g_audio_log.sfx_start[i].sfx_id & 0xFFFF0000) >> 16, g_audio_log.sfx_start[i].sfx_id & 0xFFFF,
                g_audio_log.sfx_start[i].fgm_instance,
                g_audio_log.sfx_start[i].audio_track,
                g_audio_log.sfx_start[i].sg);

                NetLog(" Cache:\n  sfx: %d:%d\n  instance: %08X\n  track: %d\n  sg: %d\n",
                (fgm->sfx_id & 0xFFFF0000) >> 16, fgm->sfx_id & 0xFFFF,
                fgm->instance,
                fgm->audio_track,
                fgm->sg);

                NetLog(" Req:\n  sfx: %d:%d\n  track: %d\n  sg: %d\n",
                (sfx_id & 0xFFFF0000) >> 16, sfx_id & 0xFFFF,
                audio_track,
                sg);

                // update fgm's data
                fgm->audio_track = audio_track;
                fgm->sg = sg;

                // update VPB's data
                VPB *vpb = &ax_live->voice_data[fgm->pid & AXDRIVER_PIDMASK];
                vpb->sg = sg;

                NetLog(" Updated FGM %08X (%p) to track: %d  sg: %d\n",
                fgm->instance,
                fgm,
                audio_track,
                sg);

                // maybe manually update the sound using the emitter params? idk
                // if i change the emitters volume level it should force a voice change next
                // emitter update
            }
            OSRestoreInterrupts(level);

            g_audio_log.sfx_start[i].is_replayed = true;
            return g_audio_log.sfx_start[i].fgm_instance;
        }
    }

    // lets play it
    int fgm_instance = _SFX_Play(sfx_id, volume, pan, r6, r7, r8, r9, audio_track, sg);

    // only log sounds played on prediction frames
    // if (!g_rollback.is_resim_frame)
    {
        // log it for the future
        if (next_free_idx != -1)
        {
            SFXLog *next_free = &g_audio_log.sfx_start[next_free_idx];
            next_free->frame = this_frame;
            next_free->sfx_id = sfx_id;
            next_free->audio_track = audio_track;
            next_free->fgm_instance = fgm_instance;
            next_free->sg = sg;
            next_free->resim_idx = g_rollback.resim_idx;
            next_free->is_replayed = false;

            NetLog("SFX: played sfx %08X for sg (%d) with instance %08X on frame %d\n", 
                sfx_id, 
                sg,
                fgm_instance,
                this_frame);
        }
        else
            NetLog("audio_log PLAY over!!\n");
    }

    return fgm_instance;
}

int BGMLog_OnPlay(char *file_name, int volume, int pan, int r6, int r7, int r8, int r9, int r10, int sg, int slot)
{
    int (*_BGM_Play)(char *file_name, int volume, int pan, int r6, int r7, int r8, int r9, int r10, int sg, int slot) = (void *)0x804452a0;

    if (!(Scene_GetCurrentMinor() == MNRKIND_3D && g_audio_log.enable && g_rollback.is_resim_frame))
        return _BGM_Play(file_name, volume, pan, r6, r7, r8, r9, r10, sg, slot);

    u32 this_frame = Gm_GetGameData()->update.engine_frames;
    int next_free_idx = -1;

    u32 entrynum = DVDConvertPathToEntrynum(file_name);

    // check if we played the music already
    for (int i = 0; i < GetElementsIn(g_audio_log.bgm); i++)
    {
        // remember next free slot so we dont have to iterate again
        if (next_free_idx == -1 && g_audio_log.bgm[i].frame == -1)
            next_free_idx = i;

        if (this_frame == g_audio_log.bgm[i].frame && 
            g_audio_log.bgm[i].event_kind == BGMEVENT_PLAY &&
            g_audio_log.bgm[i].slot == slot &&
            g_audio_log.bgm[i].entrynum == entrynum)
        {
            return g_audio_log.bgm[i].slot;
        }
    }

    // lets play it
    int result = _BGM_Play(file_name, volume, pan, r6, r7, r8, r9, r10, sg, slot);

    // log it for the future
    if (next_free_idx != -1)
    {
        BGMLog *next_free = &g_audio_log.bgm[next_free_idx];
        next_free->frame = this_frame;
        next_free->event_kind = BGMEVENT_PLAY;
        next_free->entrynum = entrynum;
        next_free->slot = slot;

        NetLog("BGM: played bgm %s with slot %08X on frame %d\n", file_name, slot, this_frame);
    }
    else
        NetLog("m_audio_log.bgm over!!\n");

    return result;

}

Text *rng_text;
void Netsync_CreateRNGText()
{
    // display test string
    Text *t = Hoshi_CreateScreenText();
    t->kerning = 1;
    t->use_aspect = 1;
    t->trans = (Vec3){480, 0, 0};
    t->viewport_scale = (Vec2){0.5, 0.5};
    t->aspect = (Vec2){320, 32};
    t->viewport_color = (GXColor){0, 0, 0, 128};
    Text_AddSubtext(t, 0, 0, "RNG Seed: %08X");

    rng_text = t;
}
void Netsync_UpdateRNGText()
{
    Text_SetText(rng_text, 0, "RNG Seed: %08X", **stc_rng_seed);
}

static Text *frame_text;
void Netsync_CreateFrameText()
{
    // display test string
    Text *t = Hoshi_CreateScreenText();
    t->kerning = 1;
    t->use_aspect = 1;
    t->trans = (Vec3){300, 0, 0};
    t->viewport_scale = (Vec2){0.2, 0.2};
    t->aspect = (Vec2){260, 32};
    t->viewport_color = (GXColor){0, 0, 0, 128};
    Text_AddSubtext(t, 0, 0, "");

    frame_text = t;
}
void Netsync_UpdateFrameText()
{
    if (frame_text)
        Text_SetText(frame_text, 0, "Frame: %d", Gm_GetGameData()->update.engine_frames);
}

void Netsync_On3DLoadStart()
{
    // tell dolphin we are starting rollback
    Netplay_StartRollback();

    // lower audio log flag when initializing the 3D scene.
    // we dont take our first savestate until after this executes 
    // entirely so there is no point in logging any sounds during this time
    g_audio_log.enable = 0;
}
void Netsync_On3DExit()
{
    // tell dolphin we are ending rollback
    Netplay_EndRollback();
}
void Netsync_OnSceneChange()
{
    // Netsync_CreateRNGText();
    Netsync_CreateFrameText();
}
void Netsync_OnFrameEnd()
{
    Audio_Cleanup();
    Netsync_UpdateFrameText();

    // send frame hash
    int hash_flags = (Scene_GetCurrentMinor() == MNRKIND_3D) ? (HASH_ALL) : (HASH_SYS);
    Netplay_SendGameState(Hash_GameState(hash_flags));

    // int index = (stc_bgm_pid[1] & AXDRIVER_PIDMASK);
    // OSReport("VPB: %x\n", &ax_live->voice_data[index]);
    // OSReport("PB Addr: %p\n", &ax_live->voice_data[index].axvpb[0]->pb.addr);


    // u32 hash = Replay_HashGameState();
    // NetLog(" frame end. hash: %08X  rng: %08X\n", hash, *hsd_rand_seed);
}

void Netsync_Init()
{
    Netsync_AdjustGameLoop();

    // rollback frame optimizations
    // CODEPATCH_HOOKAPPLY(0x800614bc);
    // CODEPATCH_HOOKAPPLY(0x8018fd6c);
    // CODEPATCH_HOOKAPPLY(0x801c6b20);

    // prevent duplicate sfx
    CODEPATCH_REPLACECALL(0x80442a40, SFXLog_OnSFXPlay);
    CODEPATCH_HOOKAPPLY(0x80440844);
    
    // prevent duplicate music events
    CODEPATCH_REPLACECALL(0x804456f0, BGMLog_OnPlay);
    
    // pause menu always uses 1p button directions on netplay (so spectators dont desync)
    if (Dolphin_IsNetplay())
        CODEPATCH_REPLACEINSTRUCTION(0x80040fd4, 0x38600001);

    // temp disable music
    // CODEPATCH_REPLACEINSTRUCTION(0x804456c0, 0x4e800020);
}