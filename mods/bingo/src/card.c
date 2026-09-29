#include "obj.h"
#include "game.h"

#include "../../wide/src/wide_export.h"

#include "code_patch/code_patch.h"

#include "bingo.h"
#include "card.h"
#include "tracker.h"

extern int is_bingo_mode;
extern WideExport *g_wide_export;
extern BingoCard g_bingo_card;
extern GOBJ *bingo_tracker_gobj[5];

GOBJ *bingo_card_gobj[5];
BingoCursor bingo_cursor[5];

JOBJSet *card_set;
JOBJSet *icon_set;
JOBJSet *notif_set;

int Bingo_OnDPadToStick(RiderData *rd)
{
    // int button_mask = ~(PAD_BUTTON_DPAD_DOWN | PAD_BUTTON_DPAD_UP | PAD_BUTTON_DPAD_LEFT | PAD_BUTTON_DPAD_RIGHT);

    if (bingo_card_gobj[rd->ply])
        return 1;

    return 0;

}
CODEPATCH_HOOKCONDITIONALCREATE(0x8018f0bc, "mr 3, 31\n\t", Bingo_OnDPadToStick, "", 0, 0x8018f0cc)

// Bingo UI
int bingo_text_canvas_idx = 0;
void BingoUI_OnBoot()
{
    CODEPATCH_HOOKAPPLY(0x8018f0bc);
}
void BingoUI_On3DLoadStart()
{
    // init bingo data
    for (int i = 0; i < GetElementsIn(bingo_card_gobj); i++)
    {
        bingo_card_gobj[i] = 0;
        bingo_cursor[i].x = (BINGO_UI_GRID_SIZE - 1) / 2;
        bingo_cursor[i].y = (BINGO_UI_GRID_SIZE - 1) / 2;
    }

    // get our file
    HSD_Archive *archive;
    Gm_LoadGameFile(&archive, BINGO_ASSET_FILENAME);
    card_set = ((JOBJSet**)Archive_GetPublicAddress(archive, "IfBingoCard_scene_models"))[0];
    icon_set = ((JOBJSet**)Archive_GetPublicAddress(archive, "IfBingoIcon_scene_models"))[0];
    notif_set = ((JOBJSet**)Archive_GetPublicAddress(archive, "IfBingoNotif_scene_models"))[0];

}

