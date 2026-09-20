
#include "text.h"
#include "os.h"
#include "hsd.h"
#include "preload.h"
#include "game.h"
#include "weapon.h"
#include "scene.h"
#include "inline.h"
#include "debug.h"

#include "hoshi/func.h"
#include "hoshi/screen_cam.h"

#include "code_patch/code_patch.h"

#include <string.h>

#include "bingo.h"

JOBJSet *card_set;
JOBJSet *icon_set;

static BingoCard g_bingo_card;

// Bingo Game Mode
void BingoMode_Start()
{
    // generate bingo card here
    for (int i = 0; i < BINGO_UI_GRID_SIZE * BINGO_UI_GRID_SIZE; i++)
    {
        g_bingo_card.goal[i].condition_num = HSD_Randi(GetElementsIn(g_bingo_card.goal[i].condition_data)) + 1;

        for (int cond_idx = 0; cond_idx < g_bingo_card.goal[i].condition_num; cond_idx++)
        {
            BingoConditionData *cd = &g_bingo_card.goal[i].condition_data[cond_idx];

            // get random condition
            int is_duplicate_condition;
            do
            {
                is_duplicate_condition = 0;

                cd->condition_kind = HSD_Randi(CONDITION_NUM);
                for (int k = 0; k < j; k++)
                {
                    BingoConditionData *that_cd = &g_bingo_card.goal[i].condition_data[k];
                    if (that_cd->condition_kind == cd->condition_kind)
                    {
                        is_duplicate_condition = 1;
                        break;
                    }
                }
            } while (is_duplicate_condition);
            
            // init condition variables
            switch(cd->condition_kind)
            {
                case (CONDITION_HIT):
                {
                    cd->is_expires = 0;
                    cd->num = HSD_Randi(3) + 1;

                    cd->hit.attack_kind = HSD_Randi(ATTACK_NUM);
                    
                    // target
                    cd->hit.target = HSD_Randi(TARGET_NUM);
                    switch(cd->hit.target)
                    {
                        case(TARGET_MACHINE):
                            cd->hit.target_kind.machine = HSD_Randi(VCKIND_NUM);
                            break;
                        case(TARGET_ENEMY):
                            cd->hit.target_kind.enemy = HSD_Randi(3);
                            break;
                        case(TARGET_ITEM):
                            cd->hit.target_kind.item = ITKIND_BOXBLUE + HSD_Randi(ITKIND_BOXRED + 1);
                            break;
                        case(TARGET_YAKUMONO):
                            cd->hit.target_kind.yakumono = HSD_Randi(5);
                            break;
                    }
                    
                    // damage source
                    cd->hit.dmg_source = HSD_Randi(DMGSOURCE_NUM);
                    switch(cd->hit.dmg_source)
                    {
                        case(AUDIOEMITTER_MACHINE):
                            cd->hit.dmg_source_kind.player_attack = HSD_Randi(5);
                            break;
                        case(AUDIOEMITTER_WEAPON):
                            cd->hit.dmg_source_kind.weapon_kind = HSD_Randi(5);
                            break;
                        case(AUDIOEMITTER_MAP):
                            cd->hit.dmg_source_kind.weapon_kind = HSD_Randi(5);
                            break;
                    }

                    break;
                }
                case (CONDITION_AIR):
                {
                    cd->is_expires = 1;
                    cd->num = 1;
                    cd->air.comparison = COMPARE_GREATER;
                    cd->air.frame = (3 * 60) + HSD_Randi(5 * 60);
                    break;
                }
                case (CONDITION_ITEM):
                {
                    cd->is_expires = 1;
                    cd->num = 1;
                    cd->item_kind = HSD_Randi(ITKIND_NUM);
                    break;
                }
                case (CONDITION_SPEED):
                {
                    cd->is_expires = 1;
                    cd->num = 1;
                    cd->speed.amt = 30 + HSD_Randi(80);
                    break;
                }
                case (CONDITION_POSITION):
                {
                    cd->is_expires = 1;
                    cd->num = 1;
                    cd->map.is_on_foot = 0;
                    cd->map.map_area = HSD_Randi(10);
                    break;
                }
            }
        }
    }

    for (int i = 0; i < BINGO_UI_GRID_SIZE * BINGO_UI_GRID_SIZE; i++)
    {
        OSReport("Bingo Icon %d with %d conditions:\n", i + 1, g_bingo_card.goal[i].condition_num);
        
        for (int j = 0; j < g_bingo_card.goal[i].condition_num; j++)
        {
            BingoConditionData *cd = &g_bingo_card.goal[i].condition_data[j];

            char s[256];

            switch(cd->condition_kind)
            {
                case (CONDITION_HIT):
                {
                    char *attack_string = (cd->hit.attack_kind == ATTACK_HIT) ? "hit" : "ko";
                    char target_string[32];
                    switch (cd->hit.target)
                    {
                        case (TARGET_PLAYER):
                            strcpy(target_string, "player");
                            break;
                        case (TARGET_MACHINE):
                        case (TARGET_ITEM):
                        case (TARGET_ENEMY):
                        case (TARGET_YAKUMONO):
                            char *target_name;
                            int index;
                            if (cd->hit.target == TARGET_MACHINE){
                                target_name = "MACHINE";
                                index = cd->hit.target_kind.machine;
                            }
                            else if (cd->hit.target == TARGET_ITEM){
                                target_name = "ITEM";
                                index = cd->hit.target_kind.item;
                            }
                            else if (cd->hit.target == TARGET_ENEMY){
                                target_name = "ENEMY";
                                index = cd->hit.target_kind.enemy;
                            }
                            else if (cd->hit.target == TARGET_YAKUMONO){
                                target_name = "YAKUMONO";
                                index = cd->hit.target_kind.yakumono;
                            }
                            sprintf(target_string, "%s index %d", target_name, index);
                            break;
                    }
                    char dmg_source_string[32];
                    switch (cd->hit.dmg_source)
                    {
                        case (DMGSOURCE_PLAYER):
                        case (DMGSOURCE_WEAPON):
                        case (DMGSOURCE_MAP):
                            char *source_name;
                            int index;
                            if (cd->hit.dmg_source == DMGSOURCE_PLAYER){
                                source_name = "player attack";
                                index = cd->hit.dmg_source_kind.player_attack;
                            }
                            else if (cd->hit.dmg_source == DMGSOURCE_WEAPON){
                                source_name = "weapon";
                                index = cd->hit.dmg_source_kind.weapon_kind;
                            }
                            else if (cd->hit.dmg_source == DMGSOURCE_MAP){
                                source_name = "map";
                                index = cd->hit.dmg_source_kind.map_kind;
                            }
                            
                            sprintf(dmg_source_string, "%s index %d", source_name, index);
                            break;

                        case (DMGSOURCE_ANY):
                            strcpy(dmg_source_string, "anything");
                            break;
                    }
                    
                    sprintf(s, "%s %s with %s", attack_string, target_string, dmg_source_string);

                    break;
                }
                case (CONDITION_AIR):
                {
                    char *compare_string;
                    switch (cd->air.comparison)
                    {
                        case (COMPARE_GREATER):
                            compare_string = "greater than";
                            break;
                        case (COMPARE_LESS):
                            compare_string = "less than";
                            break;
                    }
                    sprintf(s, "being airborne for %s %d frames", compare_string, cd->air.frame);
                    break;
                }
                case (CONDITION_ITEM):
                {
                    sprintf(s, "have item %d", cd->item_kind);
                    break;
                }
                case (CONDITION_SPEED):
                {
                    char *compare_string;
                    switch (cd->air.comparison)
                    {
                        case (COMPARE_GREATER):
                            compare_string = "greater than";
                            break;
                        case (COMPARE_LESS):
                            compare_string = "less than";
                            break;
                    }
                    sprintf(s, "have item %d", cd->item_kind);
                    break;
                }
                case (CONDITION_POSITION):
                {
                    char *on_foot_string;
                    if (cd->map.is_on_foot)
                        on_foot_string = " on foot";
                    else
                        on_foot_string = "";

                    sprintf(s, "at position %d%s", cd->map.map_area, on_foot_string);
                    break;
                }
            }

            char *intro_phrase = (cd->is_expires) ? "while " : "\0";

            OSReport(" %d: %s%s %d time(s)\n", j + 1, intro_phrase, s, cd->num);
        }
    }
}

