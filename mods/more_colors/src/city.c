
#include "text.h"
#include "useful.h"
#include "hsd.h"
#include "preload.h"
#include "scene.h"
#include "inline.h"
#include "audio.h"
#include "game.h"
#include <string.h>

#include "colors.h"
#include "airride.h"
#include "code_patch/code_patch.h"

#include "hoshi/settings.h"

extern struct UIColor stc_ui_colors[];
extern int stc_ui_colors_num;

// City Select
void CitySelect_UpdatePlayer(int ply, int p_kind, int color_idx)
{
    if (!stc_scene_menu_common->city_select.cityselect_menu_gobj)
        return;

    CitySelectMenuData *md = stc_scene_menu_common->city_select.cityselect_menu_gobj->userdata;
    int *select_color_start = (int *)0x804ab608;

    md->ply[ply].p_kind = p_kind;
    md->ply[ply].color_idx = color_idx;

    int frame = select_color_start[p_kind];
    // if (p_kind <= 1 || p_kind == 2)
    //     frame += color_idx;

    static ElementColorDesc stc_col_desc[] =
        {
            {.joint_idx = 2, .dobj_idx = 5, .col = 0},  // main fill
            {.joint_idx = 2, .dobj_idx = 6, .col = 0},  // border
            {.joint_idx = 2, .dobj_idx = 2, .col = 0},  // underside
            {.joint_idx = 2, .dobj_idx = 3, .col = 0},  // topside
            {.joint_idx = 2, .dobj_idx = 4, .col = 0},  // bottom line
            {.joint_idx = 2, .dobj_idx = 7, .col = 0},  // vehicle shadow
            {.joint_idx = 2, .dobj_idx = 8, .col = 0},  // person icon
            {.joint_idx = 2, .dobj_idx = 10, .col = 0}, // ply text player
            {-1},                                       // terminator
        };

    // copy colors to struct
    stc_col_desc[0].col = &stc_ui_colors[color_idx].main;
    stc_col_desc[1].col = &stc_ui_colors[color_idx].dark;
    stc_col_desc[2].col = &stc_ui_colors[color_idx].bright;
    stc_col_desc[3].col = &stc_ui_colors[color_idx].bright;
    stc_col_desc[4].col = &stc_ui_colors[color_idx].main;
    stc_col_desc[5].col = &stc_ui_colors[color_idx].dark;
    stc_col_desc[6].col = &stc_ui_colors[color_idx].dark;
    stc_col_desc[7].col = &stc_ui_colors[color_idx].dark;

    Select_UpdatePlyBoard(md->ply[ply].active_j, p_kind, frame, stc_col_desc);
}

void CitySelect_Cursor0Update(int ply, int color_idx)
{
    if (stc_scene_menu_common->city_select.cursor0_gobj[ply])
    {
        JOBJ *j = stc_scene_menu_common->city_select.cursor0_gobj[ply]->hsd_object;
        CitySelectCursor0Data *cd = stc_scene_menu_common->city_select.cursor0_gobj[ply]->userdata;

        cd->color = color_idx;
        cd->x18 = 0; // makeshift timer
        // JObj_AddSetAnim(j, color_idx, stc_scene_menu_common->ScMenSelplyCursor0Ct_scene_models[0], 0, 1);
        // JObj_SetAllAOBJLoopByFlags(j, 0xffff);
    }

    return;
}
void CitySelect_Cursor0Think(GOBJ *g)
{
    static void (*CitySelect_GetCursor0Pos)(int ply, int r4, float *x, float *y) = (void *)0x8015aa5c;

    CitySelectCursor0Data *cd = g->userdata;
    JOBJ *cj = g->hsd_object;

    Vec2 pos;
    CitySelect_GetCursor0Pos(cd->ply, 0, &pos.X, &pos.Y);
    cj->trans.X = pos.X;
    cj->trans.Y = pos.Y;
    JObj_SetMtxDirtySub(cj);

    JObj_AnimAll(cj);

    // inc timer
    cd->x1c++;
    if (cd->x1c > 10)
    {
        cd->x1c = 0;

        cd->x1d++;
        if (cd->x1d > 1)
            cd->x1d = 0;
    }

    // get colors
    GXColor col_start, col_end, col_cur;
    if (cd->x1d == 0)
    {
        col_start = stc_ui_colors[cd->color].main;
        col_end = stc_ui_colors[cd->color].dark;
    }
    else
    {
        col_start = stc_ui_colors[cd->color].dark;
        col_end = stc_ui_colors[cd->color].main;
    }

    // lerp colors
    float time = (float)cd->x1c / 10.0;
    col_cur.r = lerp(col_start.r, col_end.r, time);
    col_cur.g = lerp(col_start.g, col_end.g, time);
    col_cur.b = lerp(col_start.b, col_end.b, time);
    col_cur.a = 255;

    Select_SetUIColor(cj, 1, 0, col_cur); // inactive player cursor
    Select_SetUIColor(cj, 2, 0, col_cur); // active player cursor
}

