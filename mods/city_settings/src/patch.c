
#include "text.h"
#include "useful.h"
#include "hsd.h"
#include "preload.h"
#include "scene.h"
#include "inline.h"
#include "audio.h"
#include "game.h"
#include "item.h"

#include "citysettings.h"
#include "menu.h"

#include "code_patch/code_patch.h"

#include "patches/items.h"
#include "patches/events.h"
#include "patches/machines.h"
#include "patches/stadium.h"
#include "patches/select.h"

void Patches_Init()
{
    // begin applying patches
    Items_ApplyPatches();
    Events_ApplyPatches();
    Machines_ApplyPatches();
    Stadium_ApplyPatches();
    Select_ApplyPatches();
}