// Bingo UI
int is_bingo_mode = 1;
GOBJ *bingo_card_gobj[5];
BingoCursor bingo_cursor[5];
int text_canvas_idx = 0;
GOBJ *BingoUI_Create(int ply)
{
    Vec3 text_pos;
    Text *t;

    // create gobj and model
    GOBJ *b = GOBJ_EZCreator(0, GAMEPLINK_CAMHUD, 0,
                            sizeof(BingoUIData), BingoUI_Destroy,
                            HSD_OBJKIND_JOBJ, card_set->jobj, 
                            BingoUI_Think, 0, 
                            JObj_GX, GAMEGX_HUD, 3);

    BingoUIData *bp = b->userdata;
    bp->ply = ply;

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
        Text_AddSubtext(t, x_pos, 80, "Goal: " "\x0C\xFF\xFF\x50" "%d items", 10);
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
    float width = 17.5;
    for (int x = 0; x < BINGO_UI_GRID_SIZE; x++)
    {
        for (int y = 0; y < BINGO_UI_GRID_SIZE; y++)
        {
            JOBJ *icon_j = JObj_LoadJoint(icon_set->jobj);
            JObj_AddSetAnim(icon_j, 0, icon_set, 6, 0);
            JObj_AddNext(card_j, icon_j);

            // display objective icon
            int icon_frame_idx = HSD_Randi(51);
            JOBJ *objective_icon_j = JObj_GetIndex(icon_j, 2);
            JObj_SetFrameAndRate(objective_icon_j, icon_frame_idx, 0);

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
            icon_j->trans.X = -(width / 2) + (width * ((float)x / (float)(BINGO_UI_GRID_SIZE - 1)));
            icon_j->trans.Y = -(width / 2) + (width * ((float)y / (float)(BINGO_UI_GRID_SIZE - 1)));
            JObj_SetMtxDirtySub(icon_j);

            int index = (x * BINGO_UI_GRID_SIZE) + y;
            bp->icon_arr[index] = icon_j;
        }
    }
    JObj_SetMtxDirtySub(card_j);

    bingo_card_gobj[ply] = b;

    // update UI
    BingoUI_Think(b);

    return b;
}
void BingoUI_Think(GOBJ *g)
{
    BingoUIData *gp = g->userdata;

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

    // update icon highlight
    for (int x = 0; x < BINGO_UI_GRID_SIZE; x++)
    {
        for (int y = 0; y < BINGO_UI_GRID_SIZE; y++)
        {
            int index = (x * BINGO_UI_GRID_SIZE) + y;
            int fill_frame = (x == bingo_cursor[gp->ply].x && y == bingo_cursor[gp->ply].y) ? 1 : 0;
            JOBJ *fill_icon_j = JObj_GetIndex(gp->icon_arr[index], 1);

            JObj_SetFrameAndRate(fill_icon_j, fill_frame, 0);
        }
    }

    // update description
    int cursor_index = (bingo_cursor[gp->ply].x * BINGO_UI_GRID_SIZE) + bingo_cursor[gp->ply].y;
    Text_SetText(gp->text.details, 0, "This is icon index %d.", cursor_index);
    Text_SetText(gp->text.details, 1, "Please complete this task.");

}
void BingoUI_Destroy(BingoUIData *bp)
{
    Text_Destroy(bp->text.details);
    Text_Destroy(bp->text.scoreboard.game_info);
    Text_Destroy(bp->text.scoreboard.label);
    Text_Destroy(bp->text.scoreboard.leaderboard_nums);
    Text_Destroy(bp->text.scoreboard.leaderboard_teams);
    Text_Destroy(bp->text.scoreboard.leaderboard_scores);

    bingo_card_gobj[bp->ply] = 0;

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
            GObj_Destroy(bingo_card_gobj[ply]);
    }
}

