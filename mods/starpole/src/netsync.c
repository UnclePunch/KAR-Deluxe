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

HSD_Pad *sys_pads = (HSD_Pad *)0x8058b0e4;

void (*HSD_InsertIntoPadQueue)(PADStatus *status, int unk) = (void *)0x80412480;
void (*HSD_PadConsume)() = (void *)0x80062978;

extern int is_netplay;
extern StarpoleDataDolphin *dolphin_data;

int m_local_status_history_head = 0;
PADStatus m_local_status_history[10][4];

PADStatus m_local_status[4];
PADStatus m_remote_status[MAX_ROLLBACK_FRAMES + 1][4];

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
    int frame_num;
    int enable = OSDisableInterrupts();

    // transfer buffer
    PADStatus buffer[4][MAX_ROLLBACK_FRAMES + 1] __attribute__((aligned(32)));

    // request data 
    frame_num = Starpole_Imm(STARPOLE_CMD_NETPADRECV, 0);

    if (DOLPHIN_DEBUG)
    {
        // frame_num = (Scene_GetCurrentMinor() == MNRKIND_3D && Gm_GetGameData()->update.engine_frames > MAX_ROLLBACK_FRAMES) ? 6 : 1;
             
        // store all inputs to buffer
        memcpy(m_local_status_history[m_local_status_history_head], m_local_status, sizeof(m_local_status));
        m_local_status_history_head = (m_local_status_history_head + 1) % 10;
    }
    else
    {
        // request data 
        // frame_num = Starpole_Imm(STARPOLE_CMD_NETPADRECV, 0);

        if (frame_num <= 0)
        {
            OSReport("Starpole: inputs not ready.\n");
            goto CLEANUP;
        }

        // receive inputs
        if (!Starpole_DMA((StarpoleBuffer *)buffer, sizeof(buffer), EXI_READ))
            goto CLEANUP;

        // copy to remote inputs
        memcpy(m_remote_status, buffer, sizeof(buffer));
    }

    // // duplicate input for now
    // for (int i = 1; i < GetElementsIn(m_remote_status[0]) - 1; i++)
    //     memcpy(&m_remote_status[i], &m_remote_status[0], (sizeof(m_remote_status[0])));


CLEANUP:
    OSRestoreInterrupts(enable);
    return frame_num;
}
int Netplay_RequestSave()
{
    int result = 0;
    int enable = OSDisableInterrupts();

    // request data
    if (Starpole_Imm(STARPOLE_CMD_NETSAVE, 0) <= 0)
    {
        OSReport("Starpole: not savestating on catch up frame.\n");
        goto CLEANUP;
    }

    result = 1;

CLEANUP:
    OSRestoreInterrupts(enable);
    return result;
}

int m_sim_frames;
u8 is_confirm_frame;
int Netplay_WaitForClients()
{
    // if (*(int *)(&stc_bgm_data_arr[1]) != -1)
    // {
    //     VPB *vbp = &ax_live->voice_data[stc_bgm_data_arr[1].vpb_index];
    //     AXVPB *axvpb = vbp->axvpb;
    //     int cur = *(int *)(&axvpb->pb.addr.currentAddressHi);
    //     OSReport("pb.addr: 0x%08X\n", &axvpb->pb.addr);
    // }

    PADRead(m_local_status);                      // poll inputs this frame
    Netplay_SendInputs(m_local_status);           // send inputs to dolphin

    // update game sim only when we have all inputs for this frame
    m_sim_frames = Netplay_ReceiveInputs();

    // OSReport("\nwill sim %d frames\n", m_sim_frames);

    return m_sim_frames;
}
CODEPATCH_HOOKCREATE(0x80006bd4, "", Netplay_WaitForClients, "", 0)

