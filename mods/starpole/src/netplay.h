#ifndef STARPOLE_NETPLAY
#define STARPOLE_NETPLAY

#define NETPLAY_DEBUG 0
#define NETPLAY_TAGMAX 10

typedef struct PlayerTagViewData
{
    int ply;
    Text *t[5];
} PlayerTagViewData;

void Netplay_Init();
void Netplay_OverridePlayerView();
void Netplay_CreatePlayerTags();
void Netplay_DestroyPlayerTagViewGObj(PlayerTagViewData *gp);
void Netplay_PlayerTagGX(GOBJ *g, int pass);

#endif