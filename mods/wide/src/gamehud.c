#include "text.h"
#include "debug.h"
#include "os.h"
#include "hsd.h"
#include "hud.h"

#include "wide.h"
#include "gamehud.h"

#include "code_patch/code_patch.h"

CamScissor ply_viewport_1[] = {
    {
        .left = 0,
        .right = 640,
        .top = 0,
        .bottom = 480,
    },
};
CamScissor ply_viewport_2[] = {
    {
        .left = 4,
        .right = 636,
        .top = 20,
        .bottom = 228,
    },
    {
        .left = 4,
        .right = 636,
        .top = 252,
        .bottom = 460,
    },
};
CamScissor ply_viewport_4[] = {
    {
        .left = 10,
        .right = 318,
        .top = 20,
        .bottom = 228,
    },
    {
        .left = 10,
        .right = 318,
        .top = 252,
        .bottom = 460,
    },
    {
        .left = 322,
        .right = 630,
        .top = 20,
        .bottom = 228,
    },
    {
        .left = 322,
        .right = 630,
        .top = 252,
        .bottom = 460,
    },
};
CamScissor *ply_orig_viewport_lookup[] = {
    ply_viewport_1,
    ply_viewport_2,
    ply_viewport_4,
    ply_viewport_4,
};

void (*ply_viewport_get[])(int view_index, CamScissor *out) = {
    PlyCam_Get2PScissor,
    PlyCam_Get4PScissor,
    PlyCam_Get4PScissor,
};

static float ply_viewport_2_inf_y = (21.3f);
static float ply_viewport_2_event_text_y = (18);
static float ply_viewport_2_hud_scale = (0.9);
static CamScissor ply_viewport_2_custom[] = {
    // p1
    {
        .left = 4,
        .right = 318,
        .top = 40,
        .bottom = 460,
    },
    // p2
    {
        .left = 322,
        .right = 636,
        .top = 40,
        .bottom = 460,
    },
};
static CamScissor map_viewport_2_custom = {
    .top = 105,
    .bottom = 225,
    .left = 240,
    .right = 400,
};

void DebugHUD_GX(GOBJ *g, int pass)
{
    if (pass != 2)
        return;

    COBJ *c = COBJ_GetCurrent();
    CamBounds bounds;

    Game3dData *g3d = Gm_Get3dData();
    int plyview_num = Gm_GetPlyViewNum();
    if (plyview_num > 1)
    { 
        // HUDElementData *gp = g3d->plyview_pos_gobj->userdata;
        // for (int i = 0; i < plyview_num; i++)
        // {
        //     Vec3 *pos = &gp->ply_hud.pos[i];
        //     GX_DrawLine(&(Vec3){pos->X - 2, pos->Y, 0}, 
        //                 &(Vec3){pos->X + 2, pos->Y, 0},  
        //                 10, &(GXColor){255, 0, 0, 255});
        //     GX_DrawLine(&(Vec3){pos->X, pos->Y - 2, 0}, 
        //                 &(Vec3){pos->X, pos->Y + 2, 0},  
        //                 10, &(GXColor){255, 0, 0, 255});
        // }

        COBJ_GetBounds(c, 0, &bounds);
        for (int i = 0; i < plyview_num; i++)
        {
            CamScissor view_arr;
            ply_viewport_get[plyview_num - 2](i, &view_arr);
            float width_pixel = view_arr.right - view_arr.left;
            float height_pixel = view_arr.bottom - view_arr.top;
            
            // OSReport("view_arr.right: %d\n", view_arr.right);
            // OSReport("view_arr.left: %d\n", view_arr.left);
            // OSReport("view_arr.bottom: %d\n", view_arr.bottom);
            // OSReport("view_arr.top: %d\n", view_arr.top);

            float center_x_pixel = (float)(view_arr.right + view_arr.left) / 2.f;
            float center_y_pixel = (float)(view_arr.bottom + view_arr.top) / 2.f;

            Vec2 center_pos = (Vec2){.X = ((center_x_pixel / 640.f) * ORIG_WIDTH) - (ORIG_WIDTH / 2),
                                     .Y = ((center_y_pixel / 480.f) * ORIG_HEIGHT) - (ORIG_HEIGHT / 2)};

            float width_half = (width_pixel / 2 / 640.f) * ORIG_WIDTH;
            float height_half = (height_pixel / 2 / 480.f) * ORIG_HEIGHT;

            // OSReport("width_pixel: %.2f\n", width_pixel);
            // OSReport("height_pixel: %.2f\n", height_pixel);
            // OSReport("center_x_pixel: %.2f\n", center_x_pixel);
            // OSReport("center_y_pixel: %.2f\n", center_y_pixel);
            // OSReport("center_pos.X: %.2f\n", center_pos.X);
            // OSReport("center_pos.Y: %.2f\n", center_pos.Y);
            // OSReport("width_half: %.2f\n", width_half);
            // OSReport("height_half: %.2f\n", height_half);

            GX_DrawLine(&(Vec3){center_pos.X - width_half, center_pos.Y, 0}, 
                        &(Vec3){center_pos.X + width_half, center_pos.Y, 0},  
                        10, &(GXColor){255, 0, 0, 255});
            GX_DrawLine(&(Vec3){center_pos.X, center_pos.Y - height_half, 0}, 
                        &(Vec3){center_pos.X, center_pos.Y + height_half, 0},  
                        10, &(GXColor){255, 0, 0, 255});

            // position model
            HUDElementData *gp = g3d->plyview_pos_gobj->userdata;
            Vec3 *plyview_pos_offset = &gp->ply_hud.pos[i];

            GX_DrawLine(&(Vec3){plyview_pos_offset->X - 2, plyview_pos_offset->Y, 0}, 
                        &(Vec3){plyview_pos_offset->X + 2, plyview_pos_offset->Y, 0},  
                        10, &(GXColor){0, 255, 0, 255});
            GX_DrawLine(&(Vec3){plyview_pos_offset->X, plyview_pos_offset->Y - 2, 0}, 
                        &(Vec3){plyview_pos_offset->X, plyview_pos_offset->Y + 2, 0},  
                        10, &(GXColor){0, 255, 0, 255});
        }
    }
    else
    {
        COBJ_GetBounds(c, 0, &bounds);
        GX_DrawLine(&(Vec3){bounds.left, 0, 0}, 
                    &(Vec3){bounds.right, 0, 0},
                    10, &(GXColor){255,0,0,255});
                    
        GX_DrawLine(&(Vec3){0, bounds.top, 0}, 
                    &(Vec3){0, bounds.bot, 0},
                    10, &(GXColor){0,255,0,255});

        OSReport("u: %.2f  d: %.2f  l: %.2f  r: %.2f\n", 
                    bounds.top, bounds.bot, bounds.left, bounds.right);
    }
}
void DebugCamera_GX(GOBJ *g, int pass)
{
    if (pass != 2)
        return;

    for (GOBJ *r = (*stc_gobj_lookup)[GAMEPLINK_RIDER]; r; r = r->next)
    {
        RiderData *rd = r->userdata;

        PlayerCamData *ply_cam = stc_plycam_lookup->cam_gobjs[rd->ply]->userdata;

        static GXColor color = {255, 0, 255, 128};
        static GXColor color_high = {0, 255, 255, 128};
        
        DebugGX_DrawSphere(3.f, &ply_cam->cam_data->interest_pos, &ply_cam->cam_data->interest_pos, &color, &color);
        DebugGX_DrawSphere(3.f, &ply_cam->cam_data->eye_pos, &ply_cam->cam_data->eye_pos, &color_high, &color_high);
    }
}

