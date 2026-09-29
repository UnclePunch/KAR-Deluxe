#include "obj.h"
#include "text.h"
#include "game.h"

#include "bingo.h"
#include "tracker.h"
#include "log.h"
#include "notif.h"

extern int is_bingo_mode;
extern GOBJ *bingo_card_gobj[5];
extern BingoCard g_bingo_card;

// action logging
extern int g_dmg_log_num;
extern DamageLog g_dmg_log[32];
extern int g_zone_log_num;
extern ZoneLog g_zone_log[32];
extern int g_rail_log_num;
extern RailLog g_rail_log[5];

extern AreaBound g_area_bounds[];

GOBJ *bingo_tracker_gobj[5];

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
                                BingoTracker_Think, 21, 
                                BingoTracker_GX, GAMEGX_MAP, 1);

        bingo_tracker_gobj[i] = t;

        BingoTrackerData *tp = t->userdata;
        tp->ply = i;

        // init data
        for (int j = 0; j < BINGO_UI_GRID_SIZE * BINGO_UI_GRID_SIZE; j++)
            tp->progress[j] = 0;

        if (Ply_IsViewOn(i))
        {
            ;
        }
    }

    // create gobj proc to clear the damage log before updating hitcoll
    GOBJ *d = GOBJ_EZCreator(0, GAMEPLINK_11, 0,
                            0, 0,
                            HSD_OBJKIND_NONE, 0, 
                            Log_Clear, 0, 
                            0, 0, 0);

    // debug_text = Hoshi_CreateScreenText();
    // for (int i = 0; i < 2; i++)
    //     Text_AddSubtext(debug_text, 0, i * 30, "");

}
void BingoTracker_Think(GOBJ *t)
{
    BingoTrackerData *tp = t->userdata;
    RiderData *rd = Ply_GetRiderGObj(tp->ply)->userdata;
    MachineData *md = (rd->machine_gobj) ? rd->machine_gobj->userdata : 0;
    int is_view_on = Ply_IsViewOn(tp->ply);

    // update misc stats
    if (md && (md->is_airborne && (md->status == VCSTATE_FLY || md->status == VCSTATE_FLYPUSH)))
        tp->stats.glide_frames++;
    else
        tp->stats.glide_frames = 0;

    BingoGoalSFXKind sfx = GOALSFX_NONE;

    // update progress
    for (int goal_idx = 0; goal_idx < BINGO_UI_GRID_SIZE * BINGO_UI_GRID_SIZE; goal_idx++)
    {
        BingoGoal *goal = &g_bingo_card.goal[goal_idx];

        // skip if goal is already completed
        if (goal->ply_completed != -1)
            continue;

        int progress = Bingo_UpdateProgress(tp, goal_idx);

        if (progress != tp->progress[goal_idx])
        {   
            if (is_view_on)
            {
                BingoNotif_Create(goal, progress, tp->ply);

                // check if completed
                if (progress >= goal->num)
                {
                    goal->ply_completed = tp->ply;
                    sfx = GOALSFX_COMPLETE;
                }
                else
                {
                    // play sound if progress went up
                    if (progress > tp->progress[goal_idx])
                        sfx = GOALSFX_UP;
                    else
                        sfx = GOALSFX_DOWN;
                }
            }

            // update result
            tp->progress[goal_idx] = progress;
        }

    }

    if (sfx != GOALSFX_NONE)
    {
        static int goal_sfx_lookup[] = {
            FGMMENU_CS_BEEP1,
            FGMMENU_CS_KETTEI_PRE,
            FGMMENU_CS_KETTEI,
        };
        
        SFX_Play(goal_sfx_lookup[sfx]);
    }

    for (int i = 0; i < g_dmg_log_num; i++)
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


            // to-do: this crashes when taking damage from picking up a fake item
            OSReport("%s (ply %d, kind %d, state: %d/%d, attack_kind: %d, is_airborne: %d) %s %s (ply %d, kind %d, state: %d/%d, is_airborne: %d) with %.2f damage\n", 
                                                        hurt_kind_names[this_log->attacker.hurt_kind],
                                                        this_log->attacker.ply,
                                                        this_log->attacker.kind,
                                                        this_log->attacker.state,
                                                        this_log->attacker.state2,
                                                        this_log->attacker.attack_data.kind,
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
            OSReport("ply %d entered rail %d with progress %.2f in status %d with ground state %d\n", this_log->ply, this_log->idx, this_log->progress, this_log->status, this_log->is_airborne);
    }
    for (int i = 0; i < g_zone_log_num; i++)
    {
        ZoneLog *this_log = &g_zone_log[i];
        if (this_log->ply != -1)
            OSReport("ply %d entered zone index %d with kind %d\n", this_log->ply, this_log->idx, this_log->kind);
    }

    // RiderData *rp = Ply_GetRiderGObj(tp->ply)->userdata;
    // Text_SetText(debug_text, 0, "%.2f, %.2f, %.2f", rp->pos.X, rp->pos.Y, rp->pos.Z);
    // Text_SetText(debug_text, 1, "", rp->pos.X, rp->pos.Y, rp->pos.Z);

    // for (int i = 0; i < GetElementsIn(g_area_bounds); i++)
    // {
    //     float hx = g_area_bounds[i].size.X * 0.5f;
    //     float hy = g_area_bounds[i].size.Y * 0.5f;
    //     float hz = g_area_bounds[i].size.Z * 0.5f;

    //     float x_min = g_area_bounds[i].pos.X - hx;
    //     float x_max = g_area_bounds[i].pos.X + hx;
    //     float y_min = g_area_bounds[i].pos.Y - hy;
    //     float y_max = g_area_bounds[i].pos.Y + hy;
    //     float z_min = g_area_bounds[i].pos.Z - hz;
    //     float z_max = g_area_bounds[i].pos.Z + hz;

    //     if (rp->pos.X >= x_min && rp->pos.X <= x_max && 
    //         rp->pos.Y >= y_min && rp->pos.Y <= y_max && 
    //         rp->pos.Z >= z_min && rp->pos.Z <= z_max)
    //         {
    //             Text_SetText(debug_text, 1, "area: %d", i);
    //             break;
    //         }
    // }

}
void BingoTracker_GX(GOBJ *t, int pass)
{
    if (pass != 2)
        return;
}

