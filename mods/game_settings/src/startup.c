
#include "text.h"
#include "os.h"
#include "hsd.h"
#include "preload.h"
#include "scene.h"
#include "inline.h"
#include "audio.h"
#include "obj.h"
#include "game.h"

#include "hoshi/func.h"

#include "code_patch/code_patch.h"
#include "startup.h"

// Startup Behavior
StartupKind startup_kind = STARTUP_CITYSELECT;
static u8 startup_scenes[] = {
    MJRKIND_MOVIE, // opening movie
    MJRKIND_TITLE, // title screen
    MJRKIND_MENU,  // main menu
    MJRKIND_CITY,  // city trial select screen
    MJRKIND_AIR,   // air ride select screen
    MJRKIND_LAN,   // LAN
};
void Startup_Init()
{
    // change bootup scene to this
    MajorKind major_kind = startup_scenes[startup_kind];

    Hoshi_SetBootScene(major_kind, 0);
}
