
#include "text.h"
#include "os.h"
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

extern struct UIColor *stc_ui_colors;
extern int stc_ui_colors_num;

void Kicon_CopyTextures(MatAnimDesc *src_matanimdesc, HSD_Archive *ifall_archive)
{
    // load if not present
    if (!ifall_archive)
        Gm_LoadGameFile(&ifall_archive, COLORTEX_FILENAME); // get custom icons from file

    // get custom texanim
    HSD_TexAnim *dst_texanim = Archive_GetPublicAddress(ifall_archive, "ScInfPlicon_texanim");

    // copy vanilla textures to custom texanim
    for (int j = 0; (j * 50) < dst_texanim->n_imagetbl; j++)
        memcpy(&dst_texanim->imagetbl[j * 50], &src_matanimdesc->texture_anim->imagetbl[j * 8], sizeof(void *) * 8); // copy over vanilla image data pointers
    for (int j = 0; (j * 50) < dst_texanim->n_tluttbl; j++)
        memcpy(&dst_texanim->tluttbl[j * 50], &src_matanimdesc->texture_anim->tluttbl[j * 8], sizeof(void *) * 8); // copy over vanilla tlut

    // replace texanim pointer
    src_matanimdesc->texture_anim = dst_texanim;
}
void Board_SetColors(int color_idx, JOBJ *border_j, JOBJ *score_j, JOBJ *plynum_j)
{
    struct UIColor *ui_color = &stc_ui_colors[color_idx];

    Select_SetUIColor(border_j, 0, 1, ui_color->main);   // main
    Select_SetUIColor(border_j, 0, 2, ui_color->dark);   // dark
    Select_SetUIColor(plynum_j, 0, 0, ui_color->bright); // PX
    Select_SetUIColor(plynum_j, 0, 1, ui_color->main);   // hmn icon
    Select_SetUIColor(score_j, 0, 0, ui_color->bright);  // fill
    Select_SetUIColor(score_j, 0, 1, ui_color->dark);    // border
}

// Air Ride Results Screen
void AirRideResults_OverloadKicon()
{
    JOBJSet **kicon_set = (JOBJSet **)(Gm_GetMenuData()->airride_results.Kicon_jobjset);

    // get vanilla texanim
    MatAnimDesc *matanimdesc_vanilla = Matanimjoint_GetMatAnimDescByIndex(kicon_set[0]->matanimjoint[0], 1, 0);
    Kicon_CopyTextures(matanimdesc_vanilla, 0);
}
CODEPATCH_HOOKCREATE(0x801662d8, "", AirRideResults_OverloadKicon, "", 0)
void AirRideResults_SetPlyNumColor(JOBJ *j, int color_idx)
{
    struct UIColor *ui_color = &stc_ui_colors[color_idx];

    Select_SetUIColor(j, 1, 0, ui_color->main); // numbers
    Select_SetUIColor(j, 1, 1, ui_color->main); // numbers
    Select_SetUIColor(j, 1, 2, ui_color->main); // hmn icon
}
CODEPATCH_HOOKCREATE(0x801653a4, "mr 3,29\n\t"
                                 "mr 4,28\n\t",
                     AirRideResults_SetPlyNumColor, "", 0)
void AirRideResults_SetBoardColor(JOBJ *j, int color_idx)
{
    struct UIColor *ui_color = &stc_ui_colors[color_idx];

    Select_SetUIColor(j, 0, 1, ui_color->main); //
}
void AirRideResults_SetAllRankColor(JOBJ *j, int color_idx)
{
    struct UIColor *ui_color = &stc_ui_colors[color_idx];

    Select_SetUIColor(j, 0, 0, ui_color->main); //
}

// City Results Screen
void CityResults_OverloadKicon()
{
    JOBJSet **kicon_set = Gm_GetMenuData()->city_results.Kicon_jobjset;

    // get vanilla texanim
    MatAnimDesc *matanimdesc_vanilla = Matanimjoint_GetMatAnimDescByIndex(kicon_set[0]->matanimjoint[0], 1, 0);
    Kicon_CopyTextures(matanimdesc_vanilla, 0);
}
CODEPATCH_HOOKCREATE(0x801724f0, "", CityResults_OverloadKicon, "", 0)
void CityResults_SetBoardColor()
{
    MnResultsCityBoard4Data *board_data = Gm_GetMenuData()->city_results.board_gobj->userdata;

    Board_SetColors(board_data->ply[0].color_idx,
                    board_data->ply[0].border_j,
                    board_data->ply[0].score_j,
                    board_data->ply[0].plynum_j);
}
CODEPATCH_HOOKCREATE(0x801736c4, "", CityResults_SetBoardColor, "", 0)
void CityResults_SetBoard2Color(int ply)
{
    MnResultsCityBoard4Data *board_data = Gm_GetMenuData()->city_results.board2_gobj->userdata;

    Board_SetColors(board_data->ply[ply].xc,
                    board_data->ply[ply].border_j,
                    board_data->ply[ply].score_j,
                    board_data->ply[ply].plynum_j);
}
CODEPATCH_HOOKCREATE(0x80174774, "li 3,120\n\t"
                                 "divw 3,31,3\n\t",
                     CityResults_SetBoard2Color,
                     "", 0)