void Camera_AdjustWidth(WideAlign align, float left_in, float right_in, float *left_out, float *right_out)
{
    float aspect_mult = Wide_GetAspectMult();
    
    float edge_x = 0;
    float width = 640.f;
    if (aspect_mult > 1.33333333333)
    {
        width = (640.f * 1.33333333333) / aspect_mult;
        edge_x = (640.f - width) / 2;
    }
    
    // OSReport("in:\n");
    // OSReport("left %.2f, right %.2f\n", left_in, right_in);

    switch (align)
    {
        // case (WIDEALIGN_CENTER):
        // {
        //     float center_x_normalized = ((right_in + left_in) / 2) / 640.0f;
        //     float new_center_x = edge_x + (center_x_normalized * width);

        //     float view_width = right_in - left_in;    
        //     float adjusted_view_width = (view_width / aspect_mult); // shrinks with wider AR (preserves pixel width when display stretches the image)

        //     // OSReport("center_normalized: %.2f\n", center_x_normalized);
        //     // OSReport("new_center_x: %.2f\n", new_center_x);

        //     // OSReport("view_width: %.2f\n", view_width);
        //     // OSReport("adjusted_view_width: %.2f\n", adjusted_view_width);

        //     *left_out = new_center_x - (adjusted_view_width / 2);
        //     *right_out = new_center_x + (adjusted_view_width / 2);
        
        //     break;
        // }

        case (WIDEALIGN_LEFT):
        case (WIDEALIGN_RIGHT):
        {

            float inverse_scale = Wide_GetInverseScale();
            // OSReport("inverse_scale: %.2f\n", inverse_scale);

            if (align == WIDEALIGN_LEFT)
            {
                *left_out = edge_x + (left_in * inverse_scale);
                *right_out = edge_x + (right_in * inverse_scale);
            }
            else
            {
                *left_out = edge_x + (640 - (640 - left_in) * inverse_scale);
                *right_out = edge_x + (640 - (640 - right_in) * inverse_scale);
            }

            break;
        }
    }
    
    // OSReport("out:\n");
    // OSReport("left %.2f, right %.2f\n", *left_out, *right_out);
}

GOBJ *HUDMain_Create(GOBJ *g)
{
    OSReport("created main hud element with p_link %d, gx_link %d\n", g->p_link, g->gx_link);
    return g;
}
CODEPATCH_HOOKCREATE(0x80114b8c, "mr 3,31\n\t", HUDMain_Create, "", 0)
GOBJ *HUDPlayer_Create(GOBJ *g)
{
    OSReport("created player hud element with p_link %d, gx_link %d\n", g->p_link, g->gx_link);
    return g;
}
CODEPATCH_HOOKCREATE(0x80114cf0, "mr 3,30\n\t", HUDPlayer_Create, "", 0)
GOBJ *HUDMisc_Create(GOBJ *g)
{
    OSReport("created misc hud element with p_link %d, gx_link %d\n", g->p_link, g->gx_link);
    return g;
}
CODEPATCH_HOOKCREATE(0x80114858, "mr 3,31\n\t", HUDMisc_Create, "", 0)
// GOBJ *HUDAbility_Create(GOBJ *g)
// {
//     OSReport("created ability hud element with p_link %d, gx_link %d\n", g->p_link, g->gx_link);
//     return g;
// }
// CODEPATCH_HOOKCREATE(0x80119c20, "mr 3,26\n\t", HUDAbility_Create, "", 0)