int m_this_sim_idx;
void Netplay_OnFrameStart(int loop_num)
{
    // OSReport("now simulating frame %d!\n", Gm_GetGameData()->update.engine_frames);
    m_this_sim_idx = loop_num;
    is_confirm_frame = (m_sim_frames == (MAX_ROLLBACK_FRAMES + 1) && m_this_sim_idx == 0);

    // insert pad into queue
    if (DOLPHIN_DEBUG)
    {
        int frames_ago = (m_sim_frames - 1) - loop_num;
        int idx = (m_local_status_history_head - 1 - frames_ago + 10) % 10;
        HSD_InsertIntoPadQueue(m_local_status_history[idx], 0);
    }
    else
    {
        // insert the pad and consume it
        HSD_InsertIntoPadQueue(m_remote_status[loop_num], 0);
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
    return (((m_sim_frames - 1) - m_this_sim_idx) == 0);
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
u8 m_audio_log_enable = 0;
SFXLog m_sfx_start_log[64] = {-1};
SFXLog m_sfx_stop_log[64] = {-1};
BGMLog m_bgm_log[16] = {-1};
SourceLog m_source_log[64] = {-1};
TrackLog m_track_log[64] = {-1};
SoundGeneratorLog m_sg_log[64] = {-1};
void Audio_On3DLoadStart()
{
    // lower audio log flag when initializing the 3D scene.
    // we dont take our first savestate until after this executes 
    // entirely so there is no point in logging any sounds during this time
    m_audio_log_enable = 0;
}
void Audio_InitLog()
{
    // raise audio log flag after initializing the 3D scene.
    // our first savestate occurs soon after this executes.
    m_audio_log_enable = 1;

    memset(m_sfx_start_log, -1, sizeof(m_sfx_start_log));
    memset(m_sfx_stop_log, -1, sizeof(m_sfx_stop_log));
    memset(m_bgm_log, -1, sizeof(m_bgm_log));
    
    // memset(m_source_log, -1, sizeof(m_source_log));
    // memset(m_track_log, -1, sizeof(m_track_log));
    // memset(m_sg_log, -1, sizeof(m_sg_log));

    // clear log entries for sfx's on confirmed frames
    GOBJ_EZCreator(0, 0, 0,
                    0, 0, 
                    0, 0, 
                    Audio_UpdateSFXLog, 17,
                    0, 0, 0);

    // handle free'ing future source and track allocations at the top of the frame
    GOBJ_EZCreator(0, 0, 0,
                    0, 0, 
                    0, 0, 
                    Audio_UpdateLog, 0,
                    0, 0, 0);
}
void Audio_UpdateSFXLog()
{
    u32 this_frame = Gm_GetGameData()->update.engine_frames;

    if (!is_confirm_frame)
        return;

    // remove start events for confirmed frames
    for (int i = 0; i < GetElementsIn(m_sfx_start_log); i++)
    {
        if (m_sfx_start_log[i].fgm_instance != (u32)-1 && 
            m_sfx_start_log[i].frame < this_frame)
        {            
            //OSReport("SFX: expired sfx PLAY %08X from frame %d\n", m_sfx_start_log[i].sfx_id, m_sfx_start_log[i].frame);
            m_sfx_start_log[i].frame = (u32)-1;
        }
    }

    // remove stop events for confirmed frames
    for (int i = 0; i < GetElementsIn(m_sfx_stop_log); i++)
    {
        if (m_sfx_stop_log[i].fgm_instance != (u32)-1 && 
            m_sfx_stop_log[i].frame < this_frame)
        {            
            //OSReport("SFX: expired sfx END %08X from frame %d\n", m_sfx_stop_log[i].sfx_id, m_sfx_stop_log[i].frame);
            m_sfx_stop_log[i].frame = (u32)-1;
        }
    }

    // remove stop events for confirmed frames
    for (int i = 0; i < GetElementsIn(m_bgm_log); i++)
    {
        if (m_bgm_log[i].entrynum != (u32)-1 && 
            m_bgm_log[i].frame < this_frame)
        {            
            m_bgm_log[i].entrynum = (u32)-1;
        }
    }

    // // remove sg alloc events for confirmed frames
    // for (int i = 0; i < GetElementsIn(m_sg_log); i++)
    // {
    //     if (m_sg_log[i].audio_source_data != (u32)-1 && 
    //         m_sg_log[i].frame < this_frame)
    //     {            
    //         m_sg_log[i].frame = (u32)-1;
    //     }
    // }

//    // remove track alloc events for confirmed frames
//     for (int i = 0; i < GetElementsIn(m_sg_log); i++)
//     {
//         if (m_track_log[i].audio_track != (u32)-1 && 
//             m_track_log[i].frame < this_frame)
//         {            
//             m_track_log[i].frame = (u32)-1;
//         }
//     }
}
int Audio_RemoveFromSFXLog(FGMInstance fgm_instance)
{
    // stopping an invalid SFX
    if (fgm_instance == (u32)-1)
        return 0;

    // stopping an SFX before the log is initialized
    if (!(Scene_GetCurrentMinor() == MNRKIND_3D && m_audio_log_enable))
        return 0;
        
    u32 this_frame = Gm_GetGameData()->update.engine_frames;

    int next_free_idx = -1;

    // check if we stopped the sound already
    for (int i = 0; i < GetElementsIn(m_sfx_stop_log); i++)
    {
        // remember next free slot so we dont have to iterate again
        if (next_free_idx == -1 && m_sfx_stop_log[i].frame == -1)
            next_free_idx = i;

        if (this_frame == m_sfx_stop_log[i].frame && 
            m_sfx_stop_log[i].fgm_instance == fgm_instance)
        {
            // OSReport("SFX:  skipping sfx END on frame %d. matches instance %08X from frame %d\n", this_frame, m_sfx_stop_log[i].fgm_instance, m_sfx_stop_log[i].frame);
            return 1;
        }
    }

    // log it for the future
    if (next_free_idx != -1)
    {
        SFXLog *next_free = &m_sfx_stop_log[next_free_idx];
        next_free->frame = this_frame;
        next_free->fgm_instance = fgm_instance;

        OSReport("SFX: stopped sfx with instance %08X on frame %d\n", fgm_instance, this_frame);
    }
    else
        OSReport("audio_log over!!\n");

    return 0;

//     if (!is_confirm_frame || !m_audio_log_enable)
//         return;

//     // remove ended sounds
//     for (int i = 0; i < GetElementsIn(m_sfx_log); i++)
//     {
//         // check to remove logged sound effects from confirmed frames
//         if (m_sfx_log[i].fgm_instance == fgm_instance)
//         {            
//             OSReport("SFX: stopped sfx %08X from frame %d\n", m_sfx_log[i].sfx_id, m_sfx_log[i].frame);
//             m_sfx_log[i].frame = (u32)-1;
//             break;
//         }
//     }
}
CODEPATCH_HOOKCREATE(0x8044096c, "mr 3, 28\n\t", Audio_RemoveFromSFXLog, "", 0)
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
int Audio_AssignSoundGenerator(AudioEmitterData *audio_source_data, int slot_idx)
{
    if (!(Scene_GetCurrentMinor() == MNRKIND_3D && m_audio_log_enable))
        return 0;
        
    u32 this_frame = Gm_GetGameData()->update.engine_frames;

    // check if we allocated this sg already
    for (int i = 0; i < GetElementsIn(m_sg_log); i++)
    {
        if (this_frame == m_sg_log[i].frame && 
            m_sg_log[i].audio_source_data == audio_source_data)
        {
            OSReport("SFX: giving old sg %d assignment to audio_source_data %p on frame %d\n", m_sg_log[i].sg, audio_source_data, this_frame);
            
            // store sg to emitter
            u8 sg = m_sg_log[i].sg;
            if (slot_idx == 0)
                audio_source_data->sg = sg;
            else
                audio_source_data->x40 = sg;

            // mark sg as used
            audio_3d_data->sg_status[sg] = 1;

            // if the sg was stolen from an existing emitter, null it
            if (m_sg_log[i].stolen_from_data)
            {
                if (slot_idx == 0)
                    m_sg_log[i].stolen_from_data->sg = -1;
                else
                    m_sg_log[i].stolen_from_data->x40 = -1;
            }

            // return sg
            return sg;
        }
    }

    // allocate it normally
    return 0;

}
CODEPATCH_HOOKCONDITIONALCREATE(0x8005d720, "mr 3, 29\n\t"
                                            "mr 4, 30\n\t", Audio_AssignSoundGenerator,
                                            "", 0, 0x8005d84c)
void Audio_AddToSoundGeneratorLog(AudioEmitterData *audio_source_data, u8 sg, u8 slot_idx, AudioEmitterData *stolen_from_data)
{
    if (!(Scene_GetCurrentMinor() == MNRKIND_3D && m_audio_log_enable))
        return;

    u32 this_frame = Gm_GetGameData()->update.engine_frames;

    // log sg assignment
    for (int i = 0; i < GetElementsIn(m_sg_log); i++)
    {
        if (m_sg_log[i].frame == -1)
        {
            m_sg_log[i].frame = this_frame;
            m_sg_log[i].audio_source_data = audio_source_data;
            m_sg_log[i].stolen_from_data = stolen_from_data;
            m_sg_log[i].sg = sg;
            m_sg_log[i].slot_idx = slot_idx;

            OSReport("SFX: logging sg %d assignment to audio_source_data %p on frame %d\n", sg, audio_source_data, this_frame);
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
CODEPATCH_HOOKCREATE(0x8005d6f8, "li 27, 0\n\t" "b 0x8\n\t", 0, "", 0)  // init audio_source_data in use variable

// Audio Source
void Audio_AddToSourceLog(AudioEmitter audio_emitter)
{
    if (!m_audio_log_enable)
        return;

    u32 this_frame = Gm_GetGameData()->update.engine_frames;

    for (int i = 0; i < GetElementsIn(m_source_log); i++)
    {
        if (m_source_log[i].frame == (u32)-1)
        {
            OSReport("SOURCE: adding source %08X alloc'd on frame %d\n", audio_emitter, this_frame);

            m_source_log[i].frame = this_frame;
            m_source_log[i].audio_emitter = audio_emitter;
            return;
        }
    }

    OSReport("source log over!!\n");
    return;
}
CODEPATCH_HOOKCREATE(0x8005db08, "mr 3, 29\n\t", Audio_AddToSourceLog, "mr 3, 29\n\t", 0)
int Audio_RemoveFromSourceLog(AudioEmitter audio_emitter)
{
    if (m_audio_log_enable)
    {
        for (int i = 0; i < GetElementsIn(m_source_log); i++)
        {
            if (m_source_log[i].frame != (u32)-1 &&
                m_source_log[i].audio_emitter == audio_emitter)
            {
                m_source_log[i].frame = (u32)-1;
                break;
            }
        }
    }

    return audio_emitter;
}
CODEPATCH_HOOKCREATE(0x8005e08c, "stwu	1, -0x0010 (1)\n\t"
                                "mflr 0\n\t"
                                "stw 0, 0x14(1)\n\t", 
                                Audio_RemoveFromSourceLog, 
                                "lwz    0, 0x0014 (1)\n\t" 
                                "mtlr	0\n\t" 
                                "addi	1, 1, 16\n\t", 0)

// Audio Track
void Audio_AddToTrackLog(int audio_track)
{
    if (!m_audio_log_enable)
        return;

    audio_track += LBAUDIO_TRACK_AUTO_START;

    u32 this_frame = Gm_GetGameData()->update.engine_frames;

    for (int i = 0; i < GetElementsIn(m_track_log); i++)
    {
        if (m_track_log[i].frame == (u32)-1)
        {            
            OSReport("TRACK: adding track %08X alloc'd on frame %d\n", audio_track, this_frame);

            m_track_log[i].frame = this_frame;
            m_track_log[i].audio_track = audio_track;
            return;
        }
    }

    OSReport("track log over!!\n");
}
CODEPATCH_HOOKCREATE(0x8005cf30, "mr 3, 18\n\t", Audio_AddToTrackLog, "", 0)
void Audio_RemoveFromTrackLog(int audio_track)
{
    if (!m_audio_log_enable)
        return;

    for (int i = 0; i < GetElementsIn(m_track_log); i++)
    {
        if (m_track_log[i].frame != (u32)-1 && 
            m_track_log[i].audio_track == audio_track)
        {
            m_track_log[i].frame = (u32)-1;
            break;
        }
    }

    return;
}
CODEPATCH_HOOKCREATE(0x8005d6c8, "mr 3, 31\n\t", Audio_RemoveFromTrackLog, "", 0)

void Audio_UpdateLog()
{
    u32 this_frame = Gm_GetGameData()->update.engine_frames;

    if (!is_confirm_frame)
        return;

    if (0)
    {
        // free tracks alloc'd on future frames
        for (int i = 0; i < GetElementsIn(m_track_log); i++)
        {
            // check to free audio tracks alloc'd on future frames
            if (m_track_log[i].frame != (u32)-1 && 
                m_track_log[i].frame >= this_frame)
            {            
                OSReport("TRACK: freeing track %08X alloc'd on future frame %d\n", m_track_log[i].audio_track, m_track_log[i].frame);
                AudioTrack_Free(m_track_log[i].audio_track);
            }
        }

        // free sources alloc'd on future frames
        for (int i = 0; i < GetElementsIn(m_source_log); i++)
        {
            // check to free audio sources alloc'd on future frames
            if (m_source_log[i].frame != (u32)-1 && 
                m_source_log[i].frame >= this_frame)
            {            
                OSReport("SOURCE: freeing emitter %08X alloc'd on future frame %d\n", m_source_log[i].audio_emitter, m_source_log[i].frame);
                AudioEmitter_Free(m_source_log[i].audio_emitter);
            }
        }
    }
    return;
}

FGMInstance SFXLog_OnSFXPlay(int sfx_id, int volume, int pan, int r6, int r7, u8 r8, u8 r9, AudioEmitter audio_emitter, int sg)
{
    FGMInstance (*_SFX_Play)(int sfx_id, int volume, int pan, int r6, int r7, u8 r8, u8 r9, AudioEmitter audio_emitter, int sg) = (void *)0x80442674;

    if (!(Scene_GetCurrentMinor() == MNRKIND_3D && m_audio_log_enable))
        return _SFX_Play(sfx_id, volume, pan, r6, r7, r8, r9, audio_emitter, sg);
        
    u32 this_frame = Gm_GetGameData()->update.engine_frames;

    int next_free_idx = -1;
    AudioEmitterData *this_source_data = &audio_3d_data->sources[audio_emitter];

    // check if we played the sound already
    for (int i = 0; i < GetElementsIn(m_sfx_start_log); i++)
    {
        // remember next free slot so we dont have to iterate again
        if (next_free_idx == -1 && m_sfx_start_log[i].frame == -1)
            next_free_idx = i;

        if (this_frame == m_sfx_start_log[i].frame && 
            m_sfx_start_log[i].sfx_id == sfx_id)
        {
            AudioEmitterData *that_source_data = &audio_3d_data->sources[m_sfx_start_log[i].audio_emitter];
            
            // audio track data is not backed up in the savestate, so a rolled back item sound effect 
            // will acquire a new track.
            if (this_source_data->kind == that_source_data->kind)
            {
                //OSReport(" skipping sfx PLAY %08X on frame %d. matches instance %08X from frame %d\n", sfx_id, this_frame, m_sfx_log[i].fgm_instance, m_sfx_log[i].frame);
                return m_sfx_start_log[i].fgm_instance;
            }
        }
    }

    // lets play it
    int fgm_instance = _SFX_Play(sfx_id, volume, pan, r6, r7, r8, r9, audio_emitter, sg);

    // log it for the future
    if (next_free_idx != -1)
    {
        SFXLog *next_free = &m_sfx_start_log[next_free_idx];
        next_free->frame = this_frame;
        next_free->sfx_id = sfx_id;
        next_free->audio_emitter = audio_emitter;
        next_free->fgm_instance = fgm_instance;

        OSReport("SFX: played sfx %08X with instance %08X on frame %d\n", sfx_id, fgm_instance, this_frame);
    }
    else
        OSReport("audio_log over!!\n");

    return fgm_instance;
}

int BGMLog_OnPlay(char *file_name, int volume, int pan, int r6, int r7, int r8, int r9, int r10, int slot, int sp2)
{
    int (*_BGM_Play)(char *file_name, int volume, int pan, int r6, int r7, int r8, int r9, int r10, int slot, int sp2) = (void *)0x804452a0;

    if (!(Scene_GetCurrentMinor() == MNRKIND_3D && m_audio_log_enable))
        return _BGM_Play(file_name, volume, pan, r6, r7, r8, r9, r10, slot, sp2);

    u32 this_frame = Gm_GetGameData()->update.engine_frames;
    int next_free_idx = -1;

    u32 entrynum = DVDConvertPathToEntrynum(file_name);

    // check if we played the music already
    for (int i = 0; i < GetElementsIn(m_bgm_log); i++)
    {
        // remember next free slot so we dont have to iterate again
        if (next_free_idx == -1 && m_bgm_log[i].frame == -1)
            next_free_idx = i;

        if (this_frame == m_bgm_log[i].frame && 
            m_bgm_log[i].event_kind == BGMEVENT_PLAY &&
            m_bgm_log[i].slot == slot &&
            m_bgm_log[i].entrynum == entrynum)
        {
            return m_bgm_log[i].slot;
        }
    }

    // lets play it
    int result = _BGM_Play(file_name, volume, pan, r6, r7, r8, r9, r10, slot, sp2);

    // log it for the future
    if (next_free_idx != -1)
    {
        BGMLog *next_free = &m_bgm_log[next_free_idx];
        next_free->frame = this_frame;
        next_free->event_kind = BGMEVENT_PLAY;
        next_free->entrynum = entrynum;
        next_free->slot = slot;

        OSReport("BGM: played bgm %s with slot %08X on frame %d\n", file_name, slot, this_frame);
    }
    else
        OSReport("m_bgm_log over!!\n");

    return result;

}
int Audio_SpoofNoFGMInstanceMatch()
{
    return 0;
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
    CODEPATCH_REPLACEFUNC(0x80443e9c, Audio_SpoofNoFGMInstanceMatch);
    CODEPATCH_REPLACEFUNC(0x80443d8c, Audio_SpoofNoFGMInstanceMatch);

    // prevent duplicate music events
    CODEPATCH_REPLACECALL(0x804456f0, BGMLog_OnPlay);
    
    // sg assignment
    // CODEPATCH_HOOKAPPLY(0x8005d720);
    // CODEPATCH_HOOKAPPLY(0x8005d830);
    // CODEPATCH_HOOKAPPLY(0x8005d6f8);

    // audio source and track allocations
    // CODEPATCH_HOOKAPPLY(0x8005db08);
    // CODEPATCH_HOOKAPPLY(0x8005e08c);
    // CODEPATCH_HOOKAPPLY(0x8005cf30);
    // CODEPATCH_HOOKAPPLY(0x8005d6c8);

    // temp disable music
    // CODEPATCH_REPLACEINSTRUCTION(0x804456c0, 0x4e800020);
}