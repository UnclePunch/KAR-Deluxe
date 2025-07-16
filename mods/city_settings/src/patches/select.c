#include "os.h"
#include "preload.h"
#include "scene.h"
#include "inline.h"
#include "game.h"

#include "../citysettings.h"

#include "code_patch/code_patch.h"

void Hook_SetMachineIcon(GOBJ *g)
{

    ScMenuCommon *md = Gm_GetMenuData();
    JOBJSet *sicon2_set = md->city_select.ScMenSelplySicon2Ct_scene_models[0];
    MatAnimDesc *machine_icon_anim = Matanimjoint_GetMatAnimDescByIndex(sicon2_set->matanimjoint[0], 1, 0);
    MatAnimDesc *machine_shadow_anim = Matanimjoint_GetMatAnimDescByIndex(sicon2_set->matanimjoint[0], 2, 0);

    JOBJ *board_jobj = md->city_select.cityselect_menu_gobj->hsd_object;
    DOBJ *machine_shadow_dobj = JObj_GetDObjIndex(board_jobj, 21, 0);
    DOBJ *machine_icon_dobj = JObj_GetDObjIndex(board_jobj, 21, 1);

    // add it to
    DOBJ_AddAnimAll(machine_icon_dobj, 0, machine_icon_anim->texture_anim);
    DOBJ_AddAnimAll(machine_shadow_dobj, 0, machine_shadow_anim->texture_anim);
}
CODEPATCH_HOOKCREATE(0x8015a9f8, "mr 3,28\n\t", Hook_SetMachineIcon, "", 0)

///////////////////
// Apply Patches //
///////////////////

void Select_ApplyPatches()
{
    // CODEPATCH_HOOKAPPLY(0x8015a9f8);
}