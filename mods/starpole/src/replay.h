typedef enum
{
    REPLAY_NONE,
    REPLAY_RECORD,
    REPLAY_PLAYBACK,
} ReplayMode;

void Replay_On3DLoadStart();
void Replay_On3DExit();
void Replay_DisplayString();
void Replay_OnBoot();