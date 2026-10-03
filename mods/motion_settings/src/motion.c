
#include "text.h"
#include "os.h"
#include "camera.h"
#include "hsd.h"
#include "preload.h"
#include "scene.h"
#include "inline.h"
#include "audio.h"
#include "obj.h"
#include "game.h"

#include "hud.h"

#include "motion.h"
#include "code_patch/code_patch.h"
#include "hoshi/settings.h"

#include "wide/wide.h"

extern WideExport *wide_export;

int tilt_disabled = 0;
void Motion_Tilt_Hook(COBJ *c, Vec3 *up)
{
    if (tilt_disabled)
        *up = (Vec3){.X = 0, .Y = 1, .Z = 0};
    
    CObj_SetUp(c, up);
}

int shake_disabled = 0;
void Motion_EffectShake_Hook(void *effect)
{
    void (*Effect_ApplyCameraShake)(void *effect) = (void *)0x8006d690;
    
    if (!shake_disabled)
        Effect_ApplyCameraShake(effect);
}

int fovspeed_disabled = 0;
void Motion_FOVSpeed_Hook(CamData *cam_data, int r4, void *r5, float *out_fov, float f1, float f2)
{
    void (*PlyCam_GetFOVChangeFromSpeed)(CamData *cam_data, int r4, void *r5, float *out_fov, float f1, float f2) = (void *)0x800bf028;
    PlyCam_GetFOVChangeFromSpeed(cam_data, r4, r5, out_fov, f1, f2);

    if (fovspeed_disabled)
        *out_fov = 0;

    if (shake_disabled)
        cam_data->x2c8 = 0;
        
}


static float g_fov_cache[3];
static int g_is_fov_cached = 0;

int fov_level = 1;
int rotate_level = 2;
void Motion_ParamAdjust_Hook(float fov)
{
    cmMainParamCommon *param = stc_plycam_lookup->param;

    static float fov_mult[] = {
        0.9,
        1.0,
        1.1,
    };

    float *fov_arr = (float *)&param->fov_1p;

    // init fov values
    if (!g_is_fov_cached)
    {
        for (int i = 0; i < 3; i++)
            g_fov_cache[i] = fov_arr[i];
        
        g_is_fov_cached = 1;
    }

    for (int i = 0; i < 3; i++)
        fov_arr[i] = g_fov_cache[i] * fov_mult[fov_level];

    static float rotate_mult[] = {
        0.25,
        0.5,
        1.0,
        1.5,
        2,
    };
    param->x334 = 7 * rotate_mult[rotate_level];

    return;
}
CODEPATCH_HOOKCREATE(0x800bc420, "", Motion_ParamAdjust_Hook, "", 0)

