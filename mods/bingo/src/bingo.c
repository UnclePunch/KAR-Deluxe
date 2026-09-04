
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

JOBJSet *card_set;
JOBJSet *icon_set;

// Bingo UI
int is_bingo_mode = 1;
GOBJ *bingo_card_gobj[5];
int text_canvas_idx = 0;
Text *temp_text = 0;
GOBJ *BingoCardView_Create()
{
    // create gobj and model
    GOBJ *b = GOBJ_EZCreator(0, GAMEPLINK_CAMHUD, 0,
                            4, BingoCardView_Destroy,
                            HSD_OBJKIND_JOBJ, card_set->jobj, 
                            GOBJ_Anim, 0, 
                            JObj_GX, GAMEGX_HUD, 3);

    // create text
    Vec3 text_pos;
    JObj_GetChildPosition(b->hsd_object, 4, &text_pos);
    Text *t = Text_CreateText(BINGO_SIS_INDEX, text_canvas_idx);
    // t->viewport_color = (GXColor){255, 0, 0, 128};
    t->kerning = 1;
    t->align = 0;
    t->viewport_scale = (Vec2){0.045, 0.055};
    t->use_aspect = 1;
    t->aspect = (Vec2){370, 64};
    t->trans.X = text_pos.X;
    t->trans.Y = -text_pos.Y;
    Text_AddSubtext(t, 0, 0, "Lorem ipsum dolor sit amet");
    Text_AddSubtext(t, 0, 30, "sed do eiusmod tempor incididunt.");
    temp_text = t;

    // create bingo icons
    JOBJ *card_j = JObj_GetIndex(b->hsd_object, 1);
    float grid_size = 5;
    float width = 17.5;
    for (int x = 0; x < grid_size; x++)
    {
        for (int y = 0; y < grid_size; y++)
        {
            JOBJ *icon_j = JObj_LoadJoint(icon_set->jobj);
            JObj_AddSetAnim(icon_j, 0, icon_set, 6, 0);
            JObj_AddNext(card_j, icon_j);

            bp();
            JOBJ *objective_j = JObj_GetIndex(icon_j, 2);
            JObj_SetFrameAndRate(objective_j, HSD_Randi(51), 0);
            JOBJ *num_j = JObj_GetIndex(icon_j, 3);
            JObj_SetFrameAndRate(num_j, HSD_Randi(9) + 1, 0);

            icon_j->trans.X = -(width / 2) + (width * (x / (grid_size - 1)));
            icon_j->trans.Y = -(width / 2) + (width * (y / (grid_size - 1)));
            JObj_SetMtxDirtySub(icon_j);
        }
    }
    // JObj_SetMtxDirtySub(card_j);

    return b;
}
void BingoCardView_Think(GOBJ *r)
{
    // not during the intro
    if (Gm_GetIntroState() != GMINTRO_END)
        return;

    RiderData *rp = r->userdata;
    Game3dData *g3d = Gm_Get3dData();

    GOBJ *b = bingo_card_gobj[rp->ply];

    if (!b && rp->input.held & PAD_BUTTON_Y)
    {
        bingo_card_gobj[rp->ply] = BingoCardView_Create();        
        Gm_HideHUD();
    }
    else if (b)
    {
        // destroy if released button
        if (!(rp->input.held & PAD_BUTTON_Y))
        {
            GObj_Destroy(b);
            bingo_card_gobj[rp->ply] = 0;

            Gm_ShowHUD();
        }
        // update UI
        else
        {
            // 
        }
    }
}
void BingoCardView_Destroy(void *data)
{
    Text_Destroy(temp_text);
}
void Bingo_DestroyOnPause()
{
    // destroys any existing bingo UI when pausing
    if (!is_bingo_mode || !(Gm_IsInCity() && Gm_GetCityMode() == CITYMODE_TRIAL))
        return;

    Game3dData *g3d = Gm_Get3dData();

    // destroy any existing stat screens
    for (int ply = 0; ply < GetElementsIn(bingo_card_gobj); ply++)
    {
        if (Ply_GetPKind(ply) == PKIND_NONE)
            continue;

        if (bingo_card_gobj[ply])
        {
            GObj_Destroy(bingo_card_gobj[ply]);
            bingo_card_gobj[ply] = 0;
        }
    }
}
void Bingo_CreateViewCheck()
{
    // add proc to rider that checks to create the bingo UI
    if (!is_bingo_mode || Gm_GetCityMode() != CITYMODE_TRIAL)
        return;

    // create text canvas
    text_canvas_idx = Text_CreateCanvas(BINGO_SIS_INDEX, -1, 28, GAMEPLINK_CAMHUD, 0, GAMEGX_HUD, 3, 0);

    // add new proc to each rider with a viewport
    for (int i = 0; i < GetElementsIn(bingo_card_gobj); i++)
    {
        bingo_card_gobj[i] = 0;

        if (Ply_GetPKind(i) == PKIND_NONE || Gm_Get3dData()->plyview_lookup[i] == -1)
            continue;

        GOBJ *r = Ply_GetRiderGObj(i);
        GObj_AddProc(r, BingoCardView_Think, 3);
    }
}

void Bingo_Init()
{
    // apply our code patches
    ;

    // Hoshi_AddPreloadGameFile(BINGO_ASSET_FILENAME, PRELOADHEAPKIND_ALLM);
}
void Bingo_On3DLoadStart()
{
    OSReport("Load bingo card assets\n");

    // get our file
    HSD_Archive *archive;
    Gm_LoadGameFile(&archive, BINGO_ASSET_FILENAME);
    card_set = ((JOBJSet**)Archive_GetPublicAddress(archive, "IfBingoCard_scene_models"))[0];
    icon_set = ((JOBJSet**)Archive_GetPublicAddress(archive, "IfBingoIcon_scene_models"))[0];
}
void Bingo_On3DLoadEnd()
{
    Bingo_CreateViewCheck();
}
void Bingo_On3DPause(int pause_ply)
{
    Bingo_DestroyOnPause();
}
void Bingo_On3DUnpause(int pause_ply)
{
}
