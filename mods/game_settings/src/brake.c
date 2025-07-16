
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

// Brake
int brake_enabled = 1;
void Brake_AllowBToCharge(RiderData *rp, float lstick_x, float lstick_y)
{
    int is_charge;
    if (brake_enabled)
        is_charge = ((rp->input.held & (PAD_BUTTON_A | PAD_BUTTON_B)) >> 8);
    else
        is_charge = ((rp->input.held & (PAD_BUTTON_A)) >> 8);

    MachineData *mp = rp->machine_gobj->userdata;

    mp->charge.stick.X = lstick_x;
    mp->charge.stick.Y = lstick_y;
    mp->charge.is = is_charge;

    return;
}
CODEPATCH_HOOKCREATE(0x80190d70, "mr 3,31\n\t", Brake_AllowBToCharge, "b 8\n\t", 0)
int Brake_ForbidChargeForward(MachineData *mp)
{
    if (!brake_enabled)
        return 0;

    // check if held B
    if (mp->charge.is == 2)
        return 1;

    return 0;
}
CODEPATCH_HOOKCONDITIONALCREATE(0x801ef4f4, "mr 3,31\n\t", Brake_ForbidChargeForward, "lbz 0, 0x0C30(31)\n\t", 0, 0x801ef54c)
int Brake_OnBikeBuildCharge(MachineData *mp)
{
    if (!brake_enabled)
        return 0;

    // check if held B
    if (mp->charge.is == 2)
        return 1;

    return 0;
}
CODEPATCH_HOOKCONDITIONALCREATE(0x801fa298, "mr 3,31\n\t", Brake_OnBikeBuildCharge, "", 0, 0x801fa2a0)
int Brake_OnStarBuildCharge(MachineData *mp)
{
    if (!brake_enabled)
        return 0;

    // check if held B
    if (mp->charge.is == 2)
        return 1;

    return 0;
}
CODEPATCH_HOOKCONDITIONALCREATE(0x801ef414, "mr 3,31\n\t", Brake_OnStarBuildCharge, "", 0, 0x801ef428)
void Brake_Init()
{
    CODEPATCH_HOOKAPPLY(0x80190d70);
    CODEPATCH_HOOKAPPLY(0x801ef4f4);
    CODEPATCH_HOOKAPPLY(0x801fa298);
    CODEPATCH_HOOKAPPLY(0x801ef414);
}
