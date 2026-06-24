#ifndef STARPOLE_NETPLAY
#define STARPOLE_NETPLAY

#define DOLPHIN_DEBUG 0
#define NETPLAY_TAGMAX 10

typedef struct PlayerTagViewData
{
    int ply;
    Text *t[5];
} PlayerTagViewData;

typedef struct SelfTagViewData
{
    int ply;
    Text *t;
} SelfTagViewData;

typedef struct SelfTagParam
{
    Vec2 offset;
    float scale;
    float width;
} SelfTagParam;

void Dolphin_Init();
int Dolphin_ReqData(StarpoleDataDolphin *data);
int Dolphin_IsNetplay();

void Netplay_Init();
void Netplay_OverridePlayerView(StarpoleDataDolphin *dolphin_data);
void Netplay_CreatePlayerTags(StarpoleDataDolphin *dolphin_data);
void Netplay_DestroyPlayerTagViewGObj(PlayerTagViewData *gp);
void Netplay_CreatePlyNum(int ply, JOBJDesc *jobjdesc, void *gx_cb);

void Netplay_PlayerTagGX(GOBJ *g, int pass);
void Netplay_CreateSelfTag(StarpoleDataDolphin *data);
void Netplay_DestroySelfTagViewGObj(SelfTagViewData *gp);
void Netplay_SelfTagGX(GOBJ *g, int pass);

GXColor *UI_GetTextOutlineColor(GXColor *color);
void Text_AddOutline(Text *t, GXColor *color, char *s);

void PadAlarm_Remove();

void StressTest_Create();

void Netplay_On3DLoadStart();
void Netplay_On3DLoadEnd();

#endif