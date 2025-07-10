
#include "text.h"
#include "useful.h"
#include "hsd.h"
#include "preload.h"
#include "scene.h"
#include "inline.h"
#include "audio.h"
#include "game.h"
#include <string.h>

#include "colors.h"
#include "airride.h"
#include "code_patch/code_patch.h"

#include "hoshi/settings.h"

extern struct UIColor stc_ui_colors[];
extern int stc_ui_colors_num;

static HSD_Archive *stc_ifall_morecolors;
static HSD_TexAnim *stc_plicon_texanim;
GXColor stc_cpu_ui_color = {153, 153, 153, 255};

////////////
// Common //
////////////

void Game_OverloadRdKirby(RiderKind rd_kind)
{
    // copies the original 8 kirby color textures from RdKirby.dat to our
    // custom texanim in IfAllMoreColors, then modifies the RdKirby.dat's
    // matanimjoint texanim pointers to point to our texanim.
    //
    // Note:
    // I would rather not modify the matanimjoint data in RdKirby and instead
    // take a similar approach as i did later with PlIcon, where we hook every
    // bit of code that changes the texture and add the tobj anim directly to the
    // live jobj. higher effort and more hooks but less destructive.

    if (rd_kind != RDKIND_KIRBY)
        return;

    struct RdKirbyOverload
    {
        char *override_symbol;
        u8 mat_indices_num;
        u8 mat_indices[9];
    };

    static struct RdKirbyOverload rdkirby_overload[] = {
        {
            .override_symbol = "KirbyModelBody_texanim",
            .mat_indices_num = 6,
            .mat_indices = {
                0,
                2,
                4,
                15,
                17,
                19,
            },
        },
        {
            .override_symbol = "KirbyModelFace_texanim",
            .mat_indices_num = 9,
            .mat_indices = {
                1,
                3,
                5,
                6,
                9,
                11,
                16,
                18,
                20,
            },
        },
        {
            .override_symbol = "KirbyModelShoes_texanim",
            .mat_indices_num = 3,
            .mat_indices = {
                12,
                13,
                14,
            },
        },
    };

    for (int i = 0; i < GetElementsIn(rdkirby_overload); i++)
    {
        // our data
        HSD_TexAnim *texanim_custom = Archive_GetPublicAddress(stc_ifall_morecolors, rdkirby_overload[i].override_symbol);

        if (!texanim_custom)
        {
            // OSReport("MoreColors: Warning symbol %s not found in IfAllMoreColors.dat\n", rdkirby_overload[i].override_symbol);
            continue;
        }

        // each mat
        for (int mat_idx = 0; mat_idx < rdkirby_overload[i].mat_indices_num; mat_idx++)
        {
            // get vanilla matanimdesc
            MatAnimDesc *matanimdesc_vanilla = Matanimjoint_GetMatAnimDescByIndex((*stc_rdDataKirby)->model->matanimjointdesc,
                                                                                  0,
                                                                                  rdkirby_overload[i].mat_indices[mat_idx]);

            // OSReport("%s matanimdesc #%d texanim: %p\n", rdkirby_overload[i].override_symbol, rdkirby_overload[i].mat_indices[mat_idx], matanimdesc_vanilla->texture_anim);

            // ensure we haven't already overloaded
            if (matanimdesc_vanilla->texture_anim != texanim_custom)
            {
                for (int j = 0; (j * 50) < texanim_custom->n_imagetbl; j++)
                    memcpy(&texanim_custom->imagetbl[j * 50], &matanimdesc_vanilla->texture_anim->imagetbl[j * 8], sizeof(void *) * 8); // copy over vanilla image data pointers
                for (int j = 0; (j * 50) < texanim_custom->n_tluttbl; j++)
                    memcpy(&texanim_custom->tluttbl[j * 50], &matanimdesc_vanilla->texture_anim->tluttbl[j * 8], sizeof(void *) * 8); // copy over vanilla tlut

                // copy next
                texanim_custom->next = matanimdesc_vanilla->texture_anim->next;

                // overload pointer in vanilla file to our texanim
                matanimdesc_vanilla->texture_anim = texanim_custom;
            }
        }
    }

    return;
}
CODEPATCH_HOOKCREATE(0x8019061c, "rlwinm 3,31,30,0,31\n\t", Game_OverloadRdKirby, "", 0)

//////////////////
// Player Model //
//////////////////

