
#include "text.h"
#include "os.h"
#include "hsd.h"
#include "preload.h"
#include "scene.h"
#include "inline.h"
#include "audio.h"
#include "obj.h"
#include "game.h"

#include "code_patch/code_patch.h"

RiderData *PlyCam_GetRiderData(CamData *cd)
{
    // embark on a journey of epic proportions to find this player cam gobj
    // because the function doesnt pass it in.
    for (int i = 0; i < CM_CAMERA_MAX; i++)
    {
        GOBJ *g = stc_plycam_lookup->cam_gobjs[i];

        if (!g)
            continue;

        PlayerCamData *gp = g->userdata;
        if (gp->cam_data != cd)
            continue;

        // ok we found it
        GOBJ *r = Ply_GetRiderGObj(gp->ply);
        RiderData *rd = r->userdata;

        return rd;
    }

    return 0;
}

// Rearview
int rearview_enabled = 1;
u8 rearview_is_pressing[5];
void Rearview_InitFlags()
{
    // init rearview flags
    for (int i = 0; i < GetElementsIn(rearview_is_pressing); i++)
        rearview_is_pressing[i] = 0;
}

void Rearview_Check(CamData *cam_data, int controller_idx)
{
    if (!rearview_enabled)
        return;

    RiderData *rd = PlyCam_GetRiderData(cam_data);

    if (rd->input.held & HSD_BUTTON_X)
    {
        cam_data->rotation_amt = 3.1;
        rearview_is_pressing[rd->ply] = 1;
    }
    else if (rearview_is_pressing[rd->ply] == 1)
    {
        cam_data->rotation_amt = 0;
        rearview_is_pressing[rd->ply] = 0;
    }

    return;
}
CODEPATCH_HOOKCREATE(0x800c0588, "mr 3,29\n\t"
                                 "mr 4,30\n\t",
                     Rearview_Check, "", 0)
CODEPATCH_HOOKCREATE(0x800cc2d8, "mr 3,30\n\t"
                                 "mr 4,31\n\t",
                     Rearview_Check, "", 0)

// Camera
int camerazoom_kind = 1;
void Camera_InitDefaultZoom()
{
    if (camerazoom_kind == 0)
        return;

    static float *cam_default_values = (float *)0x80557248;

    // init camera height for all players
    for (int i = 0; i < 4; i++)
        stc_plycam_lookup->ply_cam_control[i].distance = 8.4;

    return;
}
CODEPATCH_HOOKCREATE(0x800b048c, "", Camera_InitDefaultZoom, "", 0)
void Camera_Init()
{

    CODEPATCH_HOOKAPPLY(0x800c0588);
    CODEPATCH_HOOKAPPLY(0x800cc2d8);

    // init camera zoom
    CODEPATCH_HOOKAPPLY(0x800b048c);
}
