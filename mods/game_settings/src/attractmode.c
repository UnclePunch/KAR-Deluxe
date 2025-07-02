
#include "text.h"
#include "useful.h"
#include "hsd.h"
#include "preload.h"
#include "scene.h"
#include "inline.h"
#include "audio.h"
#include "obj.h"
#include "game.h"

#include "attractmode.h"
#include "code_patch/code_patch.h"
#include "hoshi/settings.h"

// Menu Idle
int attractmode_enabled = 0;
int AttractMode_Hook()
{
    if (attractmode_enabled)
        return 0;
    else
        return 1;
}
CODEPATCH_HOOKCONDITIONALCREATE(0x8001823c, "", AttractMode_Hook, "", 0, 0x80018270)
void AttractMode_Init()
{
    CODEPATCH_HOOKAPPLY(0x8001823c);
}
