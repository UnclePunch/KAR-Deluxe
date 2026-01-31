#include "text.h"
#include "os.h"
#include "hsd.h"
#include "hud.h"

#include "hoshi/wide.h"
#include "wide.h"
#include "gamehud.h"

#include "code_patch/code_patch.h"

typedef struct
{
    float top;
    float bot;
    float left;
    float right;
} CamBounds;
void COBJ_GetBounds(COBJ *cam, f32 depth, CamBounds *out) 
{
    depth -= (cam->eye->pos.Z - cam->interest->pos.Z);

    switch (cam->projection_type)
    {
        /* -------------------- */
        /* Perspective camera   */
        /* -------------------- */
        case 1:
        {
            float fov = cam->projection_param.perspective.fov;
            float fov_radians = fov * (M_PI / 180.0f);

            float aspect = cam->projection_param.perspective.aspect;

            // Horizontal FOV derived from engine FOV
            float fov_x = 2.0f * atan(tan(fov_radians * 0.5f) * aspect);

            float width  = 2.0f * depth * tan(fov_x * 0.5f);
            float height = width / aspect;

            out->left  = -width  * 0.5f;
            out->right =  width  * 0.5f;
            out->bot   = -height * 0.5f;
            out->top   =  height * 0.5f;
            break;
        }

        /* -------------------- */
        /* Custom frustum       */
        /* -------------------- */
        case 2:
        {
            // Frustum params are usually specified at near plane
            float near = cam->near;
            float scale = depth / near;

            out->left   = cam->projection_param.frustrum.left   * scale;
            out->right  = cam->projection_param.frustrum.right  * scale;
            out->bot    = cam->projection_param.frustrum.bottom * scale;
            out->top    = cam->projection_param.frustrum.top    * scale;
            break;
        }

        /* -------------------- */
        /* Orthographic camera  */
        /* -------------------- */
        case 3:
        {
            out->left   = cam->projection_param.ortho.left;
            out->right  = cam->projection_param.ortho.right;
            out->bot = cam->projection_param.ortho.bottom;
            out->top    = cam->projection_param.ortho.top;
            break;
        }
    }
}
void DebugHUD_GX(GOBJ *g, int pass)
{
    if (pass != 2)
        return;

    COBJ *c = COBJ_GetCurrent();
    CamBounds bounds;

    // COBJ_GetBounds(c, 0, &bounds);
    // GX_DrawLine(&(Vec3){bounds.left, 0, 0}, 
    //             &(Vec3){bounds.right, 0, 0},
    //             10, &(GXColor){255,0,0,255});
                
    // GX_DrawLine(&(Vec3){0, bounds.top, 0}, 
    //             &(Vec3){0, bounds.bot, 0},
    //             10, &(GXColor){0,255,0,255});

    // OSReport("u: %.2f  d: %.2f  l: %.2f  r: %.2f\n", 
    //             bounds.top, bounds.bot, bounds.left, bounds.right);

    Game3dData *g3d = Gm_Get3dData();
    if (g3d->plyview_num > 1)
    { 
        HUDElementData *gp = g3d->plyview_pos_gobj->userdata;
        for (int i = 0; i < Gm_GetPlyViewNum(); i++)
        {
            Vec3 *pos = &gp->ply_hud.pos[i];
            GX_DrawLine(&(Vec3){pos->X - 2, pos->Y, 0}, 
                        &(Vec3){pos->X + 2, pos->Y, 0},  
                        10, &(GXColor){255, 0, 0, 255});
            GX_DrawLine(&(Vec3){pos->X, pos->Y - 2, 0}, 
                        &(Vec3){pos->X, pos->Y + 2, 0},  
                        10, &(GXColor){255, 0, 0, 255});
        }
    }
}

GOBJ *HUDMain_Create(GOBJ *g)
{
    OSReport("created main hud element with p_link %d, gx_link %d\n", g->p_link, g->gx_link);
    return g;
}
CODEPATCH_HOOKCREATE(0x80114b8c, "mr 3,31\n\t", HUDMain_Create, "", 0)
GOBJ *HUDUnk_Create(GOBJ *g)
{
    OSReport("created unk hud element with p_link %d, gx_link %d\n", g->p_link, g->gx_link);
    return g;
}
CODEPATCH_HOOKCREATE(0x80114cf0, "mr 3,30\n\t", HUDUnk_Create, "", 0)
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