void HUD_PixelToPos(u16 x, u16 y, float mult, Vec2 *out)
{
    float width = ORIG_WIDTH * mult;

    out->X = ((float)x / 640.f) * width - (width / 2);
    out->Y = -(((float)y / 480.f) * ORIG_HEIGHT - (ORIG_HEIGHT / 2));
}

void HUDAdjust_Element(GOBJ *g, int joint_index, int is_ply_element, WideAlign x_align, HeightAlign y_align)
{
    Game3dData *g3d = Gm_Get3dData();
    HUDElementData *gp = g->userdata;

    // if not a player view element, use fullscreen region
    int plyview_num = g3d->plyview_num;

    // WIP custom splitscreen adjustment 
    if (plyview_num > 1)
    {
        // return;

        CamScissor *orig_view_arr = &ply_orig_viewport_lookup[plyview_num - 1][gp->ply];

        CamScissor view_arr;
        ply_viewport_get[plyview_num - 2](gp->ply, &view_arr);

        Vec3 *plyview_pos_offset;
        switch (gp->kind)
        {
            default:
            {
              HUDElementData *pos_data = g3d->plyview_pos_gobj->userdata;
              plyview_pos_offset = &pos_data->ply_hud.pos[gp->ply];
              break;
            }
            case (HUDKIND_CITYSTATBG):
            case (HUDKIND_CITYSTATBAR):
            {
              HUDElementData *pos_data = g3d->cityui_pause_gobj->userdata;
              plyview_pos_offset = &pos_data->city_pause.ply_offset[gp->ply];
              break;
            }
        }

        u16 orig_x_pixel, cur_x_pixel;
        if (x_align == WIDEALIGN_LEFT)
        {
            orig_x_pixel = orig_view_arr->left;
            cur_x_pixel = view_arr.left;
        }
        else if (x_align == WIDEALIGN_RIGHT)
        {
            orig_x_pixel = orig_view_arr->right;
            cur_x_pixel = view_arr.right;
        }

        u16 orig_y_pixel, cur_y_pixel;
        if (y_align == HEIGHTALIGN_TOP)
        {
            orig_y_pixel = orig_view_arr->top;
            cur_y_pixel = view_arr.top;
        }
        else if (y_align == HEIGHTALIGN_BOTTOM)
        {
            orig_y_pixel = orig_view_arr->bottom;
            cur_y_pixel = view_arr.bottom;
        }

        // get offset from bottom of original viewport to hud position model root
        Vec2 orig_corner_pos;
        HUD_PixelToPos(orig_x_pixel, 
                        orig_y_pixel,
                        1.0f,
                        &orig_corner_pos);
        Vec2 cur_corner_pos;
        HUD_PixelToPos(cur_x_pixel, 
                        cur_y_pixel,
                        Wide_GetAspectMult(),
                        &cur_corner_pos);
        
        OSReport("orig_corner_pos.X: %.2f\n", orig_corner_pos.X);
        OSReport("orig_corner_pos.Y: %.2f\n", orig_corner_pos.Y);
        OSReport("cur_corner_pos.X: %.2f\n", cur_corner_pos.X);
        OSReport("cur_corner_pos.Y: %.2f\n", cur_corner_pos.Y);
        OSReport("plyview_pos_offset->Y: %.2f\n", plyview_pos_offset->Y);

        JOBJ *j = (joint_index == 0) ? g->hsd_object : JObj_GetIndex(g->hsd_object, joint_index);

        float new_x = cur_corner_pos.X + (plyview_pos_offset->X - orig_corner_pos.X);
        float new_y = cur_corner_pos.Y + (plyview_pos_offset->Y - orig_corner_pos.Y);
        
        OSReport("x pos: %.2f -> %.2f\n", j->trans.X, new_x);
        OSReport("y pos: %.2f -> %.2f\n", j->trans.Y, new_y);

        j->trans.X = new_x;
        j->trans.Y = new_y;

        JObj_SetMtxDirtySub(j);

        return;
    }

    CamScissor viewport;
    CamScissor *orig_viewport;
    switch (plyview_num)
    {
        case (1):
            PlyCam_GetFullscreenScissor(&viewport);
            orig_viewport = &viewport;
            break;
        case (2):
            PlyCam_Get2PScissor(gp->ply, &viewport);
            orig_viewport = &ply_viewport_2[gp->ply];
            break;
        case (3):
        case (4):
            PlyCam_Get4PScissor(gp->ply, &viewport);
            orig_viewport = &ply_viewport_4[gp->ply];
            break;
    }
    JOBJ *j = (joint_index == 0) ? g->hsd_object : JObj_GetIndex(g->hsd_object, joint_index);

    float dir = (x_align == WIDEALIGN_LEFT) ? -1 : 1;
    
    float aspect_scale = (*stc_cobj_aspect) / ORIG_ASPECT;
    if (aspect_scale > 1.3333)
        aspect_scale = 1.3333;

    float viewport_scale_x = (float)((viewport.right - viewport.left)) / (float)((orig_viewport->right - orig_viewport->left));
    float viewport_scale_y = (float)((viewport.bottom - viewport.top)) / (float)((orig_viewport->bottom - orig_viewport->top));
    float cur_width = ORIG_WIDTH * viewport_scale_x * aspect_scale;
    float cur_height = ORIG_HEIGHT * viewport_scale_y;
    float extra_x = (cur_width - ORIG_WIDTH);
    float extra_y = (cur_height - ORIG_HEIGHT);
    j->trans.X += dir * extra_x * 0.5f;
    // j->trans.Y -= extra_y * 0.5f;
    JObj_SetMtxDirtySub(j);

    // OSReport("ply %d:\n", gp->ply);
    // OSReport(" aspect scale: %.2f\n", aspect_scale);
    // OSReport(" viewport scale: %.2f\n", viewport_scale);

}
void HUDAdjust_Camera(COBJ *c)
{
    float viewport_width = c->viewport_right - c->viewport_left;
    int scissor_wdith = c->scissor_right - c->scissor_left;
    float left, right;

    WideAlign align = (c->viewport_left + (viewport_width / 2) > 320.f) ? WIDEALIGN_RIGHT : WIDEALIGN_LEFT;

    if (viewport_width < 640.f)
    {
        Camera_AdjustWidth(align, c->viewport_left, c->viewport_right, &left, &right);
        c->viewport_left = left;
        c->viewport_right = right;
    }

    if (scissor_wdith < 640)
    {
        Camera_AdjustWidth(align, c->scissor_left, c->scissor_right, &left, &right);
        c->scissor_left = left;
        c->scissor_right = right;
    }

    float aspect_mult = Wide_GetAspectMult();
    
    switch (c->projection_type)
    {
        case (PROJ_ORTHO):
        {
            c->projection_param.ortho.left *= aspect_mult;
            c->projection_param.ortho.right *= aspect_mult;
        }
    }
}

