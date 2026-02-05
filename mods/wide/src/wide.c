#include "text.h"
#include "os.h"
#include "hsd.h"

#include "hoshi/screen_cam.h"
#include "wide.h"

#include "code_patch/code_patch.h"

extern StarpoleDolphinData *starpole_export;

WideKind wide_kind = WIDEKIND_43;
static float wide_kind_fractions[] = {
    1.255,              // 4:3
    1.255 * 1.33333,    // 16:9
    1.255 * 1.2,        // 16:10
    1.255 * 2.66666,    // 32:9
};

typedef struct
{
    float top;
    float bot;
    float left;
    float right;
} CamBounds;
void Wide_GetBounds(COBJ *cam, f32 depth, CamBounds *out) 
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
float Wide_GetOriginalWidth(COBJ *cam)
{
    float aspect;

    // handle no aspect change
    if (_fabs((*stc_cobj_aspect) - ORIG_ASPECT) < 0.001f)
        aspect = cam->projection_param.perspective.aspect;                      // aspect is unchanged
    else
        aspect = cam->projection_param.perspective.aspect / (*stc_cobj_aspect); // get original aspect

    float depth = (cam->eye->pos.Z - cam->interest->pos.Z);

    switch (cam->projection_type)
    {
        case PROJ_PERSPECTIVE:
        {
            float fov = cam->projection_param.perspective.fov;
            float fov_radians = fov * (M_PI / 180.0f);

            // Horizontal FOV derived from engine FOV
            float fov_x = 2.0f * atan(tan(fov_radians * 0.5f) * aspect);
            float width  = 2.0f * depth * tan(fov_x * 0.5f);

            return width  * 0.5f;
        }
        default:
            return 0.0f;
    }
}
float Wide_GetInverseScale()
{
    return ORIG_ASPECT / (*stc_cobj_aspect);
}

float Wide_GetAspectMult()
{
    return (*stc_cobj_aspect) / ORIG_ASPECT;
}
void Wide_OnOptionChange(int val)
{
    if (starpole_export)
        return;

    WideKind kind = (WideKind)val;

    // set global aspect ratio
    *stc_cobj_aspect = wide_kind_fractions[kind];

    // adjust main menu camera
    if (Scene_GetCurrentMinor() == MNRKIND_MAINMENU)
    {
        ScMenuCommon *md = Gm_GetMenuData();
        if (md->cam_gobj)
        {
            COBJ *c = md->cam_gobj->hsd_object;
            c->projection_param.perspective.aspect = *stc_cobj_aspect;
        }
        
        // adjust menu text camera
        COBJ *c = 0;
        int idx = 0;
        int target_canvas_idx = md->text.canvas_idx;
        for (TextCanvas *tc = *stc_textcanvas_first; tc; tc = tc->next)
        {
            if (tc->sis_idx == 0)
            {
                if (idx == target_canvas_idx)
                {
                    c = tc->cam_gobj->hsd_object;

                    c->projection_param.ortho.left = 0;
                    c->projection_param.ortho.right = 640;
                    CObj_AdjustWideOrtho(c);
                    CObj_SetMtxDirty(c);
                    break;
                }

                idx++;
            }
        }

        // adjust hoshi settings camera
        for (GOBJ *g = (*stc_gobj_lookup)[17]; g; g = g->next)
        {
            if (g->obj_kind == HSD_OBJKIND_COBJ && g->cobj_links == (1ULL << 4))
            {
                COBJ *c = g->hsd_object;
                c->projection_param.perspective.aspect = *stc_cobj_aspect;
            }
        }
    }
}

// wide
void CObj_AdjustWideOrtho(COBJ *c)
{
    float adjust_ratio = Wide_GetAspectMult();
    float orig_width = c->projection_param.ortho.right;

    // widen bounds
    c->projection_param.ortho.left *= adjust_ratio;
    c->projection_param.ortho.right *= adjust_ratio;

    // shift eye and interest to center 0->640 (compatibility with original game code)
    float shift_amt = -((c->projection_param.ortho.right + c->projection_param.ortho.left) - orig_width) / 2;
    c->eye->pos.X = shift_amt;
    c->interest->pos.X = shift_amt;
}
CODEPATCH_HOOKCREATE(0x8044f778, "mr 3,28\n\t", CObj_AdjustWideOrtho, "", 0)
CODEPATCH_HOOKCREATE(0x800ab218, "lwz 3, 0x28(26)\n\t", CObj_AdjustWideOrtho, "", 0)