void CityResults_SetBoard4Color(int ply)
{
    MnResultsCityBoard4Data *board_data = Gm_GetMenuData()->city_results.board4_gobj->userdata;

    Board_SetColors(board_data->ply[ply].color_idx,
                    board_data->ply[ply].border_j,
                    board_data->ply[ply].score_j,
                    board_data->ply[ply].plynum_j);
}
CODEPATCH_HOOKCREATE(0x801759b8, "li 3,120\n\t"
                                 "divw 3,31,3\n\t",
                     CityResults_SetBoard4Color,
                     "", 0)
void CityResults_SetAllRankColor(JOBJ *j, int color_idx)
{
    struct UIColor *ui_color = &stc_ui_colors[color_idx];

    Select_SetUIColor(j, 0, 1, ui_color->main); // background
    Select_SetUIColor(j, 0, 2, ui_color->dark); // plynum
}
// MnBestrap
void Bestrap_OverloadKicon()
{
    JOBJSet **kicon_set = Gm_GetMenuData()->bestrap.Kicon_jobjset;

    // get vanilla texanim
    MatAnimDesc *matanimdesc_vanilla = Matanimjoint_GetMatAnimDescByIndex(kicon_set[0]->matanimjoint[0], 0, 0);
    Kicon_CopyTextures(matanimdesc_vanilla, 0);
}
CODEPATCH_HOOKCREATE(0x80186b88, "", Bestrap_OverloadKicon, "", 0)

void Stadium_OverloadKicon()
{

    JOBJSet **kicon_set = Gm_GetMenuData()->stadium.Kicon_jobjset;

    // get vanilla texanim
    MatAnimDesc *matanimdesc_vanilla = Matanimjoint_GetMatAnimDescByIndex(kicon_set[0]->matanimjoint[0], 0, 0);
    Kicon_CopyTextures(matanimdesc_vanilla, 0);
}
CODEPATCH_HOOKCREATE(0x8016426c, "", Stadium_OverloadKicon, "", 0)

// MnRadar
void MnRadar_SetPlyNmColor(JOBJ *j, int color_idx)
{
    struct UIColor *ui_color = &stc_ui_colors[color_idx];

    Select_SetUIColor(j, 1, 0, ui_color->hud);  // kirby face
    Select_SetUIColor(j, 1, 2, ui_color->main); // numbers
    Select_SetUIColor(j, 1, 3, ui_color->main); // numbers
}
CODEPATCH_HOOKCREATE(0x8016087c, "mr 3,28\n\t"
                                 "mr 4,27\n\t",
                     MnRadar_SetPlyNmColor, "", 0)

void Menus_Init()
{
    CODEPATCH_HOOKAPPLY(0x801724f0);                                   // city results screen - Kicon
    CODEPATCH_HOOKAPPLY(0x801736c4);                                   // city results screen - Board
    CODEPATCH_HOOKAPPLY(0x80174774);                                   // city results screen - Board2
    CODEPATCH_HOOKAPPLY(0x801759b8);                                   // city results screen - Board4
    CODEPATCH_REPLACECALL(0x80171c1c, CityResults_SetAllRankColor);    // Air Ride Results - AllRank
    CODEPATCH_HOOKAPPLY(0x80186b88);                                   // bestrap - kicon
    CODEPATCH_HOOKAPPLY(0x8016426c);                                   // stadium - Kicon
    CODEPATCH_HOOKAPPLY(0x801662d8);                                   // Air Ride Results - Kicon
    CODEPATCH_HOOKAPPLY(0x801653a4);                                   // Air Ride Results - PlyNum
    CODEPATCH_REPLACECALL(0x801649ec, AirRideResults_SetBoardColor);   // Air Ride Results - Board
    CODEPATCH_REPLACECALL(0x80166ec4, AirRideResults_SetAllRankColor); // Air Ride Results - AllRank

    CODEPATCH_HOOKAPPLY(0x8016087c); // MnRadar - PlyNum

    //

    // Kicon stride
    CODEPATCH_REPLACEINSTRUCTION(0x80166348, 0x1c830000 | stc_ui_colors_num); // air ride select
    CODEPATCH_REPLACEINSTRUCTION(0x801664f8, 0x1c000000 | stc_ui_colors_num); // air ride select
}