// speedometer
void HUDAdjust_RightTopAlign(GOBJ *g)
{
    HUDAdjust_Element(g, 0, true, WIDEALIGN_RIGHT, HEIGHTALIGN_TOP);
}
void HUDAdjust_RightBottomAlign(GOBJ *g)
{
    HUDAdjust_Element(g, 0, true, WIDEALIGN_RIGHT, HEIGHTALIGN_BOTTOM);
}
CODEPATCH_HOOKCREATE(0x80118e70, "mr 3,28\n\t", HUDAdjust_RightBottomAlign, "", 0)
CODEPATCH_HOOKCREATE(0x801181d4, "mr 3,27\n\t", HUDAdjust_RightBottomAlign, "", 0) // speedometer outer
CODEPATCH_HOOKCREATE(0x801242e4, "mr 3,28\n\t", HUDAdjust_RightBottomAlign, "", 0) // speedometer dead
CODEPATCH_HOOKCREATE(0x80119c20, "mr 3,26\n\t", HUDAdjust_RightBottomAlign, "", 0)
CODEPATCH_HOOKCREATE(0x8011a060, "mr 3,26\n\t", HUDAdjust_RightBottomAlign, "", 0)
CODEPATCH_HOOKCREATE(0x80123e18, "mr 3,30\n\t", HUDAdjust_RightBottomAlign, "", 0)
CODEPATCH_HOOKCREATE(0x80124650, "mr 3,30\n\t", HUDAdjust_RightBottomAlign, "", 0) // kirby walk speedometer
CODEPATCH_HOOKCREATE(0x80128004, "mr 3,29\n\t", HUDAdjust_RightTopAlign, "", 0)     // event compass
CODEPATCH_HOOKCREATE(0x8011ae08, "mr 3,28\n\t", HUDAdjust_RightTopAlign, "", 0)     // air ride lap num

void HUDAdjust_LeftTopAlign(GOBJ *g)
{
    HUDAdjust_Element(g, 0, true, WIDEALIGN_LEFT, HEIGHTALIGN_TOP);
}
void HUDAdjust_LeftBottomAlign(GOBJ *g)
{
    HUDAdjust_Element(g, 0, true, WIDEALIGN_LEFT, HEIGHTALIGN_BOTTOM);
}
CODEPATCH_HOOKCREATE(0x8012b650, "mr 3,30\n\t", HUDAdjust_LeftTopAlign, "", 0)
CODEPATCH_HOOKCREATE(0x801260fc, "mr 3,29\n\t", HUDAdjust_LeftBottomAlign, "", 0) // plynum2
CODEPATCH_HOOKCREATE(0x8012a94c, "mr 3,31\n\t", HUDAdjust_LeftTopAlign, "", 0) // kirby hit
CODEPATCH_HOOKCREATE(0x8011a4dc, "mr 29,3\n\t" "mr 3,28\n\t", HUDAdjust_LeftTopAlign, "b 0x8\n\t", 0) // placement number
CODEPATCH_HOOKCREATE(0x80130084, "mr 3,30\n\t", HUDAdjust_LeftTopAlign, "", 0) // target flight points
CODEPATCH_HOOKCREATE(0x8012fb74, "mr 3,29\n\t", HUDAdjust_LeftTopAlign, "", 0) // high jump previous points
CODEPATCH_HOOKCREATE(0x8012e964, "mr 3,29\n\t", HUDAdjust_LeftTopAlign, "", 0) // kirby melee points
CODEPATCH_HOOKCREATE(0x801306ec, "mr 3,29\n\t", HUDAdjust_LeftTopAlign, "li 0, 0\t\n", 0) // destruction derby points
CODEPATCH_HOOKCREATE(0x8012a350, "mr 3,30\n\t", HUDAdjust_LeftTopAlign, "", 0) // opponent finish
CODEPATCH_HOOKCREATE(0x8011bdc0, "mr 3,29\n\t", HUDAdjust_LeftTopAlign, "", 0) // stadium race record 1
CODEPATCH_HOOKCREATE(0x8011b7c0, "mr 3,30\n\t", HUDAdjust_LeftTopAlign, "", 0) // stadium race record 2

