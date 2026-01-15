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
void Replay_OnBoot();
void Replay_OnSceneChange();