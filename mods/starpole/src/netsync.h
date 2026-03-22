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
    u32 is_replayed : 16;
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

#endif