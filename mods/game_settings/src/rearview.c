
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

    if (!cam_data->target || cam_data->target->ply == 5)
        return;

    RiderData *rd = Ply_GetRiderGObj(cam_data->target->ply)->userdata;

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

    // init camera height for all players
    for (int i = 0; i < 4; i++)
    {
        stc_plycam_lookup->ply_distance[i].normal = 8.4;
        stc_plycam_lookup->ply_distance[i].rail = 6;
    }

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
