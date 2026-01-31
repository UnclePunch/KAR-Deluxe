#include "text.h"
#include "os.h"
#include "hsd.h"

#include "hoshi/wide.h"
#include "wide.h"

#include "code_patch/code_patch.h"

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

// wide
void CObj_AdjustWideOrtho(COBJ *c)
{
    float adjust_ratio = Hoshi_GetWideMult();
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
// CODEPATCH_HOOKCREATE(0x80403188, "mr 3,30\n\t", CObj_AdjustWideOrtho, "", 0)

void Wide_Init()
{
    *stc_cobj_aspect *= 1.33333333333 * 2;

    // text
    CODEPATCH_HOOKAPPLY(0x8044f778);    // text cobj
    CODEPATCH_HOOKAPPLY(0x800ab218);    // develop text cobj
    
    // all ortho cobj
    // CODEPATCH_HOOKAPPLY(0x80403188); 

    // memcard cam
    CODEPATCH_REPLACECALL(0x80182ef4, COBJ_LoadDescSetAspect);
}