void KirbyModel_SetDefaultMaterialIndex(RiderData *rd)
{
    static void (*RiderKirby_SetMaterialIndex)(RiderData *rd, int r4, int mat_idx) = (void *)0x80198d1c;

    RiderKirby_SetMaterialIndex(rd, 0, Select_ColorToAnim(rd->color_idx));
    return;
}
CODEPATCH_HOOKCREATE(0x801a5c60, "mr 3,31\n\t", KirbyModel_SetDefaultMaterialIndex, "", 0x801a5c80)

void Machine_GetArrowColor(GOBJ *rider_gobj, GXColor *out)
{
    RiderData *rp = rider_gobj->userdata;

    *out = stc_ui_colors[Select_ColorToAnim(rp->color_idx)].hud;
}

/////////
// HUD //
/////////

void Game_LoadAdditionalAssets()
{
    stc_ifall_morecolors = 0;
    stc_plicon_texanim = 0;

    // load our file
    Gm_LoadGameFile(&stc_ifall_morecolors, "IfAllMoreColors");

    // get plicon data
    if (stc_ifall_morecolors)
        stc_plicon_texanim = Archive_GetPublicAddress(stc_ifall_morecolors, "ScInfPlicon_texanim");

    return;
}
CODEPATCH_HOOKCREATE(0x80014690, "", Game_LoadAdditionalAssets, "", 0)

void Game_OverloadIfAll()
{
    static char *stc_ifall_symbols[] = {
        "ScInfDamageP_scene_models",  // single player
        "ScInfDamageP2_scene_models", // 2 player split screen
        "ScInfDamageP4_scene_models", // 3 player split screen
        "ScInfDamageP4_scene_models", // 4 player split screen
    };

    //
    int view_num = Gm_GetPlyViewNum();
    if (view_num <= 0)
        return;

    MatAnimDesc *matanimdesc_vanilla;
    JOBJSet **jobjset;

    // overload damage icon
    char *this_symbol = stc_ifall_symbols[view_num - 1];                                                  //
    HSD_Archive *ifall_archive = *Gm_GetIfAllScreenArchive();                                             //
    jobjset = Archive_GetPublicAddress(ifall_archive, this_symbol);                                       //
    matanimdesc_vanilla = Matanimjoint_GetMatAnimDescByIndex(jobjset[0]->matanimjoint[0], 2, 0);          // vanila data
    HSD_TexAnim *texanim_custom = Archive_GetPublicAddress(stc_ifall_morecolors, "ScInfDamageP_texanim"); // our data
    if (matanimdesc_vanilla->texture_anim != texanim_custom)                                              // only overload once
    {
        for (int j = 0; (j * 50) < texanim_custom->n_imagetbl; j++) // copy over vanilla image data pointers
        {
            // OSReport("DamageP: copying vanilla textures %d to %d\n", (j * 50), (j * 50) + 7);

            memcpy(&texanim_custom->imagetbl[j * 50],
                   &matanimdesc_vanilla->texture_anim->imagetbl[j * 8],
                   sizeof(void *) * 8); //
        }

        matanimdesc_vanilla->texture_anim = texanim_custom; // overload pointer in vanilla file to our texanim
    }

    // copy vanilla plicons to custom file
    ifall_archive = *Gm_GetIfAllCityArchive();                                                   //
    jobjset = Archive_GetPublicAddress(ifall_archive, "ScInfPlicon_scene_models");               //
    matanimdesc_vanilla = Matanimjoint_GetMatAnimDescByIndex(jobjset[0]->matanimjoint[0], 1, 0); // vanila data
    HSD_TexAnim *texanim_vanilla = matanimdesc_vanilla->texture_anim;
    memcpy(&stc_plicon_texanim->imagetbl[0],
           &texanim_vanilla->imagetbl[0],
           sizeof(void *) * 8);
    memcpy(&stc_plicon_texanim->tluttbl[0],
           &texanim_vanilla->tluttbl[0],
           sizeof(void *) * 8);
    stc_plicon_texanim->imagetbl[50] = texanim_vanilla->imagetbl[8]; // copy CPU image data
    stc_plicon_texanim->tluttbl[50] = texanim_vanilla->tluttbl[8];   // copy CPU tult

    return;
}
CODEPATCH_HOOKCREATE(0x80112d9c, "", Game_OverloadIfAll, "", 0)