void BingoInput_Create()
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

static int g_dmg_log_num;
static DamageLog g_dmg_log[32];
void DamageLog_Clear()
{
    g_dmg_log_num = 0;
    memset(g_dmg_log, 0, sizeof(g_dmg_log));
}
void DamageLog_CopyFromHurtData(DamageLogObject *log, HurtData *hurt_data)
{
    GOBJ *gobj = hurt_data->gobj;

    log->hurt_kind = hurt_data->kind;

    log->state2 = -1;

    switch (hurt_data->kind)
    {
        case (HURTKIND_RIDER):
        {
            RiderData *rp = gobj->userdata;
            log->kind = rp->kind;
            log->ply = rp->ply;
            log->state = rp->status;

            if (rp->machine_gobj)
                log->is_airborne = Rider_IsMachineAirborne(rp);
            else
                log->is_airborne = rp->is_airborne;

            break;
        }
        case (HURTKIND_MACHINE):
        case (HURTKIND_MACHINE_EMPTY):
        {
            MachineData *mp = gobj->userdata;
            log->kind = mp->kind;
            log->state = mp->status;
            log->state2 = mp->status2;
            
            log->ply = Machine_GetRiderPly(mp);
            log->is_airborne = mp->is_airborne;
            
            break;
        }
        case (HURTKIND_WEAPON):
        {
            WeaponData *wp = gobj->userdata;
            log->kind = wp->kind;
            log->state = wp->state;
            log->is_airborne = 1;

            if (wp->owner_gobj && wp->owner_gobj->p_link == GAMEPLINK_RIDER)
            {
                // assuming only riders can own weapons...
                RiderData *rp = wp->owner_gobj->userdata;
                log->ply = rp->ply;
            }
            else
                log->ply = -1;
            
            break;
        }
        case (HURTKIND_ITEM):
        {
            ItemData *ip = gobj->userdata;
            log->kind = ip->kind;
            log->state = ip->state;
            log->is_airborne = ip->is_airborne;
            log->ply = -1;

            
            break;
        }
        case (HURTKIND_MAP):
        {
            YakumonoData *yp = gobj->userdata;
            log->kind = yp->kind;
            log->state = yp->state;
            log->ply = -1;
            log->is_airborne = 0;
            
            break;
        }
    }

}
void DamageLog_Add(HurtData *hurt_data, HurtData *attacker_hurt_data, int ply, int kind)
{
    DamageLog *this_log = &g_dmg_log[g_dmg_log_num++];

    DamageLog_CopyFromHurtData(&this_log->victim, hurt_data);
    DamageLog_CopyFromHurtData(&this_log->attacker, attacker_hurt_data);

    this_log->kb_mag = hurt_data->kb_mag;
    this_log->dmg = hurt_data->dmg_taken;

    // determine KO
    int is_ko = 0;
    switch (hurt_data->kind)
    {
        case (HURTKIND_MACHINE):
        case (HURTKIND_MACHINE_EMPTY):
        {
            MachineData *mp = hurt_data->gobj->userdata;
            if (hurt_data->dmg_taken >= mp->hp)
                is_ko = 1;
            
            break;
        }
        case (HURTKIND_ITEM):
        {
            ItemData *ip = hurt_data->gobj->userdata;
            if (hurt_data->dmg_taken >= (ip->hp - ip->dmg))
                is_ko = 1;
            
            break;
        }
    }
    this_log->is_ko = is_ko;
}
void DamageLog_Machine(MachineData *mp, HitCollLog *log)
{
    DamageLog_Add(mp->hurt_data, log->attacker, Machine_GetRiderPly(mp), mp->kind);
}
CODEPATCH_HOOKCREATE(0x801d7358, "mr 3, 31\n\t" "mr 4, 30\n\t", DamageLog_Machine, "lwz	0, 0 (29)\n\t", 0)
HitCollLog *DamageLog_Box(HitCollLog *log, ItemData *ip)
{
    DamageLog_Add(ip->hurt_data, log->attacker, -1, ip->kind);

    return log;
}
CODEPATCH_HOOKCREATE(0x80252434, "mr 4, 31\n\t", DamageLog_Box, "", 0)
void DamageLog_Yakumono(GOBJ *y, int ply)
{
    DamageLog *this_log = &g_dmg_log[g_dmg_log_num++];
    
    YakumonoData *yp = y->userdata;

    GOBJ *m = Ply_GetMachineGObj(ply);
    MachineData *mp = m->userdata;
    
    this_log->attacker.ply = ply;
    this_log->attacker.hurt_kind = HURTKIND_MACHINE;
    this_log->attacker.kind = mp->kind;
    this_log->attacker.state = mp->status;
    this_log->attacker.is_airborne = mp->is_airborne;

    this_log->victim.hurt_kind = HURTKIND_MAP;
    this_log->victim.ply = -1;
    this_log->victim.kind = yp->kind;
    this_log->victim.state = yp->state;
    this_log->victim.is_airborne = 0;

    this_log->is_ko = 1;
    this_log->dmg = 1;
    this_log->kb_mag = 0;
}
CODEPATCH_HOOKCREATE(0x80105d90, "stwu	1, -40 (1)\n\t"
                                 "mflr 0\n\t"
                                 "stw 0, 44 (1)\n\t"
                                 "stw 3, 8 (1)\n\t"
                                 "stw 4, 12 (1)\n\t", 
                                 DamageLog_Yakumono,
                                 "lwz 3, 8 (1)\n\t"
                                 "lwz 4, 12 (1)\n\t"
                                 "lwz 0, 44 (1)\n\t"
                                 "mtlr 0\n\t"
                                 "addi 1, 1, 40\n\t", 
                                 0)