CamScissor ply_viewport_1 = {
    .left = 0,
    .right = 640,
    .top = 0,
    .bottom = 480,
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
void HUDAdjust_Element(GOBJ *g, int joint_index, WideAlign align)
{
    Game3dData *g3d = Gm_Get3dData();
    HUDElementData *gp = g->userdata;

    CamScissor viewport;
    CamScissor *orig_viewport;
    switch (g3d->plyview_num)
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

    float dir = (align == WIDEALIGN_LEFT) ? -1 : 1;
    
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

// speedometer
void HUDAdjust_Speedometer(GOBJ *g)
{
    HUDAdjust_Element(g, 0, WIDEALIGN_RIGHT);
}
CODEPATCH_HOOKCREATE(0x80118e70, "mr 3,28\n\t", HUDAdjust_Speedometer, "", 0)
CODEPATCH_HOOKCREATE(0x801181d4, "mr 3,27\n\t", HUDAdjust_Speedometer, "", 0)
CODEPATCH_HOOKCREATE(0x80119c20, "mr 3,26\n\t", HUDAdjust_Speedometer, "", 0)
CODEPATCH_HOOKCREATE(0x8011a060, "mr 3,26\n\t", HUDAdjust_Speedometer, "", 0)
CODEPATCH_HOOKCREATE(0x80123e18, "mr 3,30\n\t", HUDAdjust_Speedometer, "", 0)
CODEPATCH_HOOKCREATE(0x80124650, "mr 3,30\n\t", HUDAdjust_Speedometer, "", 0)

// pause
void HUDAdjust_PauseStats(GOBJ *g)
{
    HUDAdjust_Element(g, 0, WIDEALIGN_LEFT);
}
CODEPATCH_HOOKCREATE(0x80128fd0, "mr 3,29\n\t", HUDAdjust_PauseStats, "", 0)
CODEPATCH_HOOKCREATE(0x80129430, "mr 3,28\n\t", HUDAdjust_PauseStats, "", 0)
void HUDAdjust_PauseOptions(GOBJ *g)
{
    // line
    HUDAdjust_Element(g, 1, WIDEALIGN_LEFT);
    // options
    HUDAdjust_Element(g, 2, WIDEALIGN_RIGHT);
    // player indicator
    HUDAdjust_Element(g, 6, WIDEALIGN_LEFT);
}
CODEPATCH_HOOKCREATE(0x80128a50, "mr 3,29\n\t", HUDAdjust_PauseOptions, "", 0)

// hp bar
GOBJ *Hook_HPBarHUD(GOBJ *g)
{
    HUDAdjust_Element(g, 0, WIDEALIGN_RIGHT);
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
    
        OSReport("ply %d:\n", ply);
        OSReport(" normalized: (%.2f, %.2f)\n", normalized_center.X, normalized_center.Y);
        OSReport(" hud_center: (%.2f, %.2f)\n", hp->ply_hud.pos[ply].X, hp->ply_hud.pos[ply].Y);
    }
}
CODEPATCH_HOOKCREATE(0x80125d8c, "mr 3,28\n\t", HUDAdjust_PositionModel, "", 0)

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
        .ptr = (float *)0x805dfdd8, // bottom left pixel?
    },
    {
        .ptr = (float *)0x805dfdd4, // bottom right pixel?
    },
};

// indicator HUD
void GXProject_AdjustWidth(float *arr)
{
    arr[0x8 / 4] *= Hoshi_GetWideMult();
}
CODEPATCH_HOOKCREATE(0x800646b8, "addi	3, 1, 80\n\t", GXProject_AdjustWidth, "", 0)
void IndicatorCam_Adjust(COBJ *c)
{
    float mult = Hoshi_GetWideMult();

    c->projection_param.ortho.right *= mult;
    // c->scissor_right = c->viewport_right;

    // adjust hardcoded values for current aspect
    for (int i = 0; i < GetElementsIn(wide_adjust_data); i++)
        *wide_adjust_data[i].ptr = wide_adjust_data[i].orig * mult;
}
CODEPATCH_HOOKCREATE(0x80115a48, "mr 3, 29\n\t", IndicatorCam_Adjust, "", 0)
void CObj_CheckVisibleAdjust(CamScissor *scissor)
{
    scissor->right *= Hoshi_GetWideMult();
}
CODEPATCH_HOOKCREATE(0x800674fc, "addi 3, 1, 0x8\n\t", CObj_CheckVisibleAdjust, "", 0)

