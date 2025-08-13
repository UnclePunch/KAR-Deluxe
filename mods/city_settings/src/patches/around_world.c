#include "game.h"
#include "hsd.h"
#include "os.h"
#include "preload.h"
#include "game.h"
#include "inline.h"

#include "around_world.h"
#include "stadium.h"
#include "../citysettings.h"

#include "code_patch/code_patch.h"

int is_enabled = 1;
int stadium_lineup_num = 0;          // describes the amount of stadium events that will be played
int stadium_lineup_idx = 0;          // index of the current stadium event being played
StadiumKind stadium_lineup[7] = {0}; // contains the stadium events that will be played
int stadium_wins[4] = {0};           // tracks wins across stadiums

static char *stad_names[] = {
    "STKIND_DRAG1",
    "STKIND_DRAG2",
    "STKIND_DRAG3",
    "STKIND_DRAG4",
    "STKIND_AIRGLIDER",
    "STKIND_TARGETFLIGHT",
    "STKIND_HIGHJUMP",
    "STKIND_MELEE1",
    "STKIND_MELEE2",
    "STKIND_DESTRUCTION1",
    "STKIND_DESTRUCTION2",
    "STKIND_DESTRUCTION3",
    "STKIND_DESTRUCTION4",
    "STKIND_DESTRUCTION5",
    "STKIND_SINGLERACE1",
    "STKIND_SINGLERACE2",
    "STKIND_SINGLERACE3",
    "STKIND_SINGLERACE4",
    "STKIND_SINGLERACE5",
    "STKIND_SINGLERACE6",
    "STKIND_SINGLERACE7",
    "STKIND_SINGLERACE8",
    "STKIND_SINGLERACE9",
    "STKIND_VSKINGDEDEDE",
};

