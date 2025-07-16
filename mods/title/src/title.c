
#include "text.h"
#include "os.h"
#include "hsd.h"
#include "obj.h"
#include "preload.h"
#include "game.h"
#include "scene.h"
#include "inline.h"
#include "audio.h"
#include "scene.h"
#include "code_patch/code_patch.h"

#include <string.h>

#include "title.h"

HSD_Archive *title_archive = 0;

void Hook_OnTitleLoad()
{
    Gm_LoadGameFile(&title_archive, "MnTitleDeluxe");
}
CODEPATCH_HOOKCREATE(0x8000d2b4, "", Hook_OnTitleLoad, "", 0)

void Hook_OnTitleCreate()
{
    if (title_archive)
    {
        // hide original title
        JOBJ *j = Gm_GetMenuData()->ScMenTitleFg_gobj->hsd_object;
        JObj_SetFlagsAll(JObj_GetIndex(j, 14), JOBJ_HIDDEN);

        JOBJSet **set = Archive_GetPublicAddress(title_archive, "ScMenTitleFg_scene_models");

        // create deluxe model
        GOBJ *g = MenuElement_Create(set[0]->jobj);
        GObj_AddProc(g, (void *)0x8017b424, 1);

        TitleData *gp = (TitleData *)MenuElement_AddData(g, 99);
        gp->set = set;

        int title_state = TitleScreen_GetData()->state;
        int anim_id = (title_state == 3) ? 1 : 0;
        JObj_AddSetAnim(g->hsd_object, anim_id, set[0], 0, 1);
    }
}
CODEPATCH_HOOKCREATE(0x8017b5d8, "", Hook_OnTitleCreate, "", 0)

void Title_ApplyPatches()
{
    CODEPATCH_HOOKAPPLY(0x8000d2b4);
    CODEPATCH_HOOKAPPLY(0x8017b5d8);
}