void HUDAdjust_AirGliderHUD(int ply)
{
    Game3dData *g3d = Gm_Get3dData();

    GOBJ *score_gobj = g3d->airglider_hud.score_gobj[ply];
    GOBJ *machineicon_gobj = g3d->airglider_hud.machineicon_gobj[ply];
    
    if (score_gobj)
    {
        if (Ply_IsViewOn(ply))
            HUDAdjust_RightBottomAlign(score_gobj);
        else
            HUDAdjust_LeftTopAlign(score_gobj);
    }

    if (machineicon_gobj)
        HUDAdjust_LeftTopAlign(machineicon_gobj);

}
CODEPATCH_HOOKCREATE(0x8012d324, "mr 3,26\n\t", HUDAdjust_AirGliderHUD, "", 0) // flight distance opponent

void HUDAdjust_HighJumpHUD(int ply)
{
    Game3dData *g3d = Gm_Get3dData();

    GOBJ *score_gobj = g3d->highjump_hud.score_gobj[ply];
    GOBJ *machineicon_gobj = g3d->highjump_hud.machineicon_gobj[ply];
    
    if (score_gobj)
    {
        if (Ply_IsViewOn(ply))
            HUDAdjust_RightBottomAlign(score_gobj);
        else
            HUDAdjust_LeftTopAlign(score_gobj);
    }

    if (machineicon_gobj)
        HUDAdjust_LeftTopAlign(machineicon_gobj);

}
CODEPATCH_HOOKCREATE(0x8012c4dc, "mr 3,26\n\t", HUDAdjust_HighJumpHUD, "", 0) // flight distance opponent

// pause
void HUDAdjust_PauseStats(GOBJ *g)
{
    WideAlign wide_align;
    HeightAlign height_align;
    if (Gm_GetPlyViewNum() == 0)
    {
        wide_align = WIDEALIGN_LEFT;
        height_align = HEIGHTALIGN_CENTER;
    }
    else
    {
        wide_align = WIDEALIGN_LEFT;
        height_align = HEIGHTALIGN_TOP;
    }
    
    HUDAdjust_Element(g, 0, false, wide_align, height_align);
}
CODEPATCH_HOOKCREATE(0x80128d4c, "mr 3,29\n\t", HUDAdjust_PauseStats, "", 0)
void HUDAdjust_CityPauseOptions(GOBJ *g)
{
    if (Gm_GetPlyViewNum() == 0)
    {
        // line
        HUDAdjust_Element(g, 1, false, WIDEALIGN_LEFT, HEIGHTALIGN_TOP);
        // options
        HUDAdjust_Element(g, 2, false, WIDEALIGN_RIGHT, HEIGHTALIGN_TOP);
        // player indicator
        HUDAdjust_Element(g, 6, false, WIDEALIGN_LEFT, HEIGHTALIGN_BOTTOM);
    }
}
CODEPATCH_HOOKCREATE(0x80128a50, "mr 3,29\n\t", HUDAdjust_CityPauseOptions, "", 0)
void HUDAdjust_AirRidePauseOptions(GOBJ *g)
{
    if (Gm_GetPlyViewNum() == 0)
    {
        // pause text
        HUDAdjust_Element(g, 1, false, WIDEALIGN_LEFT, HEIGHTALIGN_TOP);
        // options
        HUDAdjust_Element(g, 2, false, WIDEALIGN_RIGHT, HEIGHTALIGN_TOP);
        // player indicator
        HUDAdjust_Element(g, 3, false, WIDEALIGN_LEFT, HEIGHTALIGN_BOTTOM);
    }
}
CODEPATCH_HOOKCREATE(0x801283c0, "mr 3,29\n\t", HUDAdjust_AirRidePauseOptions, "", 0)
// hp bar
GOBJ *Hook_HPBarHUD(GOBJ *g)
{
    HUDAdjust_Element(g, 0, true, WIDEALIGN_RIGHT, HEIGHTALIGN_BOTTOM);
    return g;
}
CODEPATCH_HOOKCREATE(0x8011f670, "mr 3,30\n\t", Hook_HPBarHUD, "", 0)

