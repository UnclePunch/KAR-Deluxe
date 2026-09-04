
#include "text.h"
#include "os.h"
#include "hsd.h"
#include "preload.h"
#include "game.h"
#include "scene.h"
#include "inline.h"
#include "audio.h"
#include "scene.h"

#include "hoshi/func.h"

#include <string.h>

#include "bingo.h"

void Bingo_Init()
{
    // apply our code patches
    ;

    Hoshi_AddPreloadGameFile(BINGO_ASSET_FILENAME, PRELOADHEAPKIND_ALLM);
}

JOBJSet *card_set;

void Bingo_On3DLoadStart()
{
    OSReport("Load bingo card assets\n");

    // get our file
    HSD_Archive *archive;
    Gm_LoadGameFile(&archive, BINGO_ASSET_FILENAME);
    card_set = ((JOBJSet**)Archive_GetPublicAddress(archive, "IfBingoCard_scene_models"))[0];
}

GOBJ *bingocard_gobj = 0;
void Bingo_On3DPause(int pause_ply)
{
    OSReport("Created bingo card\n");
    bingocard_gobj = GOBJ_EZCreator(0, GAMEPLINK_CAMHUD, 0,
                    0, 0,
                    HSD_OBJKIND_JOBJ, card_set->jobj, 
                    GOBJ_Anim, 0, 
                    JObj_GX, GAMEGX_HUD, 3);
}

void Bingo_On3DUnpause(int pause_ply)
{
    OSReport("Destroyed bingo card\n");
    GObj_Destroy(bingocard_gobj);
    bingocard_gobj = 0;
}