void CitySelect_Cursor1Update(int ply, int color_idx)
{
    if (stc_scene_menu_common->city_select.cursor1_gobj[ply])
    {
        JOBJ *j = stc_scene_menu_common->city_select.cursor1_gobj[ply]->hsd_object;
        CitySelectCursor1Data *cd = stc_scene_menu_common->city_select.cursor1_gobj[ply]->userdata;

        cd->color_idx = color_idx;

        // JObj_SetFrameAndRate(cd->background_jobj, color_idx, 0);

        Select_SetUIColor(j, 3, 0, stc_ui_colors[color_idx].main);
    }

    return;
}

void CitySelect_Cursor5Update(int ply, int color_idx)
{
    if (stc_scene_menu_common->city_select.cursor5_gobj[ply])
    {
        JOBJ *j = stc_scene_menu_common->city_select.cursor5_gobj[ply]->hsd_object;
        CitySelectCursor0Data *cd = stc_scene_menu_common->city_select.cursor5_gobj[ply]->userdata;

        cd->color = color_idx;
        cd->x18 = 0; // makeshift timer
        // JObj_AddSetAnim(j, color_idx, stc_scene_menu_common->ScMenSelplyCursor0Ct_scene_models[0], 0, 1);
        // JObj_SetAllAOBJLoopByFlags(j, 0xffff);
    }

    return;
}
void CitySelect_Cursor5Think(GOBJ *g)
{

    CitySelectCursor0Data *cd = g->userdata;
    JOBJ *cj = g->hsd_object;

    // inc timer
    cd->x1c++;
    if (cd->x1c > 10)
    {
        cd->x1c = 0;

        cd->x1d++;
        if (cd->x1d > 1)
            cd->x1d = 0;
    }

    // get colors
    GXColor *col_start, *col_end, col_cur;
    if (cd->x1d == 0)
    {
        col_start = &stc_ui_colors[cd->color].main;
        col_end = &stc_ui_colors[cd->color].dark;
    }
    else
    {
        col_start = &stc_ui_colors[cd->color].dark;
        col_end = &stc_ui_colors[cd->color].main;
    }

    // lerp colors
    float time = (float)cd->x1c / 10.0;
    col_cur.r = lerp(col_start->r, col_end->r, time);
    col_cur.g = lerp(col_start->g, col_end->g, time);
    col_cur.b = lerp(col_start->b, col_end->b, time);
    col_cur.a = 255;

    Select_SetUIColor(cj, 0, 0, col_cur); // border
}

