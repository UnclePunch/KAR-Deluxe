
#include "text.h"
#include "hud.h"
#include "os.h"
#include "camera.h"
#include "hsd.h"
#include "preload.h"
#include "scene.h"
#include "inline.h"
#include "audio.h"
#include "obj.h"
#include "game.h"

#include "mirror_mode.h"

#include "code_patch/code_patch.h"

int g_is_mirror = 0;

int g_mirror_flags;

// culling hook
int GXSetCullMode_Invert(GXCullMode cull)
{
    if (g_mirror_flags & MIRRORFLAG_CULL)
    {
        if (cull == GX_CULL_FRONT)
            cull = GX_CULL_BACK;
        else if (cull == GX_CULL_BACK)
            cull = GX_CULL_FRONT;
    }

    return cull;
}
CODEPATCH_HOOKCREATE(0x803cc518, "stwu	1, -40 (1)\n\t"
                                 "mflr 0\n\t"
                                 "stw 0, 44 (1)\n\t",
                                 GXSetCullMode_Invert, 
                                 "lwz 0, 44 (1)\n\t"
                                 "mtlr 0\n\t"
                                 "addi 1, 1, 40\n\t",
                                 0)

// GXSetProjection hook
extern void GXSetProjection_Orig(Mtx44 mtx, GXProjectionType type);
__asm__(
    ".section .text\n"
    ".balign 4\n"
    ".globl GXSetProjection_Orig\n"
    ".type  GXSetProjection_Orig, @function\n"
    "GXSetProjection_Orig:\n"
    "    lwz    5, -0x0F90 (2)\n"      // the original first instruction
    "    b    GXSetProjection+4\n"
    ".size GXSetProjection_Orig, .-GXSetProjection_Orig\n"
);
void GXSetProjection_Hook(Mtx44 mtx, GXProjectionType type)
{
    if (g_mirror_flags & MIRRORFLAG_GXSETPROJECTION) 
    {
        Mtx44 m;
        memcpy(m, mtx, sizeof(Mtx44));
        m[0][0] = -m[0][0];
        m[0][1] = -m[0][1];
        m[0][2] = -m[0][2];
        m[0][3] = -m[0][3];
        GXSetProjection_Orig(m, type);
        return;
    }
    GXSetProjection_Orig(mtx, type);
}

// GXProject hook
extern void GXProject_Orig(float mx, float my, float mz, Mtx *mtx, float* p, float* vp,  float* sx, float* sy, float* sz);
__asm__(
    ".section .text\n"
    ".balign 4\n"
    ".globl GXProject_Orig\n"
    ".type  GXProject_Orig, @function\n"
    "GXProject_Orig:\n"
    "    lfs	4, 0 (3)\n"      // the original first instruction
    "    b    GXProject+4\n"
    ".size GXProject_Orig, .-GXProject_Orig\n"
);
void GXProject_Hook(float mx, float my, float mz, Mtx *mtx, float* p, float* vp,  float* sx, float* sy, float* sz)
{
    GXProject_Orig(mx, my, mz, mtx, p, vp, sx, sy, sz);
    if (g_mirror_flags & MIRRORFLAG_GXPROJECT)
        *sx = (2.0f * vp[0] + vp[2]) - *sx;   // vp[0] = x origin, vp[2] = width
}

// Plicon edge hook
extern HUDScreenEdgeKind HUD_AdjustPliconEdge_Orig(GOBJ *g, Vec3 *pos1, Vec3 *pos2, Vec3 *out_screen_pos, Vec3 *arg4);
__asm__(
    ".section .text\n"
    ".balign 4\n"
    ".globl HUD_AdjustPliconEdge_Orig\n"
    ".type  HUD_AdjustPliconEdge_Orig, @function\n"
    "HUD_AdjustPliconEdge_Orig:\n"
    "    stwu	1, -0x0060 (1)\n"      // the original first instruction
    "    b    HUD_AdjustPliconEdge+4\n"
    ".size HUD_AdjustPliconEdge_Orig, .-HUD_AdjustPliconEdge_Orig\n"
);
HUDScreenEdgeKind HUD_AdjustPliconEdge_Hook(GOBJ *g, Vec3 *pos1, Vec3 *pos2, Vec3 *out_screen_pos, Vec3 *arg4)
{
    // run orig
    HUDScreenEdgeKind edge = HUD_AdjustPliconEdge_Orig(g, pos1, pos2, out_screen_pos, arg4);

    // check if mirror mode is enabled
    if (g_mirror_flags & MIRRORFLAG_GXPROJECT)
    {
        if (edge == SCREENEDGE_LEFT || edge == SCREENEDGE_RIGHT)
        {
            // invert rotation
            JOBJ *j = g->hsd_object;
            j->rot.Z *= -1;

            // invert edge
            if (edge == SCREENEDGE_LEFT)
                edge = SCREENEDGE_RIGHT;
            else if (edge == SCREENEDGE_RIGHT)
                edge = SCREENEDGE_LEFT;
        }
        if (edge == SCREENEDGE_BOTTOM)
        {
            // mirror X position across center

            // // function calculates using a uniform width and adjusts for splitscreen after
            // bp();
            // float bottom_edge_range = *(float *)0x805dfdd8;
            // out_screen_pos->X = bottom_edge_range/2) + bottom_edge_range  - out_screen_pos->X;
        }
    }

    return edge;
}