void AroundWorld_Init()
{
    gmDataAll *gm_data_all = (*stc_gmdataall);

    for (int i = 0; i < GetElementsIn(stadium_wins); i++)
        stadium_wins[i] = 0;

    stadium_lineup_idx = 0;

    AroundWorld_DecideStadiums();

    Gm_GetGameData()->city.stadium_kind = stadium_lineup[0];
}
void AroundWorld_DecideStadiums()
{
    CitySettingsSave *cs = CitySettings_SaveGet();
    int random_stadium_bitfield = cs->random_stadium_bitfield;
    stadium_lineup_num = 0;

    static StadiumGroup stadium_groups[] = {
        STGROUP_DRAGRACE,
        STGROUP_AIRGLIDER,
        STGROUP_TARGETFLIGHT,
        STGROUP_HIGHJUMP,
        STGROUP_MELEE,
        STGROUP_DESTRUCTION,
        STGROUP_SINGLERACE,
    };

    for (int group_idx = 0; group_idx < GetElementsIn(stadium_groups); group_idx++)
    {
        StadiumKind kinds_in_group[10];
        int kinds_in_group_num = 0;

        // create an array of all stadiums that fit this group and are enabled
        for (int kind = 0; kind < STKIND_NUM; kind++)
        {
            if ((random_stadium_bitfield & (1 << kind)) && Gm_GetStadiumGroupFromKind(kind) == stadium_groups[group_idx])
                kinds_in_group[kinds_in_group_num++] = kind;
        }

        if (kinds_in_group_num == 0)
            continue;

        // pick a random one
        stadium_lineup[stadium_lineup_num++] = kinds_in_group[HSD_Randi(kinds_in_group_num)];
    }

    // for (int i = 0; i < stadium_lineup_num; i++)
    //     OSReport("stadium %d: %s\n", i + 1, stad_names[stadium_lineup[i]]);

    return;
}
void AroundWorld_EqualizeItemChances()
{
    if (CitySettings_SaveGet()->settings[CITYSETTING_SAVE_STADIUM] != STADSELECT_ALL)
        return;

    static ItemKind power_ups[] =
        {
            ITKIND_ACCEL,
            ITKIND_TOPSPEED,
            ITKIND_OFFENSE,
            ITKIND_DEFENSE,
            ITKIND_TURN,
            ITKIND_GLIDE,
            ITKIND_CHARGE,
            ITKIND_WEIGHT,
            ITKIND_HP,
        };

    static ItemKind power_downs[] =
        {
            ITKIND_ACCELDOWN,
            ITKIND_TOPSPEEDDOWN,
            ITKIND_OFFENSEDOWN,
            ITKIND_DEFENSEDOWN,
            ITKIND_TURNDOWN,
            ITKIND_GLIDEDOWN,
            ITKIND_CHARGEDOWN,
            ITKIND_WEIGHTDOWN,
        };

    grBoxGeneObj *box_gene_obj = (*stc_grBoxGeneObj);

    int is_allup_present = 0;

    for (int item_idx = 0; item_idx < box_gene_obj->item_group_spawn[BOXKIND_BLUE].num; item_idx++)
    {
        // adjust power up chances
        for (int i = 0; i < GetElementsIn(power_ups); i++)
        {
            if (power_ups[i] == box_gene_obj->item_group_spawn[BOXKIND_BLUE].it_kind[item_idx])
            {
                box_gene_obj->item_group_spawn[BOXKIND_BLUE].chance[item_idx] = 20;
                goto next;
            }
        }

        // adjust power down chances
        for (int i = 0; i < GetElementsIn(power_downs); i++)
        {
            if (power_downs[i] == box_gene_obj->item_group_spawn[BOXKIND_BLUE].it_kind[item_idx])
            {
                box_gene_obj->item_group_spawn[BOXKIND_BLUE].chance[item_idx] = 7;
                goto next;
            }
        }

        if (box_gene_obj->item_group_spawn[BOXKIND_BLUE].it_kind[item_idx] == ITKIND_ALLUP)
        {
            box_gene_obj->item_group_spawn[BOXKIND_BLUE].chance[item_idx] = 1;
            is_allup_present = 1;
        }

    next:
        continue;
    }

    // if all_up not present, add it
    if (!is_allup_present)
    {
        // add it to the current table
        box_gene_obj->item_group_spawn[BOXKIND_BLUE].it_kind[box_gene_obj->item_group_spawn[BOXKIND_BLUE].num] = ITKIND_ALLUP;
        box_gene_obj->item_group_spawn[BOXKIND_BLUE].chance[box_gene_obj->item_group_spawn[BOXKIND_BLUE].num] = 1;
        box_gene_obj->item_group_spawn[BOXKIND_BLUE].num++;
    }
}
void AroundWorld_CalculateRanks(s8 *ranks, void *score, char data_type, int is_higher_better)
{
    // store final win count
    for (int i = 0; i < 4; i++)
    {
        if (Ply_GetPKind(i) == PKIND_NONE)
        {
            ranks[i] = -1;
            continue;
        }

        int placement = 0;
        for (int j = 0; j < 4; j++)
        {
            if (j == i)
                continue;

            if (data_type == 'i')
            {
                int this_score = ((int *)score)[i];
                int that_score = ((int *)score)[j];

                if (is_higher_better)
                {
                    if (this_score < that_score)
                        placement++;
                }
                else
                {
                    if (this_score > that_score)
                        placement++;
                }
            }
            else if (data_type == 'f')
            {
                float this_score = ((float *)score)[i];
                float that_score = ((float *)score)[j];

                if (is_higher_better)
                {
                    if (this_score < that_score)
                        placement++;
                }
                else
                {
                    if (this_score > that_score)
                        placement++;
                }
            }
        }

        ranks[i] = placement;
    }
}