void Game_ReplacePlIconAnim(GOBJ *g)
{
    HudPliconData *gp = g->userdata;
    Game3dData *gd = Gm_Get3dData();
    JOBJ *j = g->hsd_object;

    JOBJSet **set;
    int frame;
    RiderKind kind = Ply_GetRiderKind2(gp->ply);

    // get jobjset
    switch (kind)
    {
    case RDKIND_KIRBY:
    {
        set = gd->ScInfPliconKirby_scene_models;
        break;
    }
    case RDKIND_DEDEDE:
    {
        set = gd->ScInfPliconDedede_scene_models;
        break;
    }
    case RDKIND_METAKNIGHT:
    {
        set = gd->ScInfPliconMetaKnight_scene_models;
        break;
    }
    }

    // get frame
    if (Ply_CheckIfCPU(gp->ply))
        frame = 50;
    else
        frame = Select_ColorToAnim(Ply_GetColor(gp->ply));

    // add anim
    JObj_AddSetAnim(j, 0, set[0], frame, 0);

    // only expanding kirby's colors for now...
    if (kind == RDKIND_KIRBY)
    {
        // override tobjanim with our own
        TOBJ *t = JObj_GetDObjIndex(j, 1, 0)->mobj->tobj;
        TOBJ_AddAnim(t, stc_plicon_texanim);
    }

    // set frame
    JObj_SetFrameAndRate(j, frame, 0);

    return;
}
CODEPATCH_HOOKCREATE(0x80120ffc, "mr 3,30\n\t", Game_ReplacePlIconAnim, "", 0x80121074) // init
CODEPATCH_HOOKCREATE(0x801201e0, "mr 3,27\n\t", Game_ReplacePlIconAnim, "", 0x801202ac) // after hit

void HUD_SetHPBarColor(HudHpBarData *hd)
{
    if (Ply_GetRiderKind2(hd->ply) != RDKIND_KIRBY)
        return;

    Select_SetUIColor(hd->kirby_jobj, 0, 0, stc_ui_colors[Select_ColorToAnim(Ply_GetColor(hd->ply))].hud);
}
CODEPATCH_HOOKCREATE(0x8011ee00, "mr 3,30\n\t", HUD_SetHPBarColor, "", 0)

void HUD_SetMapIconColor(int ply, JOBJ *j)
{
    GXColor color;

    if (Ply_CheckIfCPU(ply))
        color = stc_cpu_ui_color;
    else
        color = stc_ui_colors[Select_ColorToAnim(Ply_GetColor(ply))].hud;

    Select_SetUIColor(j, 0, 0, color);
}
CODEPATCH_HOOKCREATE(0x8011e13c, "mr 3,26\n\t"
                                 "mr 4,28\n\t",
                     HUD_SetMapIconColor, "", 0)
CODEPATCH_HOOKCREATE(0x8011dc70, "mr 3,31\n\t"
                                 "mr 4,30\n\t",
                     HUD_SetMapIconColor, "", 0)

void HUD_SetPlyNm2Color(JOBJ *j, int color_idx)
{
    struct UIColor *ui_color = &stc_ui_colors[color_idx];

    Select_SetUIColor(j, 1, 0, ui_color->hud);  // kirby face
    Select_SetUIColor(j, 2, 0, ui_color->main); // numbers
    Select_SetUIColor(j, 2, 1, ui_color->main); // numbers
}
CODEPATCH_HOOKCREATE(0x80126284, "lwz 3, 0x28 (29)\n\t"
                                 "mr 4,28\n\t",
                     HUD_SetPlyNm2Color, "", 0)

void HUD_SetPlyNumColor(int ply, JOBJ *j)
{
    GXColor color;

    if (Ply_CheckIfCPU(ply))
        color = stc_cpu_ui_color;
    else
        color = stc_ui_colors[Select_ColorToAnim(Ply_GetColor(ply))].hud;

    Select_SetUIColor(j, 0, 0, color);
}
CODEPATCH_HOOKCREATE(0x8011fe90, "mr 3,31\n\t"
                                 "mr 4,28\n\t",
                     HUD_SetPlyNumColor, "", 0)

void HUD_SetKirbyWalkColor(int ply, JOBJ *j)
{
    GXColor color;

    if (Ply_CheckIfCPU(ply))
        color = stc_cpu_ui_color;
    else
        color = stc_ui_colors[Select_ColorToAnim(Ply_GetColor(ply))].hud;

    Select_SetUIColor(j, 0, 1, color);
}
CODEPATCH_HOOKCREATE(0x80124670, "mr 3,26\n\t"
                                 "mr 4,29\n\t",
                     HUD_SetKirbyWalkColor, "", 0)

