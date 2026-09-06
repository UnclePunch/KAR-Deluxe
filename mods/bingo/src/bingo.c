
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

#include "code_patch/code_patch.h"

#include <string.h>

#include "bingo.h"

JOBJSet *card_set;
JOBJSet *icon_set;

// Bingo UI
int is_bingo_mode = 1;
GOBJ *bingo_card_gobj[5];
int text_canvas_idx = 0;
GOBJ *BingoCardView_Create()
{
    Vec3 text_pos;
    Text *t;

    // create gobj and model
    GOBJ *b = GOBJ_EZCreator(0, GAMEPLINK_CAMHUD, 0,
                            sizeof(BingoUIData), BingoCardView_Destroy,
                            HSD_OBJKIND_JOBJ, card_set->jobj, 
                            GOBJ_Anim, 0, 
                            JObj_GX, GAMEGX_HUD, 3);

    BingoUIData *bp = b->userdata;

    // create text
    {
        int x_pos;

        // details 
        JObj_GetChildPosition(b->hsd_object, BINGO_UI_DESCRIPTION_TEXT_JOINT, &text_pos);
        t = Text_CreateText(BINGO_SIS_INDEX, text_canvas_idx);
        // t->viewport_color = (GXColor){255, 0, 0, 128};
        t->kerning = 1;
        t->align = 0;
        t->viewport_scale = (Vec2){0.045, 0.055};
        t->use_aspect = 1;
        t->aspect = (Vec2){370, 64};
        t->trans.X = text_pos.X;
        t->trans.Y = -text_pos.Y;
        x_pos = 0;
        Text_AddSubtext(t, x_pos, 0, "Lorem ipsum dolor sit amet");
        Text_AddSubtext(t, x_pos, 30, "sed do eiusmod tempor incididunt.");
        bp->text.details = t;

        // create scoreboard
        JObj_GetChildPosition(b->hsd_object, BINGO_UI_SCOREBOARD_TEXT_JOINT, &text_pos);

        // labels
        t = Text_CreateText(BINGO_SIS_INDEX, text_canvas_idx);
        // t->viewport_color = (GXColor){255, 0, 0, 128};
        t->kerning = 1;
        t->align = 0;
        t->viewport_scale = (Vec2){0.045 * 0.8, 0.055 * 0.8};
        t->use_aspect = 1;
        t->aspect = (Vec2){250, 160};
        t->trans.X = text_pos.X;
        t->trans.Y = -text_pos.Y;
        Text_AddSubtext(t, 0, 0, "Game Info");
        Text_AddSubtext(t, 330, 0, "Leaderboard");
        bp->text.scoreboard.label = t;

        // game info
        t = Text_CreateText(BINGO_SIS_INDEX, text_canvas_idx);
        // t->viewport_color = (GXColor){255, 0, 0, 128};
        t->kerning = 1;
        t->align = 0;
        t->viewport_scale = (Vec2){0.045 * 0.8, 0.055 * 0.8};
        t->use_aspect = 1;
        t->aspect = (Vec2){250, 160};
        t->trans.X = text_pos.X;
        t->trans.Y = -text_pos.Y;
        x_pos = 0;
        Text_AddSubtext(t, x_pos, 50, "Mode: " "\x0C\xFF\xFF\x50" "Standard");
        Text_AddSubtext(t, x_pos, 80, "Goal: " "\x0C\xFF\xFF\x50" "%d items", 100);
        Text_AddSubtext(t, x_pos, 110, "Time Limit: Off");
        Text_AddSubtext(t, x_pos, 140, "Team: " "\x0C\x26\x26\xD9" "Blue");
        bp->text.scoreboard.game_info = t;

        // leaderboard numbers
        t = Text_CreateText(BINGO_SIS_INDEX, text_canvas_idx);
        // t->viewport_color = (GXColor){255, 0, 0, 128};
        t->kerning = 1;
        t->align = 1;
        t->viewport_scale = (Vec2){0.045 * 0.8, 0.055 * 0.8};
        t->use_aspect = 1;
        t->aspect = (Vec2){250, 160};
        t->trans.X = text_pos.X;
        t->trans.Y = -text_pos.Y;
        x_pos = 340;
        Text_AddSubtext(t, x_pos, 50, "1.");
        Text_AddSubtext(t, x_pos, 80, "2.");
        Text_AddSubtext(t, x_pos, 110, "3.");
        Text_AddSubtext(t, x_pos, 140, "4.");
        bp->text.scoreboard.leaderboard_nums = t;
        
        // teams
        t = Text_CreateText(BINGO_SIS_INDEX, text_canvas_idx);
        // t->viewport_color = (GXColor){255, 0, 0, 128};
        t->kerning = 1;
        t->align = 0;
        t->viewport_scale = (Vec2){0.045 * 0.8, 0.055 * 0.8};
        t->use_aspect = 1;
        t->aspect = (Vec2){100, 160};
        t->trans.X = text_pos.X;
        t->trans.Y = -text_pos.Y;
        x_pos = 375;
        Text_AddSubtext(t, x_pos, 50, "\x0C\x26\x26\xD9" "Blue:" );
        Text_AddSubtext(t, x_pos, 80, "\x0C\xFF\xB2\xCC" "Pink:");
        Text_AddSubtext(t, x_pos, 110, "\x0C\x40\xFF\x40" "Green:");
        Text_AddSubtext(t, x_pos, 140, "\x0C\xFF\x40\x40" "Red:");
        bp->text.scoreboard.leaderboard_teams = t;

        // team scores
        t = Text_CreateText(BINGO_SIS_INDEX, text_canvas_idx);
        // t->viewport_color = (GXColor){255, 0, 0, 128};
        t->kerning = 1;
        t->align = 0;
        t->viewport_scale = (Vec2){0.045 * 0.8, 0.055 * 0.8};
        t->use_aspect = 1;
        t->aspect = (Vec2){140, 160};
        t->trans.X = text_pos.X;
        t->trans.Y = -text_pos.Y;
        x_pos = 490;
        Text_AddSubtext(t, x_pos, 50, "12 items");
        Text_AddSubtext(t, x_pos, 80, "0 items");
        Text_AddSubtext(t, x_pos, 110, "0 items");
        Text_AddSubtext(t, x_pos, 140, "0 items");
        bp->text.scoreboard.leaderboard_scores = t;

    }

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

            // display objective icon
            int icon_frame_idx = HSD_Randi(51);
            JOBJ *objective_icon_j = JObj_GetIndex(icon_j, 2);
            JObj_SetFrameAndRate(objective_icon_j, icon_frame_idx, 0);

            JOBJ *fill_icon_j = JObj_GetIndex(icon_j, 1);
            int fill_frame = (x == 2 && y == 2) ? 1 : 0;
            JObj_SetFrameAndRate(fill_icon_j, fill_frame, 0);

            // display objective number
            {
                int objective_num = HSD_Randi(18) + 1;
                int disabled_num_joint_idx, enabled_num_joint_idx;
                if (objective_num > 9) {
                    enabled_num_joint_idx = 5;
                    disabled_num_joint_idx = 3;
                }
                else {
                    enabled_num_joint_idx = 3;
                    disabled_num_joint_idx = 5;
                }

                // hide unused digits first
                JObj_SetFlagsAll(JObj_GetIndex(icon_j, disabled_num_joint_idx), JOBJ_HIDDEN);

                // display digits
                JOBJ *digit_j = JObj_GetIndex(icon_j, enabled_num_joint_idx);
                int num = objective_num;
                int loop_num = 0;
                while (num > 0)
                {
                    int digit = num % 10;
                    
                    JOBJ *num_j = JObj_GetIndex(digit_j, 1 + loop_num);
                    JObj_SetFrameAndRate(num_j, digit, 0);
        
                    num = num / 10;
                    loop_num++;
                }

            }

            // position on grid
            icon_j->trans.X = -(width / 2) + (width * (x / (grid_size - 1)));
            icon_j->trans.Y = -(width / 2) + (width * (y / (grid_size - 1)));
            JObj_SetMtxDirtySub(icon_j);
        }
    }
    JObj_SetMtxDirtySub(card_j);

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

    if (rp->input.held & PAD_BUTTON_Y)
    {

    }
    
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
void BingoCardView_Destroy(BingoUIData *bp)
{
    Text_Destroy(bp->text.details);
    Text_Destroy(bp->text.scoreboard.game_info);
    Text_Destroy(bp->text.scoreboard.label);
    Text_Destroy(bp->text.scoreboard.leaderboard_nums);
    Text_Destroy(bp->text.scoreboard.leaderboard_teams);
    Text_Destroy(bp->text.scoreboard.leaderboard_scores);

    HSD_Free(bp);
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
        if (Ply_GetPKind(i) == PKIND_NONE || Gm_Get3dData()->plyview_lookup[i] == -1)
            continue;

        GOBJ *r = Ply_GetRiderGObj(i);
        GObj_AddProc(r, BingoCardView_Think, 3);
    }
}

void Bingo_OnDPadToStick(RiderData *rd)
{
    int button_mask = ~(PAD_BUTTON_DPAD_DOWN | PAD_BUTTON_DPAD_UP | PAD_BUTTON_DPAD_LEFT | PAD_BUTTON_DPAD_RIGHT);
 
    // if a bingo card view is being shown, remove all dpad inputs
    if (bingo_card_gobj[rd->ply])
    {
        rd->input.held &= button_mask;
        rd->input.down &= button_mask;
        rd->input.x3dc &= button_mask;
    }
}
CODEPATCH_HOOKCREATE(0x8018f0bc, "mr 3, 31\n\t", Bingo_OnDPadToStick, "", 0)

void Bingo_Init()
{
    // apply our code patches
    CODEPATCH_HOOKAPPLY(0x8018f0bc);

    // Hoshi_AddPreloadGameFile(BINGO_ASSET_FILENAME, PRELOADHEAPKIND_ALLM);
}
void Bingo_On3DLoadStart()
{
    // init gobj pointers
    for (int i = 0; i < GetElementsIn(bingo_card_gobj); i++)
        bingo_card_gobj[i] = 0;

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
