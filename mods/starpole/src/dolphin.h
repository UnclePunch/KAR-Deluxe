#ifndef STARPOLE_NETPLAY
#define STARPOLE_NETPLAY

#define DOLPHIN_DEBUG 0
#define NETPLAY_TAGMAX 10

typedef struct PlayerTagViewData
{
    int ply;
    Text *t[5];
} PlayerTagViewData;

void Dolphin_Init();
int Dolphin_ReqData(StarpoleDataDolphin *data);
int Dolphin_IsNetplay();

void Netplay_Init();
void Netplay_OverridePlayerView(StarpoleDataDolphin *dolphin_data);
void Netplay_CreatePlayerTags(StarpoleDataDolphin *dolphin_data);
void Netplay_DestroyPlayerTagViewGObj(PlayerTagViewData *gp);
void Netplay_PlayerTagGX(GOBJ *g, int pass);

void PadAlarm_Remove();

void StressTest_Create();

void Netplay_On3DLoadStart();
void Netplay_On3DLoadEnd();

#endif