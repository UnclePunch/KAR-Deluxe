
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

/*
C21B8F60 00000005
C0030094 81DF03D8
55CE05AD 41820014
3DC04000 91C1FFFC
C081FFFC EC000132
00000000 00000000
*/

extern void run_hook();
int run_enabled = 1;

void Run_Init()
{
    OSReport("run_hook %p\n", run_hook);
}