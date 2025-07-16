
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
#include "hoshi/settings.h"

int ability_drop_enabled = 1;

/*
C21A7CEC 00000004
80010014 823F03E4
2C110010 4082000C
3A400000 925F091C
60000000 00000000
*/

void AbilityDrop_Hook(RiderData *rd)
{
    // check to drop ability
    if (ability_drop_enabled &&                                             // enabled
        rd->copy_kind != -1 &&                                              // not random ability shuffling
        rd->copy_kind != COPYKIND_SLEEP && rd->copy_kind != COPYKIND_MIC && // undroppable abilities
        rd->input.down & PAD_TRIGGER_Z)                                     // input
    {
        Rider_AbilityRemoveModel(rd);
        Rider_AbilityRemoveUnk(rd);
        Rider_LoseAbilityState_Enter(rd);
        return;
    }

    // orig functionality
    if (rd->cb_copy_input)
        rd->cb_copy_input(rd);
}

void AbilityDrop_Init()
{
    CODEPATCH_REPLACEFUNC(0x801a5fb8, AbilityDrop_Hook);
}