void MovieCObj_AdjustWide(GOBJ *g)
{
    CObj_AdjustWideOrtho((COBJ*)g->hsd_object);
}
CODEPATCH_HOOKCREATE(0x8000d454, "lwz 3, 0x470 (13)\n\t", MovieCObj_AdjustWide, "", 0)
CODEPATCH_HOOKCREATE(0x8000d87c, "lwz 3, 0x480 (13)\n\t", MovieCObj_AdjustWide, "", 0)
CODEPATCH_HOOKCREATE(0x80049608, "lwz 3, 0x49c (13)\n\t", MovieCObj_AdjustWide, "", 0)
CODEPATCH_HOOKCREATE(0x8004950c, "lwz 3, 0x49c (13)\n\t", MovieCObj_AdjustWide, "", 0)
CODEPATCH_HOOKCREATE(0x80049704, "lwz 3, 0x49c (13)\n\t", MovieCObj_AdjustWide, "", 0)
CODEPATCH_HOOKCREATE(0x80049800, "lwz 3, 0x49c (13)\n\t", MovieCObj_AdjustWide, "", 0)
CODEPATCH_HOOKCREATE(0x800498fc, "lwz 3, 0x49c (13)\n\t", MovieCObj_AdjustWide, "", 0)
CODEPATCH_HOOKCREATE(0x800499f8, "lwz 3, 0x49c (13)\n\t", MovieCObj_AdjustWide, "", 0)

// pillarbox
void Wide_PillarboxGX(GOBJ *g, int pass)
{
    // idea, add a transparent plane on screen edges w/ zupdate enabled in between background and foreground objects for menus.
    // this should prevent foreground objects from being drawn to the edge pixels.
    // combine this with setting the erase color to fill in the background color

    if (pass != 2)
        return;

    float mult = Wide_GetAspectMult();
    float width_4_3 = 640.f / mult; 

    static GXColor edge_color = (GXColor){0, 0, 0, 255};
    GX_DrawRect(&(Vec3){0, 480.f, 0}, 
                &(Vec3){0 + ((640.f - width_4_3) / 2.f), 0, 0}, 
                &edge_color);

    GX_DrawRect(&(Vec3){640.f - ((640.f - width_4_3) / 2.f), 480.f, 0}, 
                &(Vec3){640.f, 0, 0}, 
                &edge_color);

    // float edge_x = 0;
    // if (mult > 1.33333333333)
    // {
    //     float width = (640.f * 1.33333333333) / mult; 
    //     edge_x = (640.f - width) / 2;
    // }
    // GX_DrawLine(&(Vec3){edge_x, 0, 0}, 
    //             &(Vec3){edge_x + 20, 0, 0},
    //             10, 
    //             &(GXColor){128, 255, 128, 255});
}

COBJDesc desc = {
    .class_name = 0,
    .flags = 0,
    .projection_type = PROJ_ORTHO,
    .viewport_left = 0,
    .viewport_right = 640,
    .viewport_top = 0,
    .viewport_bottom = 480,
    .scissor_lr = ((u16)0 << 16) | (u16)640,
    .scissor_tb = ((u16)0 << 16) | (u16)480,
    .eye_desc = &(WOBJDesc){
        .class_name = 0,
        .next = 0,
        .pos = (Vec3){0, 0, 300},
        .robjdesc = 0,
    },
    .interest_desc = &(WOBJDesc){
        .class_name = 0,
        .next = 0,
        .pos = (Vec3){0, 0, 0},
        .robjdesc = 0,
    },
    .roll = 0,
    .vector = 0,
    .near = 0.1f,
    .far = 3277777.f,
    .projection_param.ortho.left = 0,
    .projection_param.ortho.right = 640,
    .projection_param.ortho.top = 0,
    .projection_param.ortho.bottom = 480,
};
void Wide_CreateTestGObj()
{
    MinorKind minor_kind = Scene_GetCurrentMinor();
    if (minor_kind == MNRKIND_3D)
        return;

    int gx_link = HOSHI_SCREENCAM_GXLINK - 1;

    GOBJ *g = GOBJ_EZCreator(0,0,0,
                    0, 0,
                    HSD_OBJKIND_COBJ, &desc,
                    0, 0,
                    CObjThink_Common, 0, 63);
    g->cobj_links = ((1ULL) << gx_link);

    GOBJ_EZCreator(0,0,0,
                    0, 0,
                    0, 0,
                    0, 0,
                    Wide_PillarboxGX, gx_link, 20);
}

void Wide_Init()
{
    // text
    CODEPATCH_HOOKAPPLY(0x8044f778);    // text cobj
    CODEPATCH_HOOKAPPLY(0x800ab218);    // develop text cobj

    // movie cams
    CODEPATCH_HOOKAPPLY(0x8000d454);
    CODEPATCH_HOOKAPPLY(0x8000d87c);
    CODEPATCH_HOOKAPPLY(0x80049608);
    CODEPATCH_HOOKAPPLY(0x8004950c);
    CODEPATCH_HOOKAPPLY(0x80049704);
    CODEPATCH_HOOKAPPLY(0x80049800);
    CODEPATCH_HOOKAPPLY(0x800498fc);
    CODEPATCH_HOOKAPPLY(0x800499f8);
    
    // all ortho cobj
    // CODEPATCH_HOOKAPPLY(0x80403188); 

    // memcard cam
    CODEPATCH_REPLACECALL(0x80182ef4, COBJ_LoadDescSetAspect);
}