int Bingo_UpdateProgress(BingoTrackerData *tp, int goal_idx)
{
    int ply = tp->ply;
    int progress = tp->progress[goal_idx];
    BingoGoal *gd = &g_bingo_card.goal[goal_idx];

    RiderData *rp = Ply_GetRiderGObj(ply)->userdata;
    u8 *stats = (u8 *)Ply_GetStats(ply);
    MachineData *mp = (rp->machine_gobj) ? (rp->machine_gobj->userdata) : 0;

    switch(gd->kind)
    {
        case (GOAL_STATGET):
        {
            int *item_collect_arr = (int *)&stats[0x4c8];
            progress = item_collect_arr[gd->param.stat_get.kind];
            break;
        }
        case (GOAL_FOODGET):
        {
            int *item_collect_arr = (int *)&stats[0x4c8];
            progress = item_collect_arr[gd->param.food_get.kind];
            break;
        }
        case (GOAL_ITEMFALLGET):
        {
            break;
        }
        case (GOAL_POSITION):
        {
            AreaBound *bound = &g_area_bounds[gd->param.position.kind];

            float hx = bound->size.X * 0.5f;
            float hy = bound->size.Y * 0.5f;
            float hz = bound->size.Z * 0.5f;

            float x_min = bound->pos.X - hx;
            float x_max = bound->pos.X + hx;
            float y_min = bound->pos.Y - hy;
            float y_max = bound->pos.Y + hy;
            float z_min = bound->pos.Z - hz;
            float z_max = bound->pos.Z + hz;

            if (rp->pos.X >= x_min && rp->pos.X <= x_max && 
                rp->pos.Y >= y_min && rp->pos.Y <= y_max && 
                rp->pos.Z >= z_min && rp->pos.Z <= z_max)
                {
                    progress++;
                }

            break;
        }
        case (GOAL_BREAKBOXANY):
        {
            for (int i = 0; i < g_dmg_log_num; i++)
            {
                DamageLog *this_log = &g_dmg_log[i];
                if (this_log->dmg > 0 && this_log->attacker.ply == ply && 
                    this_log->is_ko &&
                    this_log->victim.hurt_kind == HURTKIND_ITEM && 
                    this_log->victim.kind >= ITKIND_BOXBLUE && this_log->victim.kind <= ITKIND_BOXRED)
                {
                    progress++;
                }
            }

            break;
        }
        case (GOAL_BREAKBOXKIND):
        {
            for (int i = 0; i < g_dmg_log_num; i++)
            {
                DamageLog *this_log = &g_dmg_log[i];
                if (this_log->dmg > 0 && this_log->attacker.ply == ply && 
                    this_log->is_ko &&
                    this_log->victim.hurt_kind == HURTKIND_ITEM && 
                    this_log->victim.kind == gd->param.box_kind.kind)
                {
                    progress++;
                }
            }
            break;
        }
        case (GOAL_BREAKBOXWITHATTACK):
        {
            for (int i = 0; i < g_dmg_log_num; i++)
            {
                DamageLog *this_log = &g_dmg_log[i];
                if (this_log->dmg > 0 && this_log->attacker.ply == ply && 
                    this_log->is_ko &&
                    this_log->victim.hurt_kind == HURTKIND_ITEM && 
                    this_log->victim.kind >= ITKIND_BOXBLUE && this_log->victim.kind <= ITKIND_BOXRED && 
                    this_log->attacker.attack_data.kind == gd->param.box_attack.attack)
                {
                    progress++;
                }
            }
            break;
        }
        case (GOAL_HITPLAYER):
        {
            for (int i = 0; i < g_dmg_log_num; i++)
            {
                DamageLog *this_log = &g_dmg_log[i];
                if (this_log->dmg > 0 && this_log->attacker.ply == ply && this_log->victim.ply != ply &&
                    (this_log->victim.hurt_kind == HURTKIND_RIDER || this_log->victim.hurt_kind == HURTKIND_MACHINE))
                {
                    progress++;
                }
            }
            break;
        }
        case (GOAL_HITPLAYERWITHATTACK):
        {
            for (int i = 0; i < g_dmg_log_num; i++)
            {
                DamageLog *this_log = &g_dmg_log[i];
                if (this_log->dmg > 0 && this_log->attacker.ply == ply && 
                    (this_log->victim.hurt_kind == HURTKIND_RIDER || this_log->victim.hurt_kind == HURTKIND_MACHINE) && 
                    this_log->attacker.attack_data.kind == gd->param.hit_ply_attack.attack)
                {
                    progress++;
                }
            }
            break;
        }
        case (GOAL_DESTROYMACHINE):
        {
            for (int i = 0; i < g_dmg_log_num; i++)
            {
                DamageLog *this_log = &g_dmg_log[i];
                if (this_log->dmg > 0 && this_log->attacker.ply == ply && 
                    (this_log->victim.hurt_kind == HURTKIND_MACHINE || this_log->victim.hurt_kind == HURTKIND_MACHINE_EMPTY) && 
                    this_log->is_ko)
                {
                    progress++;
                }
            }
            break;
        }
        case (GOAL_RIDEMACHINEKIND):
        {
            ;
            break;
        }
        case (GOAL_RAILDISTANCE):
        {   
            float rail_dist = *(float *)&stats[0x614];

            if ((int)rail_dist > gd->param.rail_dist.dist)
                progress++;

            break;
        }
        case (GOAL_RAILLAND):
        {
            for (int i = 0; i < GetElementsIn(g_rail_log); i++)
            {
                RailLog *this_log = &g_rail_log[i];
                if (this_log->ply == ply && this_log->is_airborne && this_log->progress > 0.2)
                    progress++;
            }
            break;
        }
        case (GOAL_GLIDETIME):
        {
            progress = tp->stats.glide_frames / 60;

            break;
        }
        case (GOAL_BOOSTRING):
        {
            static u8 boost_zone_ids[] = {26, 27, 28, 29, 58, 59, 60, 61, 62, 63, 64, 65, 66, 67, 68, 69};

            u8 *zone_bits = &stats[0x661];
            int ring_num = 0;

            for (int i = 0; i < GetElementsIn(boost_zone_ids); i++)
            {
                int this_boost_idx = boost_zone_ids[i];

                int byte_offset = this_boost_idx / 8;
                int bit_idx = 1 << (this_boost_idx % 8);

                if (zone_bits[byte_offset] & bit_idx)
                    ring_num++;
            }

            progress = ring_num;

            break;
        }
    }

    return progress;
}