static int g_zone_log_num;
static ZoneLog g_zone_log[32];
void ZoneLog_Clear()
{
    g_zone_log_num = 0;
    memset(g_zone_log, -1, sizeof(g_zone_log));
}
void ZoneLog_Add(int ply, int zone_idx, ZoneKind kind)
{
    ZoneLog *this_log = &g_zone_log[g_zone_log_num++];

    this_log->ply = ply;
    this_log->idx = zone_idx;
    this_log->kind = kind;
}
void ZoneLog_Boost(MachineData *mp, int zone_idx)
{
    ZoneLog_Add(Machine_GetRiderPly(mp), zone_idx, ZONEKIND_BOOST);
}
CODEPATCH_HOOKCREATE(0x801cf6b0, "mr 3, 31\n\t" "mr 4, 30\n\t", ZoneLog_Boost, "", 0)
CODEPATCH_HOOKCREATE(0x801cf71c, "mr 3, 31\n\t" "mr 4, 30\n\t", ZoneLog_Boost, "", 0)
void ZoneLog_Lift(MachineData *mp, int zone_idx)
{
    ZoneLog_Add(Machine_GetRiderPly(mp), zone_idx, ZONEKIND_LIFT);
}
CODEPATCH_HOOKCREATE(0x801e3fa8, "mr 3, 30\n\t" "mr 4, 31\n\t", ZoneLog_Lift, "", 0)
CODEPATCH_HOOKCREATE(0x801e4014, "mr 3, 30\n\t" "mr 4, 31\n\t", ZoneLog_Lift, "", 0)

