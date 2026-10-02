
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

#include "motion.h"
#include "code_patch/code_patch.h"
#include "hoshi/settings.h"

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

int fov_level = 1;
int rotate_level = 1;
void Motion_ParamAdjust_Hook(float fov)
{
    cmMainParamCommon *param = stc_plycam_lookup->param;

    static float fov_mult[] = {
        0.9,
        1.0,
        1.1,
    };
    param->fov_1p = 78 * fov_mult[fov_level];
    param->fov_2p = 68 * fov_mult[fov_level];
    param->fov_4p = 80 * fov_mult[fov_level];

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

void Motion_Init()
{
    CODEPATCH_REPLACECALL(0x800b390c, Motion_Tilt_Hook);
    CODEPATCH_REPLACECALL(0x800c132c, Motion_FOVSpeed_Hook);
    CODEPATCH_REPLACECALL(0x8006dbd8, Motion_EffectShake_Hook);
    CODEPATCH_HOOKAPPLY(0x800bc420);
}