// splitscreen pos model
void HUDAdjust_NormalizeViewCenter(CamScissor *scissor, Vec2 *out)
{
    u16 center_x = (scissor->right + scissor->left) / 2;
    u16 center_y = (scissor->bottom + scissor->top) / 2;
    (*out) = (Vec2){(float)(center_x - 320) / 320.0f,
            -(float)(center_y - 240) / 240.0f};

    return;
}
void HUDAdjust_PositionModel(GOBJ *g)
{
    return;

    Game3dData *g3d = Gm_Get3dData();

    if (!g3d->plyview_pos_gobj)
        return;
    
    HUDElementData *hp = g3d->plyview_pos_gobj->userdata;

    CamScissor scissor;
    Vec2 *offset;
    for (int ply = 0; ply < 4; ply++)
    {        
        switch (g3d->plyview_num)
        {
            case (1):
                PlyCam_GetFullscreenScissor(&scissor);
                static Vec2 offset_1p = {0, 0};
                offset = &offset_1p;
            case (2):
                PlyCam_Get2PScissor(ply, &scissor);
                static Vec2 offset_2p = {1.2, 7.4};
                offset = &offset_2p;
                break;

            case (3):
            case (4):
                PlyCam_Get4PScissor(ply, &scissor);
                static Vec2 offset_4p = {0, 0};
                offset = &offset_4p;
                break;
        }

        Vec2 normalized_center;
        HUDAdjust_NormalizeViewCenter(&scissor, &normalized_center);
        float width = ORIG_WIDTH * (*stc_cobj_aspect) / ORIG_ASPECT;
        hp->ply_hud.pos[ply].X = offset->X + (normalized_center.X * (width/2));
        hp->ply_hud.pos[ply].Y = offset->Y + (normalized_center.Y * (ORIG_HEIGHT/2));
    
        // OSReport("ply %d:\n", ply);
        // OSReport(" normalized: (%.2f, %.2f)\n", normalized_center.X, normalized_center.Y);
        // OSReport(" hud_center: (%.2f, %.2f)\n", hp->ply_hud.pos[ply].X, hp->ply_hud.pos[ply].Y);
    }
}
CODEPATCH_HOOKCREATE(0x80125d8c, "mr 3,28\n\t", HUDAdjust_PositionModel, "", 0)

// splitscreen info frame
void HUDAdjust_InfoBar(GOBJ *g)
{
    if (Gm_GetPlyViewNum() != 2)
        return;
    
    JOBJ *j = g->hsd_object;

    // i guess these will just be hardcoded?
    j->trans.Y = ply_viewport_2_inf_y;
}
CODEPATCH_HOOKCREATE(0x801258bc, "mr 3,30\n\t", HUDAdjust_InfoBar, "", 0)
CODEPATCH_HOOKCREATE(0x801192f4, "mr 3,29\n\t", HUDAdjust_InfoBar, "", 0)
CODEPATCH_HOOKCREATE(0x801279ec, "mr 3,27\n\t", HUDAdjust_InfoBar, "", 0)

static WideAdjustData wide_adjust_data[] = {
    // view bound scissors?
    {
        .ptr = (float *)0x805dfb6c,
    },
    {
        .ptr = (float *)0x805dfb70,
    },
    {
        .ptr = (float *)0x805dfb74,
    },
    {
        .ptr = (float *)0x805dfb78,
    },

    // plicon right screen edge
    {
        .ptr = (float *)0x805dfdf0,
    },

    // plicon bottom screen edge values
    // {
    //     .ptr = (float *)0x805dfdc8, // fov?
    // },
    {
        .ptr = (float *)0x805dfdd8, // this is bottom edge width divided by 2. can move 160 in direction, centered at the value @ 805dfdd4 (320 aka center of the screen)
    },
    {
        .ptr = (float *)0x805dfdd4, // bottom right pixel?
    },
};

// indicator HUD
void GXProject_AdjustWidth(float *arr)
{
    float aspect_mult = Wide_GetAspectMult();
    arr[0x0 / 4] *= aspect_mult;
    arr[0x8 / 4] *= aspect_mult;
}
CODEPATCH_HOOKCREATE(0x800646b8, "addi	3, 1, 80\n\t", GXProject_AdjustWidth, "", 0)
void IndicatorCam_Adjust(COBJ *c)
{
    float mult = Wide_GetAspectMult();

    c->projection_param.ortho.left *= mult;
    c->projection_param.ortho.right *= mult;
    // c->scissor_right = c->viewport_right;
}
CODEPATCH_HOOKCREATE(0x80115a48, "mr 3, 29\n\t", IndicatorCam_Adjust, "", 0)
void CObj_CheckVisibleAdjust(CamScissor *scissor)
{
    float aspect_mult = Wide_GetAspectMult();
    scissor->left *= aspect_mult;
    scissor->right *= aspect_mult;
}
CODEPATCH_HOOKCREATE(0x800674fc, "addi 3, 1, 0x8\n\t", CObj_CheckVisibleAdjust, "", 0)

void Vec_Log(char *s, Vec3 *v)
{
    OSReport("%s  X: %0.2f, Y: %0.2f, Z: %0.2f\n", 
                s,
                v->X,
                v->Y,
                v->Z);
}
void Viewport_Log(char *s, float *v)
{
    OSReport("%s  l: %0.2f, r: %0.2f, t: %0.2f, b: %0.2f\n", 
                s,
                v[0],
                v[1],
                v[2],
                v[3]);
}
void Scissor_Log(char *s, CamScissor *scissor)
{
    OSReport("%s  l: %d, r: %d, t: %d, b: %d\n", 
                s,
                scissor->left,
                scissor->right,
                scissor->top,
                scissor->bottom);
}

