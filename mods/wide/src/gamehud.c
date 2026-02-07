#include "text.h"
#include "os.h"
#include "hsd.h"
#include "hud.h"

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
void HUDAdjust_Element(GOBJ *g, int joint_index, int is_ply_element, WideAlign align)
{
    Game3dData *g3d = Gm_Get3dData();
    HUDElementData *gp = g->userdata;

    // if not a player view element, use fullscreen region
    int plyview_num = (is_ply_element) ? g3d->plyview_num : 1;

    // temp disable splitscreen adjustments
    if (plyview_num > 1)
        return;

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
void HUDAdjust_RightAlign(GOBJ *g)
{
    HUDAdjust_Element(g, 0, true, WIDEALIGN_RIGHT);
}
CODEPATCH_HOOKCREATE(0x80118e70, "mr 3,28\n\t", HUDAdjust_RightAlign, "", 0)
CODEPATCH_HOOKCREATE(0x801181d4, "mr 3,27\n\t", HUDAdjust_RightAlign, "", 0)
CODEPATCH_HOOKCREATE(0x80119c20, "mr 3,26\n\t", HUDAdjust_RightAlign, "", 0)
CODEPATCH_HOOKCREATE(0x8011a060, "mr 3,26\n\t", HUDAdjust_RightAlign, "", 0)
CODEPATCH_HOOKCREATE(0x80123e18, "mr 3,30\n\t", HUDAdjust_RightAlign, "", 0)
CODEPATCH_HOOKCREATE(0x80124650, "mr 3,30\n\t", HUDAdjust_RightAlign, "", 0)
CODEPATCH_HOOKCREATE(0x80128004, "mr 3,29\n\t", HUDAdjust_RightAlign, "", 0) // event compass
CODEPATCH_HOOKCREATE(0x8011ae08, "mr 3,28\n\t", HUDAdjust_RightAlign, "", 0) // air ride lap num

void HUDAdjust_LeftAlign(GOBJ *g)
{
    HUDAdjust_Element(g, 0, true, WIDEALIGN_LEFT);
}
CODEPATCH_HOOKCREATE(0x8012b650, "mr 3,30\n\t", HUDAdjust_LeftAlign, "", 0)
CODEPATCH_HOOKCREATE(0x801260fc, "mr 3,29\n\t", HUDAdjust_LeftAlign, "", 0) // plynum2
CODEPATCH_HOOKCREATE(0x8012a94c, "mr 3,31\n\t", HUDAdjust_LeftAlign, "", 0) // kirby hit
CODEPATCH_HOOKCREATE(0x8011a4dc, "mr 29,3\n\t" "mr 3,28\n\t", HUDAdjust_LeftAlign, "b 0x8\n\t", 0) // placement number
CODEPATCH_HOOKCREATE(0x80130084, "mr 3,30\n\t", HUDAdjust_LeftAlign, "", 0) // target flight points
CODEPATCH_HOOKCREATE(0x8012fb74, "mr 3,29\n\t", HUDAdjust_LeftAlign, "", 0) // high jump previous points
CODEPATCH_HOOKCREATE(0x8012e964, "mr 3,29\n\t", HUDAdjust_LeftAlign, "", 0) // kirby melee points
CODEPATCH_HOOKCREATE(0x801306ec, "mr 3,29\n\t", HUDAdjust_LeftAlign, "li 0, 0\t\n", 0) // destruction derby points
CODEPATCH_HOOKCREATE(0x8012a350, "mr 3,30\n\t", HUDAdjust_LeftAlign, "", 0) // opponent finish
CODEPATCH_HOOKCREATE(0x8011bdc0, "mr 3,29\n\t", HUDAdjust_LeftAlign, "", 0) // stadium race record 1
CODEPATCH_HOOKCREATE(0x8011b7c0, "mr 3,30\n\t", HUDAdjust_LeftAlign, "", 0) // stadium race record 2

void HUDAdjust_AirGliderHUD(int ply)
{
    Game3dData *g3d = Gm_Get3dData();

    GOBJ *score_gobj = g3d->airglider_hud.score_gobj[ply];
    GOBJ *machineicon_gobj = g3d->airglider_hud.machineicon_gobj[ply];
    
    if (score_gobj)
    {
        if (Ply_IsViewOn(ply))
            HUDAdjust_RightAlign(score_gobj);
        else
            HUDAdjust_LeftAlign(score_gobj);
    }

    if (machineicon_gobj)
        HUDAdjust_LeftAlign(machineicon_gobj);

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
            HUDAdjust_RightAlign(score_gobj);
        else
            HUDAdjust_LeftAlign(score_gobj);
    }

    if (machineicon_gobj)
        HUDAdjust_LeftAlign(machineicon_gobj);

}
CODEPATCH_HOOKCREATE(0x8012c4dc, "mr 3,26\n\t", HUDAdjust_HighJumpHUD, "", 0) // flight distance opponent