// mini map
void Minimap_AdjustViewport(COBJ *c)
{
    float aspect_mult = Hoshi_GetWideMult();

    float center_x_normalized = ((c->viewport_right + c->viewport_left) / 2) / 640.0f;

    // // limit to 16:9 position
    // if (aspect_mult > 1.3333)
    // {
    //     float adjusted_fb_width = (640.0f / 1.3333);
    //     float new_center_x = center_x_normalized * adjusted_fb_width;
    //     (aspect_mult / 1.3333) * 
    //     float pixels_from_center = new_center_x - (640.0f / 2);
    //     center_x_normalized = new_center_x / 640.0f;
    // }

    float view_width = c->viewport_right - c->viewport_left;
    
    float adjusted_fb_width = (640.0f / aspect_mult);       // shrinks with wider AR (preserves pixel width when display stretches the image)
    float adjusted_view_width = (view_width / aspect_mult); // shrinks with wider AR (preserves pixel width when display stretches the image)

    float new_center_x = center_x_normalized * adjusted_fb_width;
    c->viewport_left = new_center_x - (adjusted_view_width / 2);
    c->viewport_right = new_center_x + (adjusted_view_width / 2);

    c->scissor_left = c->viewport_left;
    c->scissor_right = c->viewport_right;
}
CODEPATCH_HOOKCREATE(0x80067364, "mr 3,31\n\t", Minimap_AdjustViewport, "", 0)
void MiniMapDotsCam_Adjust(COBJ *c)
{
    float mult = Hoshi_GetWideMult();

    c->projection_param.ortho.right *= mult;
}
CODEPATCH_HOOKCREATE(0x80115ad8, "lwz 3, 0x28 (30)\n\t", MiniMapDotsCam_Adjust, "", 0)

void Wide_CreateDebugHUDGObj()
{
    GOBJ_EZCreator(0,0,0,
                    0, 0,
                    0, 0,
                    0, 0,
                    DebugHUD_GX, GAMEGX_HUD, 20);

    return;
}

void HUDAdjust_Init()
{
    // CODEPATCH_HOOKAPPLY(0x801a06d0);

    CODEPATCH_HOOKAPPLY(0x80114b8c);
    CODEPATCH_HOOKAPPLY(0x80114cf0);
    CODEPATCH_HOOKAPPLY(0x80114858);

    // adjust speedometer hud
    CODEPATCH_HOOKAPPLY(0x80119c20);
    CODEPATCH_HOOKAPPLY(0x801181d4);
    CODEPATCH_HOOKAPPLY(0x80118e70);
    CODEPATCH_HOOKAPPLY(0x8011a060);
    CODEPATCH_HOOKAPPLY(0x80123e18);
    CODEPATCH_HOOKAPPLY(0x80124650); // kirby walk speedometer

    // pause
    CODEPATCH_HOOKAPPLY(0x80128a50);

    // pause stats
    CODEPATCH_HOOKAPPLY(0x80128fd0);
    CODEPATCH_HOOKAPPLY(0x80129430);

    // hp bar
    CODEPATCH_HOOKAPPLY(0x8011f670);

    // position model
    CODEPATCH_HOOKAPPLY(0x80125d8c);

    // indicator hud
    CODEPATCH_HOOKAPPLY(0x80067364);
    
    CODEPATCH_HOOKAPPLY(0x80115ad8);
    CODEPATCH_HOOKAPPLY(0x800646b8);
    CODEPATCH_HOOKAPPLY(0x800674fc);
    CODEPATCH_HOOKAPPLY(0x80115a48);

    // read in original values
    for (int i = 0; i < GetElementsIn(wide_adjust_data); i++)
        wide_adjust_data[i].orig = *wide_adjust_data[i].ptr;

    static CamScissor viewport[] = {
        // p1
        {
            .left = 4,
            .right = 318,
            .top = 20,
            .bottom = 460,
        },
        // p2
        {
            .left = 322,
            .right = 636,
            .top = 20,
            .bottom = 460,
        },
    };
    // memcpy((void *)0x80499548, &viewport, sizeof(viewport));
    // memcpy((void *)0x80499558, &viewport, sizeof(viewport));
}