static int g_rail_log_num;
static RailLog g_rail_log[5];
void RailLog_Clear()
{
    g_rail_log_num = 0;
    memset(g_rail_log, -1, sizeof(g_rail_log));
}
void RailLog_Add(int ply, int rail_idx, float rail_progress)
{
    RailLog *this_log = &g_rail_log[g_rail_log_num++];

    this_log->ply = ply;
    this_log->idx = rail_idx;
    this_log->progress = rail_progress;
}
void RailLog_Enter(MachineData *mp, int rail_idx, float rail_progress)
{
    RailLog_Add(Machine_GetRiderPly(mp), rail_idx, rail_progress);
}
CODEPATCH_HOOKCREATE(0x801e46a8, "mr 3, 30\n\t" "mr 4, 31\n\t" "lfs 1, 12 (1)\n\t", RailLog_Enter, "", 0)

void Log_Clear()
{
    DamageLog_Clear();
    ZoneLog_Clear();
    RailLog_Clear();
}

static AreaBound g_area_bounds[] = {
    // forest
    MAKE_AREA_BOUND(-537, -295, 
                    20, 200,
                    -190, 212),
    // volcano
    MAKE_AREA_BOUND(-680, -350, 
                    52, 200,
                    -690, -160),
    MAKE_AREA_BOUND(-500, -219, 
                    52, 200,
                    -714, -404),
    // city
    MAKE_AREA_BOUND(-295, 240, 
                    25, 200,
                    -612, 100),
    // wharf
    MAKE_AREA_BOUND(260, 650, 
                    -10, 200,
                    -366, 360),
    // electric lounge
    MAKE_AREA_BOUND(-140, 500, 
                    -10, 200,
                    384, 750),
    // golf course
    MAKE_AREA_BOUND(-530, -250, 
                    -20, 200,
                    212, 900),

    // sky island top
    MAKE_AREA_BOUND(-183, 42, 
                    465, 520,
                    -82, 112),
    // rock flower
    MAKE_AREA_BOUND(-144, -100, 
                    200, 215,
                    -857, -815),
    // castle flower
    MAKE_AREA_BOUND(407, 412, 
                    368, 380,
                    -567, -563),
};

