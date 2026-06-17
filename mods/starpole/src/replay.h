#define REPLAY_ENABLE (1)
#define REPLAY_DEBUG (0)
#define REPLAY_SYNCRNG (1)

typedef enum
{
    REPLAY_NONE,
    REPLAY_RECORD,
    REPLAY_PLAYBACK,
} ReplayMode;

int Replay_ReqMatch();

void Replay_On3DLoadStart();
void Replay_On3DExit();
void Replay_DisplayString();
void Replay_Init();
void Replay_OnSceneChange();

void Record_OnFrameEnd();
void Playback_OnFrameEnd();

void Hash_CreateText();
void Hash_DestroyText();

u32 Replay_HashGameState(u32 kind);