// mini map
void Minimap_AdjustViewport(COBJ *c)
{
    // c->viewport_left = 480;
    // c->viewport_right = 640;
    // c->scissor_left = 480;
    // c->scissor_right = 640;

    HUDAdjust_Camera(c);
}
CODEPATCH_HOOKCREATE(0x80067364, "mr 3,31\n\t", Minimap_AdjustViewport, "", 0)
CODEPATCH_HOOKCREATE(0x800672e4, "mr 3,31\n\t", Minimap_AdjustViewport, "", 0)
void MiniMapDotsCam_Adjust(COBJ *c)
{
    HUDAdjust_Camera(c);
}
CODEPATCH_HOOKCREATE(0x80115ad8, "lwz 3, 0x28 (30)\n\t", MiniMapDotsCam_Adjust, "", 0)

// plicon edges
int Plicon_GetPosition(GOBJ *g, Vec3 *pos1, Vec3 *pos2, Vec3 *out_screen_pos, Vec3 *arg4)
{
    typedef enum ScreenEdgeKind
    {
        EDGE_TOP,
        EDGE_BOTTOM,
        EDGE_LEFT,
        EDGE_RIGHT,
    } ScreenEdgeKind;

    ScreenEdgeKind (*HUD_AdjustPliconEdge)(GOBJ *g, Vec3 *pos1, Vec3 *pos2, Vec3 *out_screen_pos, Vec3 *arg4) = (void *)0x80120360;

    // this function handles scaling the plicon and rotating it based on the edge its on
    ScreenEdgeKind edge_kind = HUD_AdjustPliconEdge(g, pos1, pos2, out_screen_pos, arg4);

    // 
    HUDElementData *gp = g->userdata;

    // get camera viewport
    CamScissor viewport;
    switch (Gm_GetPlyViewNum())
    {
        case (1):
            PlyCam_GetFullscreenScissor(&viewport);
            break;
        case (2):
            PlyCam_Get2PScissor(gp->ply, &viewport);
            break;
        case (3):
        case (4):
            PlyCam_Get4PScissor(gp->ply, &viewport);
            break;
    }

    // doing gymnastics here to avoid 3 casts (2 instead rofl)
    u16 x = (u8)-1;
    u16 y = (u8)-1;
    switch (edge_kind)
    {
        case (EDGE_BOTTOM):
            y = viewport.bottom;

            // move X position based on splitscreen configuration
            //  out_screen_pos->X will be set to the correct value according to fullscreen viewport, so correct this
            //  original game hardcodes a million checks, we're taking a more dynamic approach 
            float bottom_edge_range = *(float *)0x805dfdd8;
            float width = viewport.right - viewport.left;
            float mult = ((float)width / 4) / bottom_edge_range;
            out_screen_pos->X = (float)viewport.left + (out_screen_pos->X * mult);
            
            break;

        case (EDGE_LEFT):
            x = viewport.left + 10;
            goto HEIGHT_ADJUST;
        case (EDGE_RIGHT):
            x = viewport.right - 10;
            goto HEIGHT_ADJUST;

        HEIGHT_ADJUST:
            // move Y position based on splitscreen configuration
            //  out_screen_pos->Y will be set to the correct value according to fullscreen viewport, so correct this            
            float viewport_height = viewport.bottom - viewport.top;
            out_screen_pos->Y = viewport.top + (out_screen_pos->Y / 480.f) * viewport_height;

            break;
    }

    if (x != (u8)-1)
        out_screen_pos->X = x;
    if (y != (u8)-1)
        out_screen_pos->Y = y;

    out_screen_pos->X *= Wide_GetAspectMult();

    // exit function if plicon should not be shown
    return (edge_kind == EDGE_TOP);
}
CODEPATCH_HOOKCONDITIONALCREATE(0x8012083c, "mr 3, 27\n\t"
                                 "addi 4, 1, 68\n\t"
                                 "addi 5, 1, 44\n\t"
                                 "addi 6, 1, 56\n\t"
                                 "addi 7, 1, 32\n\t",
                                 Plicon_GetPosition, "", 
                                 0x80120cb0,
                                 0x80120e50)

float PlyCam_GetAspect(CamData *cam_data)
{
    return 1;

    PlayerCamData *ply_cam = cam_data->player_cam_data;

    CamScissor viewport;
    if (ply_cam->lod == 0)  // not active?
        return stc_plycam_lookup->param->aspect_mult_1p;
    else if (ply_cam->lod == 1)
        PlyCam_GetFullscreenScissor(&viewport);
    else
        ply_viewport_get[ply_cam->lod - 2](ply_cam->ply, &viewport);

    int width = viewport.right - viewport.left;
    int height = viewport.bottom - viewport.top;
    float ratio = (float)width / (float)height;

    float aspect = (4.f/3.f) / ratio;
    return aspect;
    
}

