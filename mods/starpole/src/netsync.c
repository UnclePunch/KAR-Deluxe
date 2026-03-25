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

void *(*OSGetCaller)(int level) = (void *)0x80428618;

HSD_Pad *sys_pads = (HSD_Pad *)0x8058b0e4;

void (*HSD_InsertIntoPadQueue)(PADStatus *status, int unk) = (void *)0x80412480;
void (*HSD_PadConsume)() = (void *)0x80062978;

extern int is_netplay;
extern StarpoleDataDolphin *dolphin_data;

DebugPadData g_debug_pad = {.head = 0};

AudioLog g_audio_log;
RollbackLog g_rollback;

PADStatus g_local_status[4];
PADStatus g_remote_status[MAX_ROLLBACK_FRAMES + 1][4];
PreserveMemRegion g_preserve_regions[] = {
    {(void *)0, 0},                                     // stay
    {(void *)0, 0},                                     // AllM
    {(void *)0x00000000, 0x96000},                      // XFB buffer 1 80589a48
    {(void *)0x00000000, 0x96000},                      // XFB buffer 2 80589a4c
    {(void *)0x00000000, 0x80000},                      // gx init alloc in arena lo, performed at 8040fc3c
    {(void *)0, 0},                                     // audio heap
    
    {(void *)(0x805f6390 - (32 * 1024)), 32 * 1024},    // stack, address derived from 80005410. im inferring 32kb stack
    {&g_audio_log, sizeof(g_audio_log)},                // audio log
    {&g_rollback, sizeof(g_rollback)},              // rollback specific data
    {&g_debug_pad, sizeof(g_debug_pad)},                // debug pad data


    {(void *)0x80003100, 0x2500},                   // dol text section 1
    {(void *)0x80005800, 0x483C40},                 // dol text section 2

    {(void *)0x80550f68, 0x1008},                   // file preload table
    {(void *)0x80508bc8, 0x4 * 3},                  // BGM PID's. needed to stop a song from playing
    
    {(void *)(0x805dd0e0 + 0xF20), 0xF68 - 0xF20},    // ARQ and hsd audio sbss
    {(void *)(0x805dd0e0 + 0xAC), 0x8},               // 64 bitfield that is raised when the corresponding sg has its volume changed, 0x8044c450
    {(void *)0x80599c60, 0x8059a818 - 0x80599c60},  // more audio stuff. sg indexed audio data in here @ 8059a178 and 8059a160?
    {(void *)(0x805dd0e0 + 0x1358), 0x1470 - 0x1358}, // hsd audio sbss
    {(void *)0x8056ccb4, 0x24},                     // DVD Waiting Queue
    {(void *)0x8056cb40, 0xE0},                     // DVD Interrupt stuff @ 803c40b4. includes alarm
    {(void *)0x8056cc20, 0x94},                     // DVD state stuff @ 803c67f0. another alarm at 0x70 of this?

    {(void *)(0x805dd0e0 + 0xC60), 0xCF4 - 0xC60},      // disc read variables
    {(void *)(0x805dd0e0 + 0xDC8), 0xDD0 - 0xDC8},      // OSAlarm variables
    {(void *)(0x805dd0e0 + 0x4C8), 0x4},                // file async load flag

    {(void *)0x8056e9e8, 0x80587A60 - 0x8056e9e8},     // all the AX data i know of, AXStack head -> end of __AXVPB
    {(void *)(0x805dd0e0 + 0xF30), 0x1054 - 0xF30},      // AX region sbss

    {(void *)0x8058e298, 64 * 0x4},                   // array of VPB pointers? indexed by FGMInstance index
    {(void *)0x8058E398, 0x8F8},                      // unknown in between chunks, part of this is the fgm_kind struct, referenced @ 80442a24
    {(void *)0x8058ec90, 0x90},                       // hps stream unk struct @ 804464bc
    {(void *)0x8058ed20, 2 * 0x4000},                 // hps double buffer?
    {(void *)0x80596d20, 0x40},                       // hps streaming @ 80446a74
    {(void *)0x80596d60, 0x50},                       // hps streaming stuff
    {(void *)0x80596da0, 160 + (512*3)},              // FGM region, multiple offsets of this loaded around 80447ee4. also includes some HPS streaming stuff
    {(void *)0x80597440, 0x220},                      // unknown in between chunks

    {(void *)0x80597660, 64 * 0x98},                  // AXLive voice array. (8044ccf0)
    {(void *)0x80597F20, 64 * 152},                   // static audio lookup 0X8c0 (8044ccf0)

    {(void *)0x8059a880, 0x618},                      // memcard thread data? referenced by the function 8045b848 in the thread func
    {(void *)0x805b4698, 0x35C},                      // memcard thread data
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

    return;
}