int MirrorMode_Set(int flags)
{
    if (!g_is_mirror)
        return 0;

    int prev = g_mirror_flags;

    g_mirror_flags = flags;
    HSD_StateInvalidate(-1);

    return prev;
}

// 3D Cam: invert everything
void GameCam_SetCurrentCOBJ(COBJ *c)
{
    MirrorMode_Set(MIRRORFLAG_CULL | MIRRORFLAG_GXPROJECT | MIRRORFLAG_GXSETPROJECTION);
    CObj_SetCurrent(c);
}
void GameCam_EndCurrentCOBJ()
{
    MirrorMode_Set(0);
    CObj_EndCurrent();
}

// Indicator HUD: invert GXProject calls only
void IndicatorHUD_CObjThink(GOBJ *g)
{
    int (*Debug_GetIsDrawHUD)() = (void *)0x80112b20;

    if (!Debug_GetIsDrawHUD())
        return;

    COBJ *c = g->hsd_object;

    int prev = MirrorMode_Set(MIRRORFLAG_GXPROJECT);

    CObj_SetCurrent(c);
    CObj_RenderGXLinks(g, 7);
    CObj_EndCurrent();

    MirrorMode_Set(prev);
}
void IndicatorHUD_InitCObj(GOBJ *g, void *cb, int gx_pri)
{
    GOBJ_InitCamera(g, IndicatorHUD_CObjThink, gx_pri);
}

// Map Dots HUD: invert GXProject calls only
void HUDMapDots_CObjThink(GOBJ *g)
{
    int (*Debug_GetIsDrawHUD)() = (void *)0x80112b20;

    if (!Debug_GetIsDrawHUD())
        return;

    COBJ *c = g->hsd_object;

    int prev = MirrorMode_Set(MIRRORFLAG_GXPROJECT);

    CObj_SetCurrent(c);
    CObj_RenderGXLinks(g, 7);
    CObj_EndCurrent();

    MirrorMode_Set(prev);
}
void HUDMapDots_InitCObj(GOBJ *g, void *cb, int gx_pri)
{
    GOBJ_InitCamera(g, HUDMapDots_CObjThink, gx_pri);
}

// Map HUD: invert everything
void HUDMap_CObjThink(GOBJ *g)
{
    int (*Debug_GetIsDrawHUD)() = (void *)0x80112b20;

    if (!Debug_GetIsDrawHUD())
        return;

    COBJ *c = g->hsd_object;

    int prev = MirrorMode_Set(MIRRORFLAG_CULL | MIRRORFLAG_GXPROJECT | MIRRORFLAG_GXSETPROJECTION);

    CObj_SetCurrent(c);
    CObj_RenderGXLinks(g, 7);
    CObj_EndCurrent();

    MirrorMode_Set(prev);
}
void HUDMap_InitCObj(GOBJ *g, void *cb, int gx_pri)
{
    GOBJ_InitCamera(g, HUDMap_CObjThink, gx_pri);
}

void Rider_InvertControls(RiderData *rd)
{
    if (!g_is_mirror)
        return;

    rd->input.stickX *= -1;
    rd->input.rstick.X *= -1;
}
CODEPATCH_HOOKCREATE(0x8018f0cc, "mr 3, 31\n\t", Rider_InvertControls, "", 0)

void MirrorMode_Init()
{
    g_mirror_flags = 0;

    CODEPATCH_HOOKAPPLY(0x803cc518);                                // invert culling when mirror enabled
    CODEPATCH_REPLACEFUNC(GXSetProjection, GXSetProjection_Hook);   // invert projection matrix when mirror enabled
    CODEPATCH_REPLACEFUNC(GXProject, GXProject_Hook);               // invert projection matrix when mirror enabled
    
    // main 3d cam
    CODEPATCH_REPLACECALL(0x800b05c0, GameCam_SetCurrentCOBJ);
    CODEPATCH_REPLACECALL(0x800b07a0, GameCam_EndCurrentCOBJ);
    
    // onscreen ortho hud cam GXProject calls
    CODEPATCH_REPLACECALL(0x80115a28, IndicatorHUD_InitCObj);
    
    // minimap cam
    CODEPATCH_REPLACECALL(0x80116730, HUDMap_InitCObj);

    // minimap dots cam
    CODEPATCH_REPLACECALL(0x80115ac4, HUDMapDots_InitCObj);

    // plicon indicator mirror
    CODEPATCH_REPLACEFUNC(HUD_AdjustPliconEdge, HUD_AdjustPliconEdge_Hook);

    // invert controls
    CODEPATCH_HOOKAPPLY(0x8018f0cc);
}
