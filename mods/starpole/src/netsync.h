#ifndef STARPOLE_NETSYNC
#define STARPOLE_NETSYNC

#include "hsd.h"
#include "audio.h"

#define MAX_ROLLBACK_FRAMES (5)

typedef enum BGMEvent
{
    BGMEVENT_PLAY,
    BGMEVENT_STOP,
    BGMEVENT_FADE,
} BGMEvent;

typedef struct
{
    u32 frame;
    int sfx_id;
    FGMInstance fgm_instance : 16;
    AudioSource audio_source : 16;
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
    int audio_source;
} SourceLog;

typedef struct
{
    u32 frame;
    int audio_track;
} TrackLog;

typedef struct
{
    u32 frame;
    AudioSourceData *audio_source_data;
    AudioSourceData *stolen_from_data;
    u8 slot_idx;
    u8 sg;
} SoundGeneratorLog;

void Netsync_Init();
void PadAlarm_NetplayLockstep();

void Netsync_CreateRNGText();
void Netsync_UpdateRNGText();

void Audio_On3DLoadStart();
void Audio_InitLog();
void Audio_UpdateSFXLog();
void Audio_UpdateLog();

#endif