// pause
void HUDAdjust_PauseStats(GOBJ *g)
{
    HUDAdjust_Element(g, 0, true, WIDEALIGN_LEFT);
}
CODEPATCH_HOOKCREATE(0x80128fd0, "mr 3,29\n\t", HUDAdjust_PauseStats, "", 0)
CODEPATCH_HOOKCREATE(0x80129430, "mr 3,28\n\t", HUDAdjust_PauseStats, "", 0)
void HUDAdjust_CityPauseOptions(GOBJ *g)
{
    // line
    HUDAdjust_Element(g, 1, false, WIDEALIGN_LEFT);
    // options
    HUDAdjust_Element(g, 2, false, WIDEALIGN_RIGHT);
    // player indicator
    HUDAdjust_Element(g, 6, false, WIDEALIGN_LEFT);
}
CODEPATCH_HOOKCREATE(0x80128a50, "mr 3,29\n\t", HUDAdjust_CityPauseOptions, "", 0)
void HUDAdjust_AirRidePauseOptions(GOBJ *g)
{
    // pause text
    HUDAdjust_Element(g, 1, false, WIDEALIGN_LEFT);
    // options
    HUDAdjust_Element(g, 2, false, WIDEALIGN_RIGHT);
    // player indicator
    HUDAdjust_Element(g, 3, false, WIDEALIGN_LEFT);
}
CODEPATCH_HOOKCREATE(0x801283c0, "mr 3,29\n\t", HUDAdjust_AirRidePauseOptions, "", 0)
// hp bar
GOBJ *Hook_HPBarHUD(GOBJ *g)
{
    HUDAdjust_Element(g, 0, true, WIDEALIGN_RIGHT);
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
    
        // OSReport("ply %d:\n", ply);
        // OSReport(" normalized: (%.2f, %.2f)\n", normalized_center.X, normalized_center.Y);
        // OSReport(" hud_center: (%.2f, %.2f)\n", hp->ply_hud.pos[ply].X, hp->ply_hud.pos[ply].Y);
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

// mini map
void Minimap_AdjustViewport(COBJ *c)
{
    float aspect_mult = Wide_GetAspectMult();

    float center_x_normalized = ((c->viewport_right + c->viewport_left) / 2) / 640.0f;

    float edge_x = 0;
    float width = 640.f; 
    if (aspect_mult > 1.33333333333)
    {
        width = (640.f * 1.33333333333) / aspect_mult; 
        edge_x = (640.f - width) / 2;
    }

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

    float new_center_x = edge_x + (center_x_normalized * width);
    c->viewport_left = new_center_x - (adjusted_view_width / 2);
    c->viewport_right = new_center_x + (adjusted_view_width / 2);

    c->scissor_left = c->viewport_left;
    c->scissor_right = c->viewport_right;

    OSReport("center_normalized: %.2f\n", center_x_normalized);
    OSReport("left %.2f, right %.2f\n", c->viewport_left, c->viewport_right);
}
CODEPATCH_HOOKCREATE(0x80067364, "mr 3,31\n\t", Minimap_AdjustViewport, "", 0)
CODEPATCH_HOOKCREATE(0x800672e4, "mr 3,31\n\t", Minimap_AdjustViewport, "", 0)
void MiniMapDotsCam_Adjust(COBJ *c)
{
    float aspect_mult = Wide_GetAspectMult();
    c->projection_param.ortho.left *= aspect_mult;
    c->projection_param.ortho.right *= aspect_mult;
}
CODEPATCH_HOOKCREATE(0x80115ad8, "lwz 3, 0x28 (30)\n\t", MiniMapDotsCam_Adjust, "", 0)

void Wide_CreateDebugHUDGObj()
{
    return;
    
    GOBJ_EZCreator(0,0,0,
                    0, 0,
                    0, 0,
                    0, 0,
                    DebugHUD_GX, GAMEGX_HUD, 20);

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

    // adjust speedometer hud
    CODEPATCH_HOOKAPPLY(0x80119c20);
    CODEPATCH_HOOKAPPLY(0x801181d4);
    CODEPATCH_HOOKAPPLY(0x80118e70);
    CODEPATCH_HOOKAPPLY(0x8011a060);
    CODEPATCH_HOOKAPPLY(0x80123e18);
    CODEPATCH_HOOKAPPLY(0x80124650); // kirby walk speedometer

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
    CODEPATCH_HOOKAPPLY(0x80128fd0);
    CODEPATCH_HOOKAPPLY(0x80129430);

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