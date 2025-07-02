
#include "text.h"
#include "useful.h"
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

void Rearview_Check(float *unk_cam, int controller_idx)
{
    if (!rearview_enabled)
        return;

    if (stc_engine_pads[controller_idx].held & HSD_BUTTON_X)
    {
        unk_cam[0x88 / 4] = 3.1;
        rearview_is_pressing[controller_idx] = 1;
    }
    else if (rearview_is_pressing[controller_idx] == 1)
    {
        unk_cam[0x88 / 4] = 0;
        rearview_is_pressing[controller_idx] = 0;
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
        cam_default_values[(0x240 / 4) + i] = 8.4;

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