Text *debug_text;
void BingoTracker_Create()
{
    // add proc to rider that updates bingo progress
    if (!is_bingo_mode || Gm_GetCityMode() != CITYMODE_TRIAL)
        return;

    // create a new gobj for each player
    for (int i = 0; i < GetElementsIn(bingo_card_gobj); i++)
    {
        if (Ply_GetPKind(i) == PKIND_NONE)
            continue;

        GOBJ *t = GOBJ_EZCreator(0, GAMEPLINK_11, 0,
                                sizeof(BingoTrackerData), HSD_Free,
                                HSD_OBJKIND_NONE, 0, 
                                BingoTracker_Think, RDPRI_DMGAPPLY + 1, 
                                BingoTracker_GX, GAMEGX_MAP, 1);

        BingoTrackerData *tp = t->userdata;
        tp->ply = i;

        // init data
        for (int j = 0; j < BINGO_UI_GRID_SIZE * BINGO_UI_GRID_SIZE; j++)
        {
            tp->goal[j].is_goal_complete = 0;
            for (int k = 0; k < 3; k++)
                tp->goal[j].condition_progress[k] = 0;
        }

    }

    // create gobj proc to clear the damage log before updating hitcoll
    GOBJ *d = GOBJ_EZCreator(0, GAMEPLINK_11, 0,
                            0, 0,
                            HSD_OBJKIND_NONE, 0, 
                            Log_Clear, 0, 
                            0, 0, 0);

    debug_text = Hoshi_CreateScreenText();
    for (int i = 0; i < 2; i++)
        Text_AddSubtext(debug_text, 0, i * 30, "");

}
void BingoTracker_Think(GOBJ *t)
{
    BingoTrackerData *tp = t->userdata;
    RiderData *rp = Ply_GetRiderGObj(tp->ply)->userdata;
    GOBJ *m = rp->machine_gobj;

    // Game3dData *g3d = Gm_Get3dData();

    // update progress
    for (int goal_idx = 0; goal_idx < BINGO_UI_GRID_SIZE * BINGO_UI_GRID_SIZE; goal_idx++)
    {
        BingoGoal *goal = &g_bingo_card.goal[goal_idx];
        BingoGoalProgress *goal_progress = &tp->goal[goal_idx];

        // skip if goal is already completed
        if (goal_progress->is_goal_complete)
            continue;

        // update condition progress
        for (int condition_idx = 0; condition_idx < goal->condition_num; condition_idx++)
        {
            int is_condition_complete;

            BingoConditionData *condition = &goal->condition_data[condition_idx];
            is_condition_complete = goal_progress->condition_progress[condition_idx] >= condition->num;

            // if its completed and doesnt expire, skip it
            if (is_condition_complete && !condition->is_expires)
                continue;

            int result = 0;

            // check for condition success
            switch (condition->condition_kind)
            {
                case (CONDITION_HIT):
                {
                    // check all other players for damage this frame
                    for (int ply = 0; ply < 4; ply++)
                    {
                        // not me and exists
                        if (ply == rp->ply || Ply_GetPKind(ply) == PKIND_NONE)
                            continue;
                        
                        RiderData *this_rp = Ply_GetRiderGObj(ply)->userdata;

                        if (this_rp->machine_gobj)
                        {
                            MachineData *mp = this_rp->machine_gobj->userdata;
                            
                            if (mp->hurt_data->kb_mag > 0)
                            {
                                ;
                            }

                        }
                    }
                    break;
                }
                case (CONDITION_ITEM):
                {

                    break;
                }
                case (CONDITION_AIR):
                {
                    if (m)
                    {
                        MachineData *mp = m->userdata;
                        int frames = mp->frames_airborne;

                        if (condition->air.comparison == COMPARE_GREATER)
                            result = (frames >= condition->air.frame);
                        else
                            result = (frames < condition->air.frame);

                        break;
                    }

                    break;
                }
            }

            // debug log
            is_condition_complete = goal_progress->condition_progress[condition_idx] >= condition->num;
            if (result && is_condition_complete == 0)
                OSReport("completed goal #%d-%d\n", goal_idx + 1, condition_idx + 1);
            
            // update result
            goal_progress->condition_progress[condition_idx]++;
        }
    }

    for (int i = 0; i < GetElementsIn(g_dmg_log); i++)
    {
        DamageLog *this_log = &g_dmg_log[i];
        if (this_log->dmg > 0)
        {
            static char *hurt_kind_names[] = {
                "rider",
                "machine",
                "machine_empty",
                "3",
                "item",
                "weapon",
                "map",
            };
            static char *action_names[] = {
                "hurt",
                "KO'd",
            };


            OSReport("%s (ply %d, kind %d, state: %d/%d, is_airborne: %d) %s %s (ply %d, kind %d, state: %d/%d, is_airborne: %d) with %.2f damage\n", 
                                                        hurt_kind_names[this_log->attacker.hurt_kind],
                                                        this_log->attacker.ply,
                                                        this_log->attacker.kind,
                                                        this_log->attacker.state,
                                                        this_log->attacker.state2,
                                                        this_log->attacker.is_airborne,
                                                        action_names[this_log->is_ko],
                                                        hurt_kind_names[this_log->victim.hurt_kind],
                                                        this_log->victim.ply,
                                                        this_log->victim.kind,
                                                        this_log->victim.state,
                                                        this_log->victim.state2,
                                                        this_log->victim.is_airborne,
                                                        this_log->dmg
                                                        );
        }
    }
    for (int i = 0; i < GetElementsIn(g_rail_log); i++)
    {
        RailLog *this_log = &g_rail_log[i];
        if (this_log->ply != -1)
            OSReport("ply %d entered rail %d with progress %.2f\n", this_log->ply, this_log->idx, this_log->progress);
    }
    for (int i = 0; i < GetElementsIn(g_zone_log); i++)
    {
        ZoneLog *this_log = &g_zone_log[i];
        if (this_log->ply != -1)
            OSReport("ply %d entered zone index %d with kind %d\n", this_log->ply, this_log->idx, this_log->kind);
    }

    Text_SetText(debug_text, 0, "%.2f, %.2f, %.2f", rp->pos.X, rp->pos.Y, rp->pos.Z);
    Text_SetText(debug_text, 1, "", rp->pos.X, rp->pos.Y, rp->pos.Z);

    for (int i = 0; i < GetElementsIn(g_area_bounds); i++)
    {
        float hx = g_area_bounds[i].size.X * 0.5f;
        float hy = g_area_bounds[i].size.Y * 0.5f;
        float hz = g_area_bounds[i].size.Z * 0.5f;

        float x_min = g_area_bounds[i].pos.X - hx;
        float x_max = g_area_bounds[i].pos.X + hx;
        float y_min = g_area_bounds[i].pos.Y - hy;
        float y_max = g_area_bounds[i].pos.Y + hy;
        float z_min = g_area_bounds[i].pos.Z - hz;
        float z_max = g_area_bounds[i].pos.Z + hz;

        if (rp->pos.X >= x_min && rp->pos.X <= x_max && 
            rp->pos.Y >= y_min && rp->pos.Y <= y_max && 
            rp->pos.Z >= z_min && rp->pos.Z <= z_max)
            {
                Text_SetText(debug_text, 1, "area: %d", i);
                break;
            }
    }


}
void BingoTracker_GX(GOBJ *t, int pass)
{
    return;

    if (pass != 2)
        return;

    // draw areas
    for (int i = 0; i < GetElementsIn(g_area_bounds); i++)
        GX_DrawBox(&g_area_bounds[i].pos, &g_area_bounds[i].size, &(GXColor){255,0,0,128});
}

