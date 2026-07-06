#ifndef STARPOLE_NETPLAY
#define STARPOLE_NETPLAY

#define DOLPHIN_DEBUG 0
#define NETPLAY_TAGMAX 10

typedef struct PlayerTagViewData
{
    int ply;
    Text *t[5];
} PlayerTagViewData;

typedef struct ViewportTagViewData
{
    int ply;
    Text *t;
} ViewportTagViewData;

typedef struct ViewportTagParam
{
    Vec2 offset;
    float scale;
    float width;
} ViewportTagParam;

void Dolphin_Init();
int Dolphin_ReqData(StarpoleDataDolphin *data);
int Dolphin_IsNetplay();

void Netplay_Init();
void Netplay_OverridePlayerView(StarpoleDataDolphin *dolphin_data);
void Netplay_CreatePlayerTags(StarpoleDataDolphin *dolphin_data);
void Netplay_DestroyPlayerTagViewGObj(PlayerTagViewData *gp);

JOBJSet ***Netplay_GetPlyNumSet();
void Netplay_AdjustPlyNum(JOBJ *j);
void Netplay_LoadAltPlyNumFile();

void Netplay_PlayerTagGX(GOBJ *g, int pass);
void Netplay_CreateViewportTag(StarpoleDataDolphin *data);
void Netplay_DestroyViewportTagViewGObj(ViewportTagViewData *gp);
void Netplay_ViewportTagGX(GOBJ *g, int pass);

void Netplay_CreateInputDisplay();
void Netplay_UpdateInputDisplay(GOBJ *g);
void Netplay_RenderInputDisplay(GOBJ *g, int pass);

GXColor *UI_GetTextOutlineColor(GXColor *color);
void Text_AddOutline(Text *t, GXColor *color, char *s);

void PadAlarm_Remove();

void StressTest_Create();

void Dolphin_OnSceneChange();
void Netplay_On3DLoadStart();
void Netplay_On3DLoadEnd();

#endif