// Hook Functions
int AroundWorld_OnMinorExit()
{
    // check if atw is enabled
    if (!(Scene_GetCurrentMajor() == MJRKIND_CITY && Gm_GetGameData()->city.mode == CITYMODE_TRIAL) ||
        CitySettings_SaveGet()->settings[CITYSETTING_SAVE_STADIUM] != STADSELECT_ALL)
        return 0;

    GameData *gd = Gm_GetGameData();
    gmDataAll *gm_data_all = (*stc_gmdataall);

    stadium_lineup_idx++;

    OSReport("stadium %d (%d/%d)\n", stadium_lineup[stadium_lineup_idx], stadium_lineup_idx, stadium_lineup_num);

    // get player placements from the last stadium
    s8 ranks[4];
    switch (Gm_GetStadiumGroupFromKind(gd->city.stadium_kind))
    {
    case (STGROUP_DRAGRACE):
    case (STGROUP_SINGLERACE):
    {
        AroundWorld_CalculateRanks(ranks, &gd->stadium_results.ply_race_time, 'i', 0);
        break;
    }
    case (STGROUP_AIRGLIDER):
    case (STGROUP_HIGHJUMP):
    {
        AroundWorld_CalculateRanks(ranks, &gd->stadium_results.ply_dist, 'f', 1);
        break;
    }
    case (STGROUP_TARGETFLIGHT):
    case (STGROUP_DESTRUCTION):
    case (STGROUP_MELEE):
    {
        AroundWorld_CalculateRanks(ranks, &gd->stadium_results.ply_points, 'i', 1);
        break;
    }
    }

    // update wins
    for (int i = 0; i < 4; i++)
    {
        int placement = ranks[i];

        if (placement == -1)
            continue;

        // int (*Ply_GetStadiumScore)(int ply) = (void *)0x8000b674;
        // int placement = Ply_GetStadiumScore(i);

        OSReport("ply %d placement: %d\n", i + 1, placement);

        if (placement == 0)
            stadium_wins[i]++;
    }

    if (stadium_lineup_idx >= stadium_lineup_num)
    {
        // spoof as DD so results screen uses points to rank players
        gd->city.stadium_kind = STKIND_DESTRUCTION1;
        gd->city_kind = gm_data_all->stadium_desc[gd->city.stadium_kind].city_kind;

        // store final win count
        for (int i = 0; i < 4; i++)
        {
            if (Ply_GetPKind(i) == PKIND_NONE)
                continue;

            int placement = 0;
            for (int j = 0; j < 4; j++)
            {
                if (j == i)
                    continue;

                if (stadium_wins[j] > stadium_wins[i])
                    placement++;
            }

            gd->stadium_results.ply_placement[i] = placement;
            gd->stadium_results.ply_points[i] = stadium_wins[i];

            // OSReport("ply %d points (%d) placement (%d)\n", i + 1, stadium_wins[i]);
        }

        gd->city.scene = 8;
        Scene_SetNextMinor(MNRKIND_CITYRESULT);
    }
    else
    {
        // get next stadium event
        StadiumKind stad_kind = stadium_lineup[stadium_lineup_idx];
        gd->city.stadium_kind = stad_kind;
        gd->city_kind = gm_data_all->stadium_desc[stad_kind].city_kind;

        // preload its files
        Preload_SetGrKind(gm_data_all->stadium_desc[stad_kind].gr_kind);
        Preload_Update();

        // load its sounds
        Gm_LoadGroundFGMBank(Gm_GetGrKindFromStageKind(Gm_GetCurrentStageKind()));

        gd->city.stadium_round = 0;
        gd->city.scene = 6;
        Scene_SetNextMinor(MNRKIND_STADIUMSPLASH);
    }

    return 1;
}
CODEPATCH_HOOKCONDITIONALCREATE(0x80040160, "", AroundWorld_OnMinorExit, "", 0, 0x800404a8)

///////////////////
// Apply Patches //
///////////////////

void AroundWorld_ApplyPatches()
{
    CODEPATCH_HOOKAPPLY(0x80040160);
}