int Bingo_OnDPadToStick(RiderData *rd)
{
    // int button_mask = ~(PAD_BUTTON_DPAD_DOWN | PAD_BUTTON_DPAD_UP | PAD_BUTTON_DPAD_LEFT | PAD_BUTTON_DPAD_RIGHT);

    if (bingo_card_gobj[rd->ply])
        return 1;

    return 0;

}
CODEPATCH_HOOKCONDITIONALCREATE(0x8018f0bc, "mr 3, 31\n\t", Bingo_OnDPadToStick, "", 0, 0x8018f0cc)

void GOBJProc_Log(GOBJProc *proc, int level)
{
    while (proc)
    {
        int s_link = (proc->gobj) ? proc->gobj->p_link : -1;
        OSReport(" %*sProc %p s_link %d. GOBJ %p (%d) callback %p\n", level, "", proc, proc->s_link, proc->gobj, s_link, proc->cb);

        // if (proc->child)
        //     GOBJProc_Log(proc->child, level + 1);

        proc = proc->next;
    }

    return;
}

void Bingo_Init()
{
    // apply our code patches
    CODEPATCH_HOOKAPPLY(0x8018f0bc);

    // damage logging
    CODEPATCH_HOOKAPPLY(0x801d7358);
    CODEPATCH_HOOKAPPLY(0x80252434);

    // zone logging
    CODEPATCH_HOOKAPPLY(0x801cf6b0);
    CODEPATCH_HOOKAPPLY(0x801cf71c);
    CODEPATCH_HOOKAPPLY(0x801e3fa8);
    CODEPATCH_HOOKAPPLY(0x801e4014);
    
    // rail logging
    CODEPATCH_HOOKAPPLY(0x801e46a8);

    // yakumono logging
    // CODEPATCH_HOOKAPPLY(0x80105d90);     // events give all yakubreak hits to the lowest port player @ 80108334. disabling for now i guess
    
    // Hoshi_AddPreloadGameFile(BINGO_ASSET_FILENAME, PRELOADHEAPKIND_ALLM);
}
void Bingo_On3DLoadStart()
{
    // init bingo data
    for (int i = 0; i < GetElementsIn(bingo_card_gobj); i++)
    {
        bingo_card_gobj[i] = 0;
        bingo_cursor[i].x = (BINGO_UI_GRID_SIZE - 1) / 2;
        bingo_cursor[i].y = (BINGO_UI_GRID_SIZE - 1) / 2;;
    }

    OSReport("Load bingo card assets\n");

    // get our file
    HSD_Archive *archive;
    Gm_LoadGameFile(&archive, BINGO_ASSET_FILENAME);
    card_set = ((JOBJSet**)Archive_GetPublicAddress(archive, "IfBingoCard_scene_models"))[0];
    icon_set = ((JOBJSet**)Archive_GetPublicAddress(archive, "IfBingoIcon_scene_models"))[0];
}
void Bingo_On3DLoadEnd()
{
    BingoInput_Create();
    BingoTracker_Create();

    u8 proc_num = 26; // *stc_gobj_proc_num;
    for (int i = 0; i < proc_num; i++)
    {
        OSReport("Checking proc %d\n", i);

        GOBJProc *proc = (*stc_gobjproc_lookup)[i];
        GOBJProc_Log(proc, 0);
        OSReport("\n");
    }
    
}
void Bingo_On3DPause(int pause_ply)
{
    Bingo_DestroyOnPause();
}
void Bingo_On3DUnpause(int pause_ply)
{
}
void Bingo_OnPlayerSelectLoad()
{
    BingoMode_Start();
}