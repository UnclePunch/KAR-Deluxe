#include "datatypes.h"
#include "os.h"
#include "inline.h"
#include "scene.h"
#include "hoshi/func.h"

#include "playback.h"
#include "replay.h"
#include "starpole.h"

extern StarpoleBuffer *starpole_buf;
OSThread playback_thread;

MajorSceneDesc playback_major_desc = {
    .major_id = -1,
    .next_major_id = MJRKIND_MENU,
    .initial_minor_id = MNRKIND_3D,
    .cb_Enter = PlaybackMajor_Enter,
    .cb_ExitMinor = PlaybackMajor_ExitMinor,
};
MinorSceneDesc playback_minor_desc = {
    .idx = 0,
    .x1 = -1,
    .x2 = 1,
    .cb_Load = PlaybackMinor_Load,
    .cb_Exit = (void *)0x80014d5c,
    .cb_ThinkPreGObjProc = (void *)0x8001528c,
    .cb_ThinkPostGObjProc = 0,
    .cb_ThinkPostGObjProc2 = (void *)0x800152ac,
    .cb_ThinkPreRender = 0,
    .cb_ThinkPostRender = 0,
    .preload_kind = 4,
};

// EXI Function
int Playback_CheckForMatch()
{
    // request data
    if (Starpole_Imm(STARPOLE_CMD_CHECKPLAYBACK, 0) > 0)
        return 1;

    return 0;
}

// Thread
void Playback_Listen()
{
    if (Playback_CheckForMatch())
    {
        SFX_Play(FGMMENU_CS_KETTEI);
        BGM_Stop();
        Scene_SetNextMajor(playback_major_desc.major_id);
        Scene_ExitMinor();
        Scene_ExitMajor();
    }
}

// Scene
int Playback_IsMajor()
{
    return (Scene_GetCurrentMajor() == playback_major_desc.major_id);
}
void PlaybackMajor_Enter()
{
    if (Replay_ReqMatch())
        Preload_SetGrKind(starpole_buf->match.stage_kind);
}
void PlaybackMajor_ExitMinor()
{
    int dir = Scene_GetDirection();
    if (dir == PAD_BUTTON_B)
    {
        Scene_SetNextMajor(MJRKIND_MENU);
        Scene_ExitMajor();
    }
}
void PlaybackMinor_Load()
{
    void (*MinorLoad_3D)() = (void *)0x8001442c;
    MinorLoad_3D();
}

void Playback_Init()
{
    if (!Starpole_IsPresent())
        return;

    Hoshi_InstallMinorScene(&playback_minor_desc);

    playback_major_desc.initial_minor_id = playback_minor_desc.idx;
    Hoshi_InstallMajorScene(&playback_major_desc);
}