void HUD_GetDamagePFrame(int attacker_ply, RiderKind victim_kind, int victim_color_idx)
{
    static void (*HUD_CreateDamageP)(int attacker_ply, int r4, int frame) = (void *)0x80113f10;
    static u8 *stc_DamageP_frame_lookup = (u8 *)0x804a91e0;

    int frame;

    // custom color
    if (victim_color_idx > 7)
        frame = victim_kind * 50 + victim_color_idx; // each rider has 50 colors, simple
    // original color
    else
    {
        // vanilla riders' damage icons have a stride of 10, so get the original index
        int index = victim_kind * 10 + victim_color_idx;

        // the textures are not directly mapped to the color index, so lookup the correct frame using a dol struct... (8012a6f4)
        int vanilla_frame = stc_DamageP_frame_lookup[index];

        // finally convert this vanilla frame index to an index in our IfAllMoreColors texanim, where the stride is 50
        frame = vanilla_frame - (victim_kind * 10) + (victim_kind * 50);
    }

    // OSReport("DamageP: displaying texture %d\n", frame);

    HUD_CreateDamageP(attacker_ply, 0, frame);

    return;
}

void Game_Init()
{
    // Player Model
    CODEPATCH_HOOKAPPLY(0x801a5c60);                            // redirect
    CODEPATCH_HOOKAPPLY(0x8019061c);                            // Insert hook to overload RdKirby data
    CODEPATCH_REPLACEINSTRUCTION(0x801a6948, 0x38a00000 | 50);  // fire ability textures start
    CODEPATCH_REPLACEINSTRUCTION(0x801a7b70, 0x38a00000 | 100); // bird ability textures start

    // Machine Arrow
    CODEPATCH_REPLACEFUNC(0x80192720, Machine_GetArrowColor);

    //  HUD
    CODEPATCH_HOOKAPPLY(0x80014690); // Insert hook to load custom assets in game
    CODEPATCH_HOOKAPPLY(0x80112d9c); // Insert hook to overload IfAll data
    CODEPATCH_HOOKAPPLY(0x80120ffc); // insert hook to use custom Plicon matanim - init
    CODEPATCH_HOOKAPPLY(0x801201e0); // insert hook to use custom Plicon matanim - after hit
    // CODEPATCH_REPLACEINSTRUCTION(0x8012101c, 0x3b600000 | 50); // Plicon stride (cpu starts at 50)
    // CODEPATCH_REPLACEINSTRUCTION(0x80120200, 0x3be00000 | 50); // Plicon stride (cpu starts at 50)
    CODEPATCH_REPLACEFUNC(0x80010fac, HUD_GetDamagePFrame); // new damage frame lookup
    CODEPATCH_REPLACEINSTRUCTION(0x8012a700, 0x60000000);   // null original damage frame lookup
    CODEPATCH_HOOKAPPLY(0x8011ee00);                        // HP bar
    CODEPATCH_HOOKAPPLY(0x8011dc70);                        // map icon
    CODEPATCH_HOOKAPPLY(0x8011e13c);                        // map icon
    CODEPATCH_HOOKAPPLY(0x80126284);                        // PlyNm2
    CODEPATCH_HOOKAPPLY(0x8011fe90);                        // PlyNum
    CODEPATCH_HOOKAPPLY(0x80124670);                        // KirbyWalk

    KARPlus_AddPreloadGameFile("IfAllMoreColors");

    // HUD/
    /*
    IfAll1c:
        ScInfMapkirby4_scene_models         map face icon 8011dcac - done
        ScInfPlynum_scene_models            PX overhead indicator 8011f8a8 - done
        ScInfPlicon_scene_models            peripheral face icon 801202cc
    IfAll12s:
        ScInfDamageP2_scene_modes           damage indicator - change rider kind stride, done
        ScInfHp2_scene_models               hp bar 8011eff4 - change CPU's ID, done
        ScInfPlynm2_scene_models            PX indicator static - done

    there are numerous files that contain these duplicated assets
    plan is to move all the data into a single new file, load it every game and replace matanim pointers in the original file
    leave space in new matanim for original images and copy the pointers there?
    */
}