int Netplay_StartRollback()
{
    int result = 0;
    int enable = OSDisableInterrupts();

    // create buffer of memory regions to preserve
    char buffer[sizeof(g_preserve_regions)] __attribute__((aligned(32)));
    Netplay_GetPreserveRegions((PreserveMemRegion *)&buffer);

    // notify of incoming data
    if (Starpole_Imm(STARPOLE_CMD_NETSTART, GetElementsIn(g_preserve_regions)) <= 0)
    {
        OSReport("Starpole: unable to start rollback.\n");
        goto CLEANUP;
    }

    // send it
    if (!Starpole_DMA((StarpoleBuffer *)buffer, sizeof(buffer), EXI_WRITE))
        goto CLEANUP;

    result = 1;

CLEANUP:
    OSRestoreInterrupts(enable);
    return result;
}
int Netplay_EndRollback()
{
    int result = 0;
    int enable = OSDisableInterrupts();

    // notify of incoming data
    if (Starpole_Imm(STARPOLE_CMD_NETEND, 0) <= 0)
    {
        OSReport("Starpole: unable to end rollback.\n");
        goto CLEANUP;
    }

    result = 1;

CLEANUP:
    OSRestoreInterrupts(enable);
    return result;
}
int Netplay_RequestSave(u32 frame_idx)
{
    int result = 0;
    int enable = OSDisableInterrupts();

    // notify of incoming data
    if (Starpole_Imm(STARPOLE_CMD_NETSAVE, frame_idx) <= 0)
    {
        OSReport("Starpole: unable to savestate.\n");
        goto CLEANUP;
    }

    result = 1;

CLEANUP:
    OSRestoreInterrupts(enable);
    return result;
}
int Netplay_SendInputs(PADStatus *status)
{
    int result = 0;
    int enable = OSDisableInterrupts();

    // copy pad data to aligned buffer
    char buffer[sizeof(PADStatus) * 4] __attribute__((aligned(32)));
    memcpy(buffer, status, sizeof(buffer));

    // notify of incoming data
    if (Starpole_Imm(STARPOLE_CMD_NETPADSEND, sizeof(buffer)) <= 0)
    {
        OSReport("Starpole: unable to send pad data.\n");
        goto CLEANUP;
    }

    // send it
    if (!Starpole_DMA((StarpoleBuffer *)buffer, sizeof(buffer), EXI_WRITE))
        goto CLEANUP;

    result = 1;

CLEANUP:
    OSRestoreInterrupts(enable);
    return result;
}
int Netplay_ReceiveInputs()
{
    int sim_num;
    u32 this_frame_idx = Gm_GetGameData()->update.engine_frames;
    int enable = OSDisableInterrupts();

    // transfer buffer
    PADStatus buffer[4][MAX_ROLLBACK_FRAMES + 1] __attribute__((aligned(32)));

    // request data 
    sim_num = Starpole_Imm(STARPOLE_CMD_NETPADRECV, this_frame_idx);

    if (DOLPHIN_DEBUG)
    {
        // sim_num = (Scene_GetCurrentMinor() == MNRKIND_3D && Gm_GetGameData()->update.engine_frames > MAX_ROLLBACK_FRAMES) ? 6 : 1;
             
        // store all inputs to buffer
        memcpy(g_debug_pad.history[g_debug_pad.head], g_local_status, sizeof(g_local_status));
        g_debug_pad.head = (g_debug_pad.head + 1) % 10;
    }
    else
    {
        // request data 
        // frame_num = Starpole_Imm(STARPOLE_CMD_NETPADRECV, 0);

        if (sim_num <= 0)
        {
            OSReport("Starpole: inputs not ready.\n");
            goto CLEANUP;
        }

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
    return sim_num;
}

int Netplay_WaitForClients()
{
    // if (*(int *)(&stc_bgm_data_arr[1]) != -1)
    // {
    //     VPB *vbp = &ax_live->voice_data[stc_bgm_data_arr[1].vpb_index];
    //     AXVPB *axvpb = vbp->axvpb;
    //     int cur = *(int *)(&axvpb->pb.addr.currentAddressHi);
    //     OSReport("pb.addr: 0x%08X\n", &axvpb->pb.addr);
    // }

    PADRead(g_local_status);                      // poll inputs this frame
    Netplay_SendInputs(g_local_status);           // send inputs to dolphin

    // update game sim only when we have all inputs for this frame
    g_rollback.sim_frames = Netplay_ReceiveInputs();

    // OSReport("\nwill sim %d frames\n", g_rollback.sim_frames);

    // if we are running more than 1 frame we are resimulating after a rollback, validate sounds
    // if (g_rollback.sim_frames > 1)
    //     Audio_ValidateAX();

    return g_rollback.sim_frames;
}
CODEPATCH_HOOKCREATE(0x80006bd4, "", Netplay_WaitForClients, "", 0)

void Netplay_OnFrameStart(int loop_num)
{
    // request save/load
    Netplay_RequestSave(Gm_GetGameData()->update.engine_frames);

    OSReport("now simulating frame %d!\n", Gm_GetGameData()->update.engine_frames);

    g_rollback.this_sim_idx = loop_num;
    g_rollback.is_resim_frame = (g_rollback.sim_frames > 1 && g_rollback.this_sim_idx < (g_rollback.sim_frames - 1));

    // check if this is the final forward simulation frame after a resim
    if (g_rollback.sim_frames > 1 && g_rollback.this_sim_idx == (g_rollback.sim_frames - 1))
    {
        // we just finished resimulating after a rollback
        // lets cleanup AX state if sounds that shouldn't be playing are. lets also clear logs
        Audio_ValidateAX();
        Audio_ResetLogs(true);
    }

    // insert pad into queue
    if (DOLPHIN_DEBUG)
    {
        int frames_ago = (g_rollback.sim_frames - 1) - loop_num;
        int idx = (g_debug_pad.head - 1 - frames_ago + 10) % 10;
        HSD_InsertIntoPadQueue(g_debug_pad.history[idx], 0);
    }
    else
    {
        // insert the pad and consume it
        HSD_InsertIntoPadQueue(g_remote_status[loop_num], 0);
    }
}
CODEPATCH_HOOKCREATE(0x8000682c, "mr 3, 29\t\n", Netplay_OnFrameStart, "", 0)

void Netsync_OutputFrameHash()
{
    if (Scene_GetCurrentMinor() != MNRKIND_3D)
        return;

    // u32 hash = Replay_HashGameState();
    // OSReport(" frame end. hash: %08X  rng: %08X\n", hash, *hsd_rand_seed);
}
CODEPATCH_HOOKCREATE(0x80006a80, "", Netsync_OutputFrameHash, "", 0)

void Netsync_AdjustGameLoop()
{
    int *is_alarm_active = (int *)0x80550ca8;
    OSAlarm *alarm_ptr = (OSAlarm *)0x80550d28;

    // cancel any active alarms
    if (*is_alarm_active)
        OSCancelAlarm(alarm_ptr);
    CODEPATCH_REPLACEINSTRUCTION(0x80062660, 0x4e800020);       // disable pad alarm creation

    CODEPATCH_REPLACECALL(0x80006b94, VIWaitForRetrace);        // replace pad alarm jam with viwaitforretrace

    // netpause at the top of each frame
    // CODEPATCH_REPLACEINSTRUCTION(0x8000682c, 0x60000000);    // remove pad consume
    CODEPATCH_REPLACEINSTRUCTION(0x80006bd4, 0x60000000);       // remove GetPadQueue Call
    CODEPATCH_REPLACEINSTRUCTION(0x80006bdc, 0x60000000);       // render on 0 ticks
    CODEPATCH_HOOKAPPLY(0x80006bd4);                            // wait for inputs
    CODEPATCH_HOOKAPPLY(0x8000682c);                            // consume pad

    CODEPATCH_HOOKAPPLY(0x80006a80);                            // output frame hash
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
void Audio_InitLog()
{
    // raise audio log flag after initializing the 3D scene.
    // our first savestate occurs soon after this executes.
    g_audio_log.enable = 1;

    memset(g_audio_log.sfx_start, -1, sizeof(g_audio_log.sfx_start));
    memset(g_audio_log.sfx_stop, -1, sizeof(g_audio_log.sfx_stop));
    memset(g_audio_log.bgm, -1, sizeof(g_audio_log.bgm));
    memset(g_audio_log.sg, -1, sizeof(g_audio_log.sg));
    memset(g_audio_log.emitter, -1, sizeof(g_audio_log.emitter));
    memset(g_audio_log.track, -1, sizeof(g_audio_log.track));

    // clear log entries for sfx's on confirmed frames
    // GOBJ_EZCreator(0, 0, 0,
    //                 0, 0, 
    //                 0, 0, 
    //                 Audio_UpdateSFXLog, 17,
    //                 0, 0, 0);

    // handle free'ing future source and track allocations at the top of the frame
    // GOBJ_EZCreator(0, 0, 0,
    //                 0, 0, 
    //                 0, 0, 
    //                 Audio_UpdateLog, 0,
    //                 0, 0, 0);
}
void Audio_UpdateSFXLog()
{
    u32 this_frame = Gm_GetGameData()->update.engine_frames;

    if (!g_rollback.is_resim_frame)
        return;

    // remove start events for confirmed frames
    for (int i = 0; i < GetElementsIn(g_audio_log.sfx_start); i++)
    {
        if (g_audio_log.sfx_start[i].fgm_instance != (u32)-1 && 
            g_audio_log.sfx_start[i].frame < this_frame)
        {            
            //OSReport("SFX: expired sfx PLAY %08X from frame %d\n", g_audio_log.sfx_start[i].sfx_id, g_audio_log.sfx_start[i].frame);
            g_audio_log.sfx_start[i].frame = (u32)-1;
        }
    }

    // remove stop events for confirmed frames
    for (int i = 0; i < GetElementsIn(g_audio_log.sfx_stop); i++)
    {
        if (g_audio_log.sfx_stop[i].fgm_instance != (u32)-1 && 
            g_audio_log.sfx_stop[i].frame < this_frame)
        {            
            //OSReport("SFX: expired sfx END %08X from frame %d\n", g_audio_log.sfx_stop[i].sfx_id, g_audio_log.sfx_stop[i].frame);
            g_audio_log.sfx_stop[i].frame = (u32)-1;
        }
    }

    // remove bgm events for confirmed frames
    for (int i = 0; i < GetElementsIn(g_audio_log.bgm); i++)
    {
        if (g_audio_log.bgm[i].entrynum != (u32)-1 && 
            g_audio_log.bgm[i].frame < this_frame)
        {            
            g_audio_log.bgm[i].entrynum = (u32)-1;
        }
    }

    // remove sg alloc events for confirmed frames
    for (int i = 0; i < GetElementsIn(g_audio_log.sg); i++)
    {
        if (g_audio_log.sg[i].sg != (u32)-1 && 
            g_audio_log.sg[i].frame < this_frame)
        {            
            g_audio_log.sg[i].frame = (u32)-1;
        }
    }

    // remove sg alloc events for confirmed frames
    for (int i = 0; i < GetElementsIn(g_audio_log.emitter); i++)
    {
        if (g_audio_log.emitter[i].emitter.index != (u32)-1 && 
            g_audio_log.emitter[i].frame < this_frame)
        {            
            g_audio_log.emitter[i].frame = (u32)-1;
        }
    }

   // remove track alloc events for confirmed frames
    for (int i = 0; i < GetElementsIn(g_audio_log.sg); i++)
    {
        if (g_audio_log.track[i].audio_track != (u32)-1 && 
            g_audio_log.track[i].frame < this_frame)
        {            
            g_audio_log.track[i].frame = (u32)-1;
        }
    }
}
void Audio_ResetLogs(int is_clear_all)
{
    if (is_clear_all)
    {
        OSReport("SFX: clearing entire audio log after resimulating from a %d frame rollback\n", g_rollback.sim_frames - 1);

        // remove start events
        for (int i = 0; i < GetElementsIn(g_audio_log.sfx_start); i++)
        {
            g_audio_log.sfx_start[i].frame = (u32)-1;
        }

        // remove stop events
        for (int i = 0; i < GetElementsIn(g_audio_log.sfx_stop); i++)
        {
            g_audio_log.sfx_stop[i].frame = (u32)-1;
        }

        // remove bgm events
        for (int i = 0; i < GetElementsIn(g_audio_log.bgm); i++)
        {
            g_audio_log.bgm[i].entrynum = (u32)-1;
        }
    }

    // remove sg alloc events
    for (int i = 0; i < GetElementsIn(g_audio_log.sg); i++)
    {
        g_audio_log.sg[i].frame = (u32)-1;
    }

    // remove emitter alloc events
    for (int i = 0; i < GetElementsIn(g_audio_log.emitter); i++)
    {
     
        g_audio_log.emitter[i].frame = (u32)-1;
    }

   // remove track alloc events
    for (int i = 0; i < GetElementsIn(g_audio_log.sg); i++)
    {
        g_audio_log.track[i].frame = (u32)-1;
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
            // OSReport("SFX:  skipping sfx END on frame %d. matches instance %08X from frame %d\n", this_frame, g_audio_log.sfx_stop[i].fgm_instance, g_audio_log.sfx_stop[i].frame);
            return 1;
        }
    }

    // log it for the future
    if (next_free_idx != -1)
    {
        SFXLog *next_free = &g_audio_log.sfx_stop[next_free_idx];
        next_free->frame = this_frame;
        next_free->fgm_instance = fgm_instance;

        OSReport("SFX: stopped sfx with instance %08X on frame %d\n", fgm_instance, this_frame);
    }
    else
        OSReport("audio_log over!!\n");

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

// Sound Generator
int Audio_AssignSoundGenerator(AudioEmitterData *emitter_data, int slot_idx)
{
    if (!(Scene_GetCurrentMinor() == MNRKIND_3D && g_audio_log.enable && g_rollback.is_resim_frame))
        return 0;
        
    u32 this_frame = Gm_GetGameData()->update.engine_frames;

    // map emitters are id'd by their emitter pointer because map all emitters use 0 for their instance
    u32 this_emitter_instance = GET_EMITTER_INSTANCE(emitter_data);

    // check if we allocated this sg already
    for (int i = 0; i < GetElementsIn(g_audio_log.sg); i++)
    {
        if (this_frame == g_audio_log.sg[i].frame && 
            g_audio_log.sg[i].this_emitter.kind == emitter_data->kind && g_audio_log.sg[i].this_emitter.instance == this_emitter_instance)
        {
            int emitter_index = emitter_data - audio_3d_data->emitters;

            OSReport("SFX: giving old sg %d assignment to emitter (%d) %p (identifier %d:%d) on frame %d\n", 
                g_audio_log.sg[i].sg, 
                emitter_index, 
                emitter_data, 
                g_audio_log.sg[i].this_emitter.kind, 
                g_audio_log.sg[i].this_emitter.instance, 
                this_frame);
            
            // store sg to emitter
            u8 sg = g_audio_log.sg[i].sg;
            if (slot_idx == 0)
                emitter_data->sg = sg;
            else
                emitter_data->ig = sg;

            // mark sg as used
            audio_3d_data->sg_status[sg] = 1;

            // // if the sg was stolen from an existing emitter, null it
            // if (g_audio_log.sg[i].stolen_from_data)
            // {
            //     if (slot_idx == 0)
            //         g_audio_log.sg[i].stolen_from_data->sg = -1;
            //     else
            //         g_audio_log.sg[i].stolen_from_data->x40 = -1;
            // }

            // null log entry
            g_audio_log.sg[i].frame = -1;

            // return sg
            return sg;
        }
    }

    // if this is a correction frame and we missed cache, null the entire log
    if (g_rollback.is_resim_frame)
    {
        OSReport("SFX: missed sg cache, nulling audio log!\n");
        Audio_ResetLogs(false);
    }

    // allocate it normally
    return 0;

}
CODEPATCH_HOOKCONDITIONALCREATE(0x8005d720, "mr 3, 29\n\t"
                                            "mr 4, 30\n\t", Audio_AssignSoundGenerator,
                                            "", 0, 0x8005d84c)
void Audio_AddToSoundGeneratorLog(AudioEmitterData *emitter_data, u8 sg, u8 slot_idx, AudioEmitterData *stolen_from_data)
{
    if (!(Scene_GetCurrentMinor() == MNRKIND_3D && g_audio_log.enable))
        return;

    u32 this_frame = Gm_GetGameData()->update.engine_frames;

    // map emitters are id'd by their emitter pointer because map all emitters use 0 for their instance
    u32 this_emitter_instance = GET_EMITTER_INSTANCE(emitter_data);

    // log sg assignment
    for (int i = 0; i < GetElementsIn(g_audio_log.sg); i++)
    {
        if (g_audio_log.sg[i].frame == -1)
        {
            g_audio_log.sg[i].frame = this_frame;
            g_audio_log.sg[i].this_emitter.kind = emitter_data->kind;
            g_audio_log.sg[i].this_emitter.instance = this_emitter_instance;
            // g_audio_log.sg[i].stolen_from_data = stolen_from_data;
            g_audio_log.sg[i].sg = sg;
            g_audio_log.sg[i].slot_idx = slot_idx;

            int emitter_index = ((u32)emitter_data - (u32)audio_3d_data->emitters) / sizeof(*emitter_data);
            
            OSReport("SFX: logging sg %d assignment to emitter (%d) %p (identifier %d:%d) on frame %d\n", 
                    sg, 
                    emitter_index, 
                    emitter_data, 
                    g_audio_log.sg[i].this_emitter.kind, 
                    g_audio_log.sg[i].this_emitter.instance, 
                    this_frame);
            break;
        }
    }

    return;

}
CODEPATCH_HOOKCREATE(0x8005d830, "mr 3, 29\n\t"
                                 "mr 4, 31\n\t"
                                 "mr 5, 30\n\t"
                                 "mr 6, 27\n\t", Audio_AddToSoundGeneratorLog,
                                 "", 0)
CODEPATCH_HOOKCREATE(0x8005d6f8, "li 27, 0\n\t" "b 0x8\n\t", 0, "", 0)  // init emitter_data in use variable

// Audio Source

// Audio Emitter
AudioEmitter Audio_AllocEmitter(AudioEmitterKind kind, u32 instance)
{
    if (!(Scene_GetCurrentMinor() == MNRKIND_3D && g_audio_log.enable && g_rollback.is_resim_frame))
        return 0;
        
    u32 this_frame = Gm_GetGameData()->update.engine_frames;

    // check if we allocated this emitter already
    for (int i = 0; i < GetElementsIn(g_audio_log.emitter); i++)
    {
        if (this_frame == g_audio_log.emitter[i].frame && 
            g_audio_log.emitter[i].emitter.identifier.kind == kind && g_audio_log.emitter[i].emitter.identifier.instance == instance)
        {
            OSReport("SFX: using old emitter %d (%p) alloc for (identifier %d:%d) on frame %d\n", 
                g_audio_log.emitter[i].emitter.index, 
                &audio_3d_data->emitters[g_audio_log.emitter[i].emitter.index], 
                kind, 
                instance, 
                this_frame);

            // null log
            g_audio_log.emitter[i].frame = -1;

            // return sg
            return g_audio_log.emitter[i].emitter.index;
        }
    }

    // if this is a correction frame and we missed cache, null the entire log
    if (g_rollback.is_resim_frame)
    {
        OSReport("SFX: missed emitter cache, nulling audio log!\n");
        Audio_ResetLogs(false);
    }

    // allocate it normally
    return 0;
}
CODEPATCH_HOOKCONDITIONALCREATE(0x8005d88c,
                                 "mr 3, 26\n\t"
                                 "mr 4, 27\n\t",
                                 Audio_AllocEmitter, 
                                 "mr 29, 3\n\t", 0, 0x8005da0c)
void Audio_AddToEmitterLog(AudioEmitter emitter)
{
    if (!(Scene_GetCurrentMinor() == MNRKIND_3D && g_audio_log.enable && !g_rollback.is_resim_frame))
        return;

    u32 this_frame = Gm_GetGameData()->update.engine_frames;
    AudioEmitterData *emitter_data = &audio_3d_data->emitters[emitter];

    // log emitter alloc
    for (int i = 0; i < GetElementsIn(g_audio_log.emitter); i++)
    {
        if (g_audio_log.emitter[i].frame == -1)
        {
            g_audio_log.emitter[i].frame = this_frame;
            g_audio_log.emitter[i].emitter.index = emitter;
            g_audio_log.emitter[i].emitter.identifier.kind = emitter_data->kind;
            g_audio_log.emitter[i].emitter.identifier.instance = emitter_data->instance;
            
            OSReport("SFX: logging emitter %d (%p) alloc to (%d:%d) on frame %d\n", 
                    emitter, 
                    emitter_data, 
                    emitter_data->kind, 
                    emitter_data->instance, 
                    this_frame);
            break;
        }
    }

    return;

}
CODEPATCH_HOOKCREATE(0x8005db04, "mr 3, 29\n\t", Audio_AddToEmitterLog, "", 0)

// Audio Track
AudioTrackOwner AudioTrack_GetOwnerFromCaller(void *addr)
{
    static AudioTrackCallerMap track_alloc_caller_map[] = {
        {
            .addr = (void *)0x8005d140,
            .owner = AUDIOTRACKOWNER_RIDER,  
        },
        {
            .addr = (void *)0x8005d1c4,
            .owner = AUDIOTRACKOWNER_MACHINE,  
        },
        {
            .addr = (void *)0x8005d244,
            .owner = AUDIOTRACKOWNER_ENEMY,  
        },
        {
            .addr = (void *)0x8005d2c8,
            .owner = AUDIOTRACKOWNER_ITEM,  
        },
        {
            .addr = (void *)0x8005d34c,
            .owner = AUDIOTRACKOWNER_MAP,  
        },
        {
            .addr = (void *)0x8005d3d0,
            .owner = AUDIOTRACKOWNER_NONE,  
        },
        {
            .addr = (void *)0x8005d454,
            .owner = AUDIOTRACKOWNER_RIDER,  
        },
        {
            .addr = (void *)0x8005d4d8,
            .owner = AUDIOTRACKOWNER_MACHINE,  
        },
        {
            .addr = (void *)0x8005d55c,
            .owner = AUDIOTRACKOWNER_WEAPON,  
        },
        {
            .addr = (void *)0x8005d5dc,
            .owner = AUDIOTRACKOWNER_MAP,  
        },
    };

    for (int i = 0; i < GetElementsIn(track_alloc_caller_map); i++)
    {
        if (track_alloc_caller_map[i].addr == addr)
            return track_alloc_caller_map[i].owner;
    }

    OSReport("audio track owner unknown!!!\n");
    return -1;

}
int Audio_AssignTrack()
{
    if (!(Scene_GetCurrentMinor() == MNRKIND_3D && g_audio_log.enable && g_rollback.is_resim_frame))
        return 0;
        
    u32 this_frame = Gm_GetGameData()->update.engine_frames;

    // check what kind of object is allocating this track
    AudioTrackOwner track_owner = AudioTrack_GetOwnerFromCaller(OSGetCaller(1));

    // check if we allocated this track already
    for (int i = 0; i < GetElementsIn(g_audio_log.track); i++)
    {
        if (this_frame == g_audio_log.track[i].frame && 
            g_audio_log.track[i].owner == track_owner)
        {
            OSReport("SFX: giving old track %d assignment to owner kind %d on frame %d\n", 
                g_audio_log.track[i].audio_track, 
                g_audio_log.track[i].owner, 
                this_frame);

            // null this logged track
            g_audio_log.track[i].frame = -1;

            return g_audio_log.track[i].audio_track;
        }
    }

    // if this is a correction frame and we missed cache, null the entire log
    if (g_rollback.is_resim_frame)
    {
        OSReport("SFX: missed track cache, nulling audio log!\n");
        Audio_ResetLogs(false);
    }

    // allocate it normally
    return 0;

}
CODEPATCH_HOOKCONDITIONALCREATE(0x8005c6f0, "", Audio_AssignTrack, "", 0, 0x8005cf34)
void Audio_AddToTrackLog(u32 track)
{
    if (!(Scene_GetCurrentMinor() == MNRKIND_3D && g_audio_log.enable))
        return;

    u32 this_frame = Gm_GetGameData()->update.engine_frames;

    // log track assignment
    for (int i = 0; i < GetElementsIn(g_audio_log.track); i++)
    {
        if (g_audio_log.track[i].frame == -1)
        {
            g_audio_log.track[i].frame = this_frame;
            g_audio_log.track[i].audio_track = track;
            g_audio_log.track[i].owner = AudioTrack_GetOwnerFromCaller(OSGetCaller(1));

            OSReport("SFX: logging track %d assignment to owner kind %d on frame %d\n", 
                    g_audio_log.track[i].audio_track, 
                    g_audio_log.track[i].owner, 
                    this_frame);
            break;
        }
    }

    return;

}
CODEPATCH_HOOKCREATE(0x8005cf30, "mr 3, 18\n\t", Audio_AddToTrackLog, "", 0)

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
            g_audio_log.sfx_start[i].sfx_id == sfx_id && 
            g_audio_log.sfx_start[i].audio_track == audio_track)
        {
            // if (sfx_id == 0x00000000)
            // {
            //     bp();
            //     OSReport("SFX: spoofing MISS %08X with instance %08X from frame %d\n", 
            //         sfx_id, 
            //         g_audio_log.sfx_start[i].fgm_instance, 
            //         g_audio_log.sfx_start[i].frame);
            //     break;
            // }

            OSReport("SFX: skipping PLAY %08X on frame %d. matches instance %08X from frame %d\n", sfx_id, this_frame, g_audio_log.sfx_start[i].fgm_instance, g_audio_log.sfx_start[i].frame);
            g_audio_log.sfx_start[i].is_replayed = true;
            return g_audio_log.sfx_start[i].fgm_instance;
        }
    }

    // lets play it
    int fgm_instance = _SFX_Play(sfx_id, volume, pan, r6, r7, r8, r9, audio_track, sg);

    // if this is a correction frame and we missed cache, null the entire log
    if (g_rollback.is_resim_frame)
    {
        OSReport("SFX: missed sfx cache, nulling audio log!\n");
        OSReport("     unable to find sfx %08x while resimming frame %d\n", sfx_id, this_frame);
        Audio_ResetLogs(false);
    }

    // only log sounds played on prediction frames
    else
    {
        // log it for the future
        if (next_free_idx != -1)
        {
            SFXLog *next_free = &g_audio_log.sfx_start[next_free_idx];
            next_free->frame = this_frame;
            next_free->sfx_id = sfx_id;
            next_free->audio_track = audio_track;
            next_free->fgm_instance = fgm_instance;
            next_free->is_replayed = false;

            OSReport("SFX: played sfx %08X for sg (%d) with instance %08X on frame %d\n", 
                sfx_id, 
                sg,
                fgm_instance,
                this_frame);
        }
        else
            OSReport("audio_log over!!\n");
    }

    return fgm_instance;
}

