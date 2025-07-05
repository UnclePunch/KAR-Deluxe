#ifndef MUSICCHANGE_H
#define MUSICCHANGE_H

#define SONGNAME_GXLINK 45
#define MUSICCHANGE_SCROLLSPEED (0.05)

typedef enum MusicChangeScrollState
{
    MUSICCHANGE_SCROLLSTATE_NONE,
    MUSICCHANGE_SCROLLSTATE_STARTWAIT,
    MUSICCHANGE_SCROLLSTATE_MOVE,
    MUSICCHANGE_SCROLLSTATE_STOPWAIT,
} MusicChangeScrollState;

typedef struct NowPlayingAssets
{
    JOBJSet *np_1;
    JOBJSet *np_air_2;
    JOBJSet *np_ct_2;
    JOBJSet *np_air_4;
    JOBJSet *np_ct_4;
} NowPlayingAssets;

typedef struct MusicChangeTextParams
{
    Vec2 pos;
    Vec2 scale;
    Vec2 aspect;
    u16 scissor_left;
    u16 scissor_right;
    float textbox_width;
} MusicChangeTextParams;

typedef struct MusicChangeData
{
    int state;
    int timer;
    Vec2 offset;
    MusicChangeTextParams *param;
} MusicChangeData;

void MusicChange_Init();
void MusicChange_On3DLoad();
GOBJ *MusicChange_Create();
void MusicChange_Destroy();
void MusicChange_Think(GOBJ *g);
void MusicChange_TextCObj(GOBJ *g);
JOBJSet *MusicChange_GetJOBJSet();
void MusicChange_ScaleStats(int ply, float scale, Vec2 offsets);
void MusicChange_UpdateSongName(MusicChangeData *gp);
float MusicChange_GetScrollAmount(Text *t, float textbox_width);
MusicChangeTextParams *MusicChange_GetTextParam();
float Text_GetWidth(Text *t);
#endif