GOBJ *BingoUI_Create(int ply)
{
    int tick_start = OSGetTick();
    int tick;

    Vec3 text_pos;
    Text *t;

    // create gobj and model
    GOBJ *b = GOBJ_EZCreator(0, GAMEPLINK_CAMHUD, 0,
                            sizeof(BingoUIData), BingoUI_Destroy,
                            HSD_OBJKIND_JOBJ, card_set->jobj, 
                            BingoUI_Think, 22, 
                            JObj_GX, GAMEGX_HUD, 3);

    BingoUIData *bd = b->userdata;
    bd->ply = ply;

    // wide adjust
    if (g_wide_export)
    {
        g_wide_export->HUDAdjust_Element(b, BINGO_UI_ICONGRID_JOINT, false, WIDEALIGN_RIGHT, HEIGHTALIGN_CENTER);
        g_wide_export->HUDAdjust_Element(b, BINGO_UI_SCOREBOARD_JOINT, false, WIDEALIGN_LEFT, HEIGHTALIGN_CENTER);
        g_wide_export->HUDAdjust_Element(b, BINGO_UI_DESCRIPTION_JOINT, false, WIDEALIGN_RIGHT, HEIGHTALIGN_CENTER);
    }

    // create text
    {
        int x_pos;

        // details 
        JObj_GetChildPosition(b->hsd_object, BINGO_UI_DESCRIPTION_TEXT_JOINT, &text_pos);
        t = Text_CreateText(BINGO_SIS_INDEX, bingo_text_canvas_idx);
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
        bd->text.details = t;

        // create scoreboard
        JObj_GetChildPosition(b->hsd_object, BINGO_UI_SCOREBOARD_TEXT_JOINT, &text_pos);

        // labels
        t = Text_CreateText(BINGO_SIS_INDEX, bingo_text_canvas_idx);
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
        bd->text.scoreboard.label = t;

        // game info
        t = Text_CreateText(BINGO_SIS_INDEX, bingo_text_canvas_idx);
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
        Text_AddSubtext(t, x_pos, 80, "Goal: " "\x0C\xFF\xFF\x50" "%d items", 10);
        Text_AddSubtext(t, x_pos, 110, "Time Limit: Off");
        Text_AddSubtext(t, x_pos, 140, "Team: " "\x0C\x26\x26\xD9" "Blue");
        bd->text.scoreboard.game_info = t;

        // leaderboard numbers
        t = Text_CreateText(BINGO_SIS_INDEX, bingo_text_canvas_idx);
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
        bd->text.scoreboard.leaderboard_nums = t;
        
        // teams
        t = Text_CreateText(BINGO_SIS_INDEX, bingo_text_canvas_idx);
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
        bd->text.scoreboard.leaderboard_teams = t;

        // team scores
        t = Text_CreateText(BINGO_SIS_INDEX, bingo_text_canvas_idx);
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
        bd->text.scoreboard.leaderboard_scores = t;

    }

    // create bingo icons
    JOBJ *card_j = JObj_GetIndex(b->hsd_object, BINGO_UI_ICONGRID_JOINT);
    float width = 18.5;
    for (int x = 0; x < BINGO_UI_GRID_SIZE; x++)
    {
        for (int y = 0; y < BINGO_UI_GRID_SIZE; y++)
        {
            int goal_idx = (x * BINGO_UI_GRID_SIZE) + y;
            BingoGoal *goal = &g_bingo_card.goal[goal_idx];


            JOBJ *icon_j = JObj_LoadJoint(icon_set->jobj);
            JObj_AddSetAnim(icon_j, 0, icon_set, 0, 0);
            JObj_AddNext(card_j, icon_j);

            Bingo_SetIconForGoal(goal, icon_j, BINGO_UI_JOINT_SINGLE_ICON,
                                               BINGO_UI_JOINT_MULTI_ICON,
                                               BINGO_UI_JOINT_SINGLE_DIGIT,
                                               BINGO_UI_JOINT_DOUBLE_DIGIT);

            // position on grid
            icon_j->trans.X = -(width / 2) + (width * ((float)x / (float)(BINGO_UI_GRID_SIZE - 1)));
            icon_j->trans.Y = -(width / 2) + (width * ((float)y / (float)(BINGO_UI_GRID_SIZE - 1)));
            JObj_SetMtxDirtySub(icon_j);

            int index = (x * BINGO_UI_GRID_SIZE) + y;
            bd->icon_arr[index] = icon_j;
        }
    }
    JObj_SetMtxDirtySub(card_j);



    bingo_card_gobj[ply] = b;

    // update UI
    BingoUI_Think(b);

    OSReport("Created Bingo UI in %.2fms\n", MillisecondsSinceTick(tick_start));

    return b;
}
void BingoUI_Think(GOBJ *g)
{
    BingoUIData *gp = g->userdata;
    BingoTrackerData *tp = bingo_tracker_gobj[gp->ply]->userdata;

    // update cursor
    HSD_Pad *pad = &stc_engine_pads[Ply_GetControllerIndex(gp->ply)];
    if (pad->down & PAD_BUTTON_DPAD_RIGHT)
        if (++bingo_cursor[gp->ply].x > BINGO_UI_GRID_SIZE - 1) bingo_cursor[gp->ply].x = 0;
    if (pad->down & PAD_BUTTON_DPAD_LEFT)
        if (--bingo_cursor[gp->ply].x < 0) bingo_cursor[gp->ply].x = BINGO_UI_GRID_SIZE - 1;
    if (pad->down & PAD_BUTTON_DPAD_UP)
        if (++bingo_cursor[gp->ply].y > BINGO_UI_GRID_SIZE - 1) bingo_cursor[gp->ply].y = 0;
    if (pad->down & PAD_BUTTON_DPAD_DOWN)
        if (--bingo_cursor[gp->ply].y < 0) bingo_cursor[gp->ply].y = BINGO_UI_GRID_SIZE - 1;

    int sel_icon_idx = (bingo_cursor[gp->ply].x * BINGO_UI_GRID_SIZE) + bingo_cursor[gp->ply].y;
    BingoGoal *sel_goal = &g_bingo_card.goal[sel_icon_idx];

    // update icons 
    for (int x = 0; x < BINGO_UI_GRID_SIZE; x++)
    {
        for (int y = 0; y < BINGO_UI_GRID_SIZE; y++)
        {
            int index = (x * BINGO_UI_GRID_SIZE) + y;

            // update highlight
            int outline_frame = (x == bingo_cursor[gp->ply].x && y == bingo_cursor[gp->ply].y) ? 1 : 0;
            JOBJ *outline_icon_j = JObj_GetIndex(gp->icon_arr[index], BINGO_UI_JOINT_BACKGROUND_OUTLINE);
            JObj_SetFrameAndRate(outline_icon_j, outline_frame, 0);
            
            BingoGoal *goal = &g_bingo_card.goal[index];
            Bingo_UpdateIconProgress(goal, tp->progress[index], gp->icon_arr[index], BINGO_UI_JOINT_BACKGROUND_FILL);
        }
    }

    // update description
    char s[128];
    Bingo_GetDescriptionForGoal(sel_goal, s);
    Text_SetText(gp->text.details, 0, s);
    
    Text_SetText(gp->text.details, 1, "Progress: %d \x81\x5E %d.", tp->progress[sel_icon_idx], sel_goal->num);

}
void BingoUI_Destroy(BingoUIData *bd)
{
    Text_Destroy(bd->text.details);
    Text_Destroy(bd->text.scoreboard.game_info);
    Text_Destroy(bd->text.scoreboard.label);
    Text_Destroy(bd->text.scoreboard.leaderboard_nums);
    Text_Destroy(bd->text.scoreboard.leaderboard_teams);
    Text_Destroy(bd->text.scoreboard.leaderboard_scores);

    bingo_card_gobj[bd->ply] = 0;

    HSD_Free(bd);
}
void BingoUI_DestroyOnPause()
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
            GObj_Destroy(bingo_card_gobj[ply]);
    }
}

