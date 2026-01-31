#include "os.h"
#include "inline.h"
#include "obj.h"
#include "hud.h"
#include "game.h"

#include "code_patch/code_patch.h"

// Quick Stats
int quickstats_enabled = 1;
void QuickStat_Think(GOBJ *r)
{
    // not during the intro
    if (Gm_GetIntroState() != GMINTRO_END)
        return;

    RiderData *rp = r->userdata;
    Game3dData *g3d = Gm_Get3dData();

    GOBJ *cityui_stat_gobj = g3d->cityui_statchart_gobj[rp->ply];

    if (!cityui_stat_gobj && rp->input.held & PAD_BUTTON_Y)
    {
        // temporarily create pause hud so the stat creation function
        // knows where to place the jobj
        HUD_PauseCreate();

        CityHUD_CreateStatChart(rp->ply, rp->ply);
        GObj_RemoveProc(g3d->cityui_statchart_gobj[rp->ply]); // remove proc that attempts to self destruct
        // GObj_AddProc(g3d->cityui_statchart_gobj[rp->ply], CityUI_StatThink, 20);

        for (int i = 0; i < 9; i++)
        {
            CityHUD_CreateStatBar(rp->ply, rp->ply, i);
            GObj_RemoveProc(g3d->cityui_statbar_gobj[rp->ply][i]); // remove proc that attempts to self destruct
            // GObj_AddProc(g3d->cityui_statbar_gobj[rp->ply][i], CityUI_StatThink, 20);
        }

        // remove temp pause hud
        HUD_PauseDestroy();
    }
    else if (cityui_stat_gobj)
    {
        // destroy if released button
        if (!(rp->input.held & PAD_BUTTON_Y))
        {
            GObj_Destroy(cityui_stat_gobj);
            g3d->cityui_statchart_gobj[rp->ply] = 0;

            // destroy bars
            for (int i = 0; i < 9; i++)
            {
                GObj_Destroy(g3d->cityui_statbar_gobj[rp->ply][i]);
                g3d->cityui_statbar_gobj[rp->ply][i] = 0;
            }
        }
        // update bars
        else
        {
            for (int stat_kind = 0; stat_kind < 9; stat_kind++)
            {
                HUDElementData *hp = g3d->cityui_statbar_gobj[rp->ply][stat_kind]->userdata;
                int stat_num = Ply_GetCityStatNum(rp->ply, stat_kind, 0);

                if (stat_kind != 8)
                    stat_num += 2;

                // update bar length
                if (stat_num > 0)
                    HUD_UpdateElement(hp->city_stat_bar.bar_j, stat_num); // 8012931c
                else
                    HUD_UpdateElement(hp->city_stat_bar.bar_j, 0);

                // 80129328
                if (stat_num > -10)
                    JObj_SetFlagsAll(hp->city_stat_bar.sign_j, JOBJ_HIDDEN);
                else
                    JObj_ClearFlagsAll(hp->city_stat_bar.sign_j, JOBJ_HIDDEN);

                // 8012933c
                if (stat_num < 10 && stat_num > -10)
                {
                    // 80129394
                    if (stat_num < 0 && stat_num > -10)
                    {
                        HUD_UpdateElement(hp->city_stat_bar.num_left_j, 10);           // 801293a4
                        JObj_ClearFlagsAll(hp->city_stat_bar.num_left_j, JOBJ_HIDDEN); // 801293b4
                    }
                    else
                        JObj_SetFlagsAll(hp->city_stat_bar.num_left_j, JOBJ_HIDDEN); // 801293b4
                }
                else
                {
                    int left_digit = (stat_num > 0) ? ((int)((float)stat_num / 10.0)) : ((int)(((float)stat_num * -1) / 10.0));

                    // 8012934c
                    HUD_UpdateElement(hp->city_stat_bar.num_left_j, left_digit);
                    JObj_ClearFlagsAll(hp->city_stat_bar.num_left_j, JOBJ_HIDDEN);
                }

                // 801293c0
                int right_digit = (stat_num > 0) ? (stat_num % 10) : ((stat_num * -1) % 10);
                HUD_UpdateElement(hp->city_stat_bar.num_right_j, right_digit);
            }
        }
    }
}
void QuickStat_OnPause()
{
    // destroys any existing stat charts when pausing
    if (!quickstats_enabled || !(Gm_IsInCity() && Gm_GetCityMode() == CITYMODE_TRIAL))
        return;

    Game3dData *g3d = Gm_Get3dData();

    // destroy any existing stat screens
    for (int ply = 0; ply < 4; ply++)
    {
        if (Ply_GetPKind(ply) == PKIND_NONE)
            continue;

        if (g3d->cityui_statchart_gobj[ply])
        {

            GObj_Destroy(g3d->cityui_statchart_gobj[ply]);
            g3d->cityui_statchart_gobj[ply] = 0;

            // destroy bars
            for (int stat_kind = 0; stat_kind < 9; stat_kind++)
            {
                GObj_Destroy(g3d->cityui_statbar_gobj[ply][stat_kind]);
                g3d->cityui_statbar_gobj[ply][stat_kind] = 0;
            }
        }
    }
}
void QuickStat_On3DStart()
{
    // add proc to rider that checks to create the stat HUD
    if (!quickstats_enabled || !(Gm_IsInCity() && Gm_GetCityMode() == CITYMODE_TRIAL))
        return;

    // add new proc to each rider with a viewport
    for (int i = 0; i < 4; i++)
    {
        if (Ply_GetPKind(i) == PKIND_NONE || Gm_Get3dData()->plyview_lookup[i] == -1)
            continue;

        GOBJ *r = Ply_GetRiderGObj(i);
        GObj_AddProc(r, QuickStat_Think, 3);
    }
}
void QuickStat_Init()
{
}