int border_enabled = 0;
GOBJ *Motion_BorderCreate()
{   
    if (!border_enabled || Gm_GetPlyViewNum() != 1)
        return 0;

    GOBJ *g = GOBJ_EZCreator(27, GAMEPLINK_HUD, 0,
                             0, 0,
                             HSD_OBJKIND_NONE, 0,
                             0, 0,
                             Motion_BorderGX, GAMEGX_HUD, 10);

    return g;
}
void Motion_BorderGX(GOBJ *g, int pass)
{
    if (*g_hud_is_hidden)
        return;

    #define OUTER_BOX_MULT 0.9
    #define INNER_BOX_MULT 0.75

    float horiz_edge = ORIG_WIDTH / 2;
    float vert_edge = ORIG_HEIGHT / 2;

    if (wide_export)
    {
        float aspect_mult = wide_export->Wide_GetAspectMult();
        
        // limit range to that of 16:9
        if (aspect_mult > 1.3333)
            aspect_mult = 1.3333;
        
        horiz_edge *= aspect_mult;
    }

    int width = 50;
    static GXColor color = {255, 0, 0, 128};
    Vec3 outer_top_right =   { horiz_edge * OUTER_BOX_MULT,  vert_edge * OUTER_BOX_MULT, 0};
    Vec3 outer_bottom_left = {-horiz_edge * OUTER_BOX_MULT, -vert_edge * OUTER_BOX_MULT, 0};
    Vec3 inner_top_right =   { horiz_edge * INNER_BOX_MULT,  vert_edge * INNER_BOX_MULT, 0};
    Vec3 inner_bottom_left = {-horiz_edge * INNER_BOX_MULT, -vert_edge * INNER_BOX_MULT, 0};

    GXSetZMode(GX_ENABLE, GX_ALWAYS, GX_DISABLE);

    // draw boxes
    Motion_DrawBoxWithLines(&outer_top_right, &outer_bottom_left, width, &color);
    Motion_DrawBoxWithLines(&inner_top_right, &inner_bottom_left, width, &color);

    // draw lines on edges
    Motion_DrawLine(-horiz_edge, 0, inner_bottom_left.X + 1, 0, width, &color);
    Motion_DrawLine( horiz_edge, 0, inner_top_right.X - 1, 0, width, &color);
    Motion_DrawLine(0, -vert_edge, 0, inner_bottom_left.Y + 1, width, &color);
    Motion_DrawLine(0,  vert_edge, 0, inner_top_right.Y - 1, width, &color);
}
void Motion_DrawBoxWithLines(Vec3 *tr, Vec3 *bl, int width, GXColor *color)
{
    HSD_StateInitDirect(GX_VTXFMT0, 2);
    GXLoadPosMtxImm(&COBJ_GetCurrent()->view_mtx, GX_PNMTX0);
    GXSetLineWidth(width, 5);
    GXBegin(GX_LINES, GX_VTXFMT0, 8);

    // top
    GXPosition3f32(bl->X, tr->Y, 0);
    GXColor4u8(color->r, color->g, color->b, color->a);
    GXPosition3f32(tr->X, tr->Y, 0);
    GXColor4u8(color->r, color->g, color->b, color->a);

    // right
    GXPosition3f32(tr->X, tr->Y, 0);
    GXColor4u8(color->r, color->g, color->b, color->a);
    GXPosition3f32(tr->X, bl->Y, 0);
    GXColor4u8(color->r, color->g, color->b, color->a);

    // bottom
    GXPosition3f32(tr->X, bl->Y, 0);
    GXColor4u8(color->r, color->g, color->b, color->a);
    GXPosition3f32(bl->X, bl->Y, 0);
    GXColor4u8(color->r, color->g, color->b, color->a);

    // left
    GXPosition3f32(bl->X, bl->Y, 0);
    GXColor4u8(color->r, color->g, color->b, color->a);
    GXPosition3f32(bl->X, tr->Y, 0);
    GXColor4u8(color->r, color->g, color->b, color->a);

    HSD_StateInvalidate(-1);
}
void Motion_DrawLine(float x1, float y1, float x2, float y2, int width, GXColor *color)
{
    HSD_StateInitDirect(GX_VTXFMT0, 2);
    GXLoadPosMtxImm(&COBJ_GetCurrent()->view_mtx, GX_PNMTX0);
    GXSetLineWidth(width, 5);
    GXBegin(GX_LINES, GX_VTXFMT0, 2);

    // draw line
    GXPosition3f32(x1, y1, 0);
    GXColor4u8(color->r, color->g, color->b, color->a);
    GXPosition3f32(x2, y2, 0);
    GXColor4u8(color->r, color->g, color->b, color->a);

    HSD_StateInvalidate(-1);
}

void Motion_Init()
{
    CODEPATCH_REPLACECALL(0x800b390c, Motion_Tilt_Hook);
    CODEPATCH_REPLACECALL(0x800c132c, Motion_FOVSpeed_Hook);
    CODEPATCH_REPLACECALL(0x8006dbd8, Motion_EffectShake_Hook);
    CODEPATCH_HOOKAPPLY(0x800bc420);
}