int BGMLog_OnPlay(char *file_name, int volume, int pan, int r6, int r7, int r8, int r9, int r10, int slot, int sp2)
{
    int (*_BGM_Play)(char *file_name, int volume, int pan, int r6, int r7, int r8, int r9, int r10, int slot, int sp2) = (void *)0x804452a0;

    if (!(Scene_GetCurrentMinor() == MNRKIND_3D && g_audio_log.enable && g_rollback.is_resim_frame))
        return _BGM_Play(file_name, volume, pan, r6, r7, r8, r9, r10, slot, sp2);

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
    int result = _BGM_Play(file_name, volume, pan, r6, r7, r8, r9, r10, slot, sp2);

    // log it for the future
    if (next_free_idx != -1)
    {
        BGMLog *next_free = &g_audio_log.bgm[next_free_idx];
        next_free->frame = this_frame;
        next_free->event_kind = BGMEVENT_PLAY;
        next_free->entrynum = entrynum;
        next_free->slot = slot;

        OSReport("BGM: played bgm %s with slot %08X on frame %d\n", file_name, slot, this_frame);
    }
    else
        OSReport("m_audio_log.bgm over!!\n");

    return result;

}
int Audio_SpoofNoFGMInstanceMatch()
{
    return 0;
}

void Emitter_NullSoundGenerator(u32 sg)
{
    for (int i = 0; i < GetElementsIn(audio_3d_data->emitters); i++)
    {
        if (audio_3d_data->emitters[i].kind != AUDIOEMITTER_NONE)
        {
            if (audio_3d_data->emitters[i].ig == sg)
                audio_3d_data->emitters[i].ig = -1;
            else if (audio_3d_data->emitters[i].sg == sg)
                audio_3d_data->emitters[i].sg = -1;
        }
    }
}
void FGM_StopAllUsingSoundGenerator(u32 sg)
{
    int level = OSDisableInterrupts();

    FGMInstance fgm_to_stop_arr[16];
    int fgm_to_stop_num = 0;
    
    for (FGMInstanceData *fgm = (*stc_fgm_data_start); fgm; fgm = fgm->next)
    {
        if (fgm->sg == sg)
            fgm_to_stop_arr[fgm_to_stop_num++] = fgm->instance;
    }

    // stop em all
    for (int i = 0; i < fgm_to_stop_num; i++)
        FGM_Stop(fgm_to_stop_arr[i]);

    OSRestoreInterrupts(level);
}
void Audio_ValidateAX()
{
    int (*Audio_GetFGMusingSoundGenerator)(int sg) = (void *)0x80443d8c;

    for (int i = 0; i < GetElementsIn(g_audio_log.sfx_start); i++)
    {
        // stop sounds that weren't replayed 
        // we can probably assume they shouldn't be playing anymore
        if (g_audio_log.sfx_start[i].frame != -1 &&
            !g_audio_log.sfx_start[i].is_replayed && 
            !Audio_CheckStopLog(g_audio_log.sfx_start[i].fgm_instance))
        {
            bp();

            OSReport("SFX: stopping sfx %08X with fgm instance %08X from frame %d due to not being replayed on resim\n", 
                g_audio_log.sfx_start[i].sfx_id,
                g_audio_log.sfx_start[i].fgm_instance,
                g_audio_log.sfx_start[i].frame);
            FGM_Stop(g_audio_log.sfx_start[i].fgm_instance);
        }
    }

    // for (int i = 5; i < 64; i++)
    // {
    //     int sound_is_playing = Audio_GetFGMusingSoundGenerator(i) > 0;
    //     int sg_marked_inuse  = audio_3d_data->sg_status[i] == 1;

    //     if (sound_is_playing && !sg_marked_inuse)
    //     {
    //         // Voice was grabbed and played during prediction frames.
    //         // The restored emitter thinks it owns this sg but it was
    //         // reassigned, null the reference so the game doesnt try
    //         // to update parameters on a voice it no longer owns
    //         // Do NOT stop the sound; it belongs to whoever owns it now.
    //         Emitter_NullSoundGenerator(i);
    //     }
    //     else if (!sound_is_playing && sg_marked_inuse)
    //     {
    //         // Sound finished during prediction frames.
    //         // Free the sg so the game can reallocate it.
    //         audio_3d_data->sg_status[i] = 0;
    //         Emitter_NullSoundGenerator(i);
    //     }
    //     // sound_is_playing && sg_marked_inuse: voice is intact, leave alone
    //     // !sound_is_playing && !sg_marked_inuse, nothing to do.
    // }
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

    // disable live fgm instance references
    // CODEPATCH_REPLACEFUNC(0x80443e9c, Audio_SpoofNoFGMInstanceMatch);
    // CODEPATCH_REPLACEFUNC(0x80443d8c, Audio_SpoofNoFGMInstanceMatch);
    CODEPATCH_REPLACEINSTRUCTION(0x80256a14, 0x38600000);
    
    // prevent duplicate music events
    CODEPATCH_REPLACECALL(0x804456f0, BGMLog_OnPlay);
    
    // audio emitter allocs 
    CODEPATCH_HOOKAPPLY(0x8005d88c); 
    CODEPATCH_HOOKAPPLY(0x8005db04);

    // sg assignment
    CODEPATCH_HOOKAPPLY(0x8005d720);
    CODEPATCH_HOOKAPPLY(0x8005d830);
    CODEPATCH_HOOKAPPLY(0x8005d6f8);

    // track assignment
    CODEPATCH_HOOKAPPLY(0x8005c6f0);
    CODEPATCH_HOOKAPPLY(0x8005cf30);

    // temp disable music
    // CODEPATCH_REPLACEINSTRUCTION(0x804456c0, 0x4e800020);
}