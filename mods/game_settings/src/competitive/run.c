
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
#include "hoshi/settings.h"

// extern declaration of functions defined in assembly
extern void run_hook();

// variables used in assembly functions
int is_run_enabled = 1;
float run_speed_mult = 1.5;

void Run_Init()
{
    _CodePatch_HookApply((int *)0x801b8f60, run_hook);
}