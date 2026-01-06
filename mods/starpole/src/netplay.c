#include <stddef.h>

#include "text.h"
#include "os.h"
#include "hsd.h"
#include "game.h"
#include "exi.h"
#include "scene.h"
#include "inline.h"
#include "scene.h"

#include "hoshi/func.h"
#include "hoshi/screen_cam.h"

#include "starpole.h"
#include "replay.h"
#include "code_patch/code_patch.h"
#include "text_joint/text_joint.h"

extern StarpoleBuffer *starpole_buf;
int netplay_ply;

int Netplay_ReqPlayerIndex()
{
    return Starpole_Imm(STARPOLE_CMD_NETPLAY, 0);
}

void Netplay_DisplayPlayer()
{
    if (netplay_ply == -1)
        TextConsole_AddString(0, 0, "Netplay not detected.");
    else
        TextConsole_AddString(0, 0, "Netplay player %d", netplay_ply + 1);
}

void Netplay_Init()
{
    netplay_ply = Netplay_ReqPlayerIndex();
    OSReport("netplay player is %d\n", netplay_ply);
}

void Netplay_OverridePlayerView()
{
    bp();
    if (netplay_ply == -1)
        return;

    GameData *gd = Gm_GetGameData();

    for (int i = 0; i < GetElementsIn(gd->ply_view_desc); i++)
        gd->ply_view_desc[i].flag = PLYCAM_OFF;
    
    gd->ply_view_desc[netplay_ply].flag = PLYCAM_ON;
}