void CitySelect_Cursor6Think(GOBJ *g)
{
    CitySelectCursor6Data *cd = g->userdata;
    JOBJ *cj = g->hsd_object;

    JObj_AnimAll(cj);

    if (cd->timer > 0)
    {
        cd->timer--;

        if (cd->timer <= 0)
        {
            if (cd->x1c == 0)
                cd->x1c = 1;

            cd->timer = 15;

            JObj_ReqAnimAll(cd->motion_jobj, 11);
            AOBJ_SetRate(cd->motion_jobj->aobj, 1);
            JObj_Anim(cd->motion_jobj);
        }
    }

    // update color
    Select_SetUIColor(cj, 2, 0, stc_ui_colors[cd->color_idx].main); // underside
}

int CitySelect_GetNextColor(int ply, int dir, int col_idx)
{
    GameData *gd = Gm_GetGameData();
    int inc_amt = (dir == 0) ? -1 : 1;
    int icon_idx = gd->city_select_ply.icon[ply];

    while (1)
    {

        // dedede and meta knight
        if (icon_idx == 10 || icon_idx == 19)
        {
            // clamp
            if (col_idx < 0)
                col_idx = 7;
            if (col_idx > 7)
                col_idx = 0;
        }
        // kirby
        else
        {
            // clamp
            if (col_idx < 0)
                col_idx = stc_ui_colors_num - 1;
            else if (col_idx >= stc_ui_colors_num)
                col_idx = 0;
        }
        // check if unlocked
        if (0)
        {
            col_idx += inc_amt;
            continue;
        }

        break;
    }

    return col_idx;
}
CODEPATCH_HOOKCREATE(0x8002f288, "mr 3,22\n\t"               // ply
                                 "mr 4,27\n\t"               // next or prev
                                 "mr 5,30\n\t",              // current color
                     CitySelect_GetNextColor, "mr 30,3\n\t", // next color
                     0x8002f358)

void CitySelect_LimitNonKirbyColor(int ply)
{
    GameData *gd = Gm_GetGameData();
    int icon_idx = gd->city_select_ply.icon[ply];

    if (icon_idx == 10 || icon_idx == 19)
    {
        if (gd->city_select_ply.color[ply] > 7)
        {
            gd->city_select_ply.color[ply] = 0;

            int anim_col = Select_ColorToAnim(gd->city_select_ply.color[ply]);

            // update all UI elements to reflect this color
            CitySelect_UpdatePlayer(ply, PKIND_HMN, anim_col);
            CitySelect_Cursor0Update(ply, anim_col);
            CitySelect_Cursor1Update(ply, anim_col);
            CitySelect_Cursor5Update(ply, anim_col);
            CitySelect_Cursor6Update(ply, anim_col);
        }
    }
}
CODEPATCH_HOOKCREATE(0x800315b0, "mr 3,25\n\t", CitySelect_LimitNonKirbyColor, "", 0)

void CitySelect_Init()
{
    // City Select Player
    CODEPATCH_REPLACEFUNC(0x80159f00, CitySelect_UpdatePlayer);
    CODEPATCH_REPLACEFUNC(0x8015b65c, CitySelect_Cursor0Update);
    CODEPATCH_REPLACEFUNC(0x8015b820, CitySelect_Cursor0Think);
    CODEPATCH_REPLACEFUNC(0x8015d0e4, CitySelect_Cursor1Update);
    CODEPATCH_REPLACEFUNC(0x8015f554, CitySelect_Cursor6Think);
    CODEPATCH_REPLACEFUNC(0x8015e1cc, CitySelect_Cursor5Update);
    CODEPATCH_REPLACEFUNC(0x8015e478, CitySelect_Cursor5Think);
    // CODEPATCH_REPLACEINSTRUCTION(0x8002f28c, 0x2c000000 | stc_ui_colors_num - 1); // city color bound check
    // CODEPATCH_REPLACEINSTRUCTION(0x8002f2a0, 0x3bc00000 | stc_ui_colors_num - 1); // city color bound check
    CODEPATCH_REPLACEINSTRUCTION(0x8015b620, 0x38800000); // always use anim0 for cursor1

    CODEPATCH_HOOKAPPLY(0x8002f288);

    CODEPATCH_HOOKAPPLY(0x800315b0);
}