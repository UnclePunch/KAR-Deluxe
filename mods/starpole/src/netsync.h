#ifndef STARPOLE_NETSYNC
#define STARPOLE_NETSYNC

#include "hsd.h"
#include "audio.h"
    
// map emitters are id'd by their emitter pointer because map all emitters use 0 for their instance
#define GET_EMITTER_INSTANCE(emitter_data) (emitter_data->kind == AUDIOEMITTER_MAP) ? (u32)emitter_data : emitter_data->instance

#define MAX_ROLLBACK_FRAMES (5)

typedef enum BGMEvent
{
    BGMEVENT_PLAY,
    BGMEVENT_STOP,
    BGMEVENT_FADE,
} BGMEvent;

typedef struct
{
    void *addr;
    u32 size;
} PreserveMemRegion;

typedef struct 
{
    int head;
    PADStatus history[10][4];
} DebugPadData;

typedef struct
{
    void *addr;
    AudioTrackOwner owner;
} AudioTrackCallerMap;

typedef struct
{
    AudioEmitterKind kind;
    u32 instance;
} EmitterID;

typedef struct
{
    u32 frame;
    int sfx_id;
    FGMInstance fgm_instance;
    u32 audio_track : 16;
    u32 sg : 8;
    u32 is_replayed : 8;
    // EmitterID emitter;
} SFXLog;

typedef struct
{
    u32 frame;
    int slot : 8;
    BGMEvent event_kind : 8;
    int entrynum : 16;
} BGMLog;

typedef struct
{
    u32 frame;
    struct
    {   
        AudioEmitter index;
        EmitterID identifier;
    } emitter;
} EmitterLog;

typedef struct
{
    u32 frame;
    AudioTrackOwner owner : 16;
    u16 audio_track : 16;
} TrackLog;

typedef struct
{
    u32 frame;
    EmitterID this_emitter;
    EmitterID stolen_from_emitter;
    u8 slot_idx;
    u8 sg;
} SoundGeneratorLog;

typedef struct
{
    int enable;
    SFXLog sfx_start[64];
    SFXLog sfx_stop[64];
    BGMLog bgm[16];
    EmitterLog emitter[64];
    TrackLog track[64];
    SoundGeneratorLog sg[64];
} AudioLog;

typedef struct
{
    u32 sim_frames;
    u32 is_resim_frame;
    u32 this_sim_idx;
} RollbackLog;

typedef struct
{
    FGMInstance instance;       // 0xc, index in the FGMInstanceData array
    PID pid;                    // 0x10, as evidenced by assert @ 80442538 and 80441274
    u32 sfx_id;                 // 0x14, what was passed into SFX_Play. internally fid
    u8 priority;                // 0x19
    u16 audio_track;            // 0x1a. is r6 of SFX_Play
    struct
    {
        VPB *addr;
        float current_vol;
        float target_vol;   
    } vpb;
    struct
    {
        AXVPB *addr;
        u16 state;
        u16 vol;
        u16 vol_l;
        u16 vol_r;
        u16 pitch;
        void *currentAddress;
    } axvpb;             
} FGMDebugLog;

void Netsync_Init();
void PadAlarm_NetplayLockstep();

void Netsync_CreateRNGText();
void Netsync_UpdateRNGText();

void Netsync_On3DLoadStart();
void Netsync_On3DExit();

void Audio_InitLog();
void Audio_UpdateSFXLog();
void Audio_UpdateLog();
void Audio_ValidateAX();
void Audio_ResetLogs(int is_clear_all);
void Audio_Debug();

#endif