void BingoInput_Create()
{
    // add proc to rider that checks to create the bingo UI
    if (!is_bingo_mode || Gm_GetCityMode() != CITYMODE_TRIAL)
        return;

    // create text canvas
    bingo_text_canvas_idx = Text_CreateCanvas(BINGO_SIS_INDEX, -1, 28, GAMEPLINK_CAMHUD, 0, GAMEGX_HUD, 3, 0);

    // add new proc to each rider with a viewport
    for (int i = 0; i < GetElementsIn(bingo_card_gobj); i++)
    {
        if (Ply_GetPKind(i) == PKIND_NONE || Gm_Get3dData()->plyview_lookup[i] == -1)
            continue;

        GOBJ *r = Ply_GetRiderGObj(i);
        GObj_AddProc(r, BingoInput_Think, 3);
    }
}
void BingoInput_Think(GOBJ *r)
{
    // not during the intro
    if (Gm_GetIntroState() != GMINTRO_END)
        return;

    RiderData *rp = r->userdata;
    Game3dData *g3d = Gm_Get3dData();

    GOBJ *b = bingo_card_gobj[rp->ply];
    
    if (!b && rp->input.held & PAD_BUTTON_Y)
    {
        BingoUI_Create(rp->ply);        
        Gm_HideHUD();
    }
    else if (b)
    {
        // destroy if released button
        if (!(rp->input.held & PAD_BUTTON_Y))
        {
            GObj_Destroy(b);
            Gm_ShowHUD();
        }
        // update UI
        else
        {
            // 
        }
    }
}