void Wide_CreateDebugHUDGObj()
{   
    // GOBJ_EZCreator(0,0,0,
    //                 0, 0,
    //                 0, 0,
    //                 0, 0,
    //                 DebugCamera_GX, GAMEGX_RIDER, 20);

    // GOBJ_EZCreator(0,0,0,
    //                 0, 0,
    //                 0, 0,
    //                 0, 0,
    //                 DebugHUD_GX, GAMEGX_HUD, 20);

    return;
}

void Wide_AdjustConstants()
{
    float mult = Wide_GetAspectMult();
    
    // adjust hardcoded values for current aspect
    for (int i = 0; i < GetElementsIn(wide_adjust_data); i++)
        *wide_adjust_data[i].ptr = wide_adjust_data[i].orig * mult;
}

void HUDAdjust_Init()
{
    // CODEPATCH_HOOKAPPLY(0x801a06d0);

    // debug osreports on hud element creation
    // CODEPATCH_HOOKAPPLY(0x80114b8c);
    // CODEPATCH_HOOKAPPLY(0x80114cf0);
    // CODEPATCH_HOOKAPPLY(0x80114858);

    // ready go finish
    // CODEPATCH_HOOKAPPLY(0x8011ca80);

    // adjust speedometer hud
    CODEPATCH_HOOKAPPLY(0x80119c20);
    CODEPATCH_HOOKAPPLY(0x801181d4); // speedometer outer 
    CODEPATCH_HOOKAPPLY(0x80118e70);
    CODEPATCH_HOOKAPPLY(0x8011a060);
    CODEPATCH_HOOKAPPLY(0x80123e18);
    CODEPATCH_HOOKAPPLY(0x80124650); // kirby walk speedometer
    CODEPATCH_HOOKAPPLY(0x801242e4); // kirby dead speedometer

    // legendary pieces
    CODEPATCH_HOOKAPPLY(0x8012b650);

    // plynum2
    CODEPATCH_HOOKAPPLY(0x801260fc);

    // hit icon
    CODEPATCH_HOOKAPPLY(0x8012a94c);

    // stadiums
    CODEPATCH_HOOKAPPLY(0x8011a4dc); // placement number
    CODEPATCH_HOOKAPPLY(0x8012d324); // air glider hud
    CODEPATCH_HOOKAPPLY(0x8012c4dc); // high jump hud
    CODEPATCH_HOOKAPPLY(0x80130084); // target flight points
    CODEPATCH_HOOKAPPLY(0x8012fb74); // high jump points
    CODEPATCH_HOOKAPPLY(0x8012e964); // kirby melee points
    CODEPATCH_HOOKAPPLY(0x801306ec); // destruction derby points

    // air ride
    CODEPATCH_HOOKAPPLY(0x8011ae08); // lap num
    CODEPATCH_HOOKAPPLY(0x8012a350); // cpu finish
    CODEPATCH_HOOKAPPLY(0x8011bdc0); // stadium race record 1
    CODEPATCH_HOOKAPPLY(0x8011b7c0); // stadium race record 2
    
    // event compass
    CODEPATCH_HOOKAPPLY(0x80128004);

    // pause
    CODEPATCH_HOOKAPPLY(0x80128a50); // city trial
    CODEPATCH_HOOKAPPLY(0x801283c0); // air ride

    // pause stats
    CODEPATCH_HOOKAPPLY(0x80128d4c);

    // hp bar
    CODEPATCH_HOOKAPPLY(0x8011f670);

    // position model
    CODEPATCH_HOOKAPPLY(0x80125d8c);

    // minimap viewport
    CODEPATCH_HOOKAPPLY(0x80067364);
    CODEPATCH_HOOKAPPLY(0x800672e4);
    
    // minimap dots
    CODEPATCH_HOOKAPPLY(0x80115ad8);

    // indicator hud
    CODEPATCH_HOOKAPPLY(0x800646b8);
    CODEPATCH_HOOKAPPLY(0x800674fc);
    CODEPATCH_HOOKAPPLY(0x80115a48);

    // plicons
    CODEPATCH_HOOKAPPLY(0x8012083c);

    // read in original values
    for (int i = 0; i < GetElementsIn(wide_adjust_data); i++)
        wide_adjust_data[i].orig = *wide_adjust_data[i].ptr;

    // splitscreen modifications
    memcpy((void *)0x80499548, &ply_viewport_2_custom, sizeof(ply_viewport_2_custom));
    memcpy((void *)0x80499558, &ply_viewport_2_custom, sizeof(ply_viewport_2_custom));
    memcpy((void *)0x80499568, &ply_viewport_2_custom, sizeof(ply_viewport_2_custom));
    memcpy((void *)0x805d5330, &map_viewport_2_custom, sizeof(map_viewport_2_custom));  // viewport
    memcpy((void *)0x805d5338, &map_viewport_2_custom, sizeof(map_viewport_2_custom));  // scissor
    
    *((float*)0x805dfcfc) = ply_viewport_2_hud_scale;
    *((float*)0x805dfbfc) = ply_viewport_2_event_text_y;
    
    // 2p splitscreen info frame y adjustment
    CODEPATCH_HOOKAPPLY(0x801258bc);
    CODEPATCH_HOOKAPPLY(0x801192f4);
    CODEPATCH_HOOKAPPLY(0x801279ec);
    
    // override splitscreen aspect multipler
    CODEPATCH_REPLACEFUNC(0x800bbc8c, PlyCam_GetAspect);

}