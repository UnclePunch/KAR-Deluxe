
#include "os.h"
#include "inline.h"
#include "game.h"
#include "item.h"

#include "../citysettings.h"
#include "around_world.h"

#include "code_patch/code_patch.h"

static u8 stc_is_box_enabled[BOXKIND_NUM];

void ItemToggle_Apply()
{
    CitySettingsSave *city_save = CitySettings_SaveGet();
    grBoxGeneObj *box_gene_obj = (*stc_grBoxGeneObj);

    // add/remove items to reflect its toggle status
    for (int it_kind = ITKIND_ACCEL; it_kind <= ITKIND_GORDO; it_kind++)
    {
        int is_enabled = ((city_save->random_item_bitfield & (1ULL << it_kind)) != 0);
        BoxKind this_box_kind = Item_GetCommonAttr(it_kind)->box_kind;

        // OSReport("item %d toggle: %d\n", it_kind, is_enabled);

        for (int box_kind = BOXKIND_BLUE; box_kind < BOXKIND_NUM; box_kind++) // search for it in each box's spawn table
        {
            // check if item belongs in this box?
            if (this_box_kind == box_kind)
            {
                int item_found = 0;

                for (int i = 0; i < box_gene_obj->item_group_spawn[box_kind].num; i++) // check this box's spawn table
                {
                    if (box_gene_obj->item_group_spawn[box_kind].it_kind[i] == it_kind) // check for the item
                    {
                        if (!is_enabled) // item is disabled
                        {
                            //  OSReport("removing item %d from box_kind %d\n", it_kind, this_box_kind);

                            // remove it and shift ones below it up
                            for (int j = i; j < box_gene_obj->item_group_spawn[box_kind].num - 1; j++)
                            {
                                box_gene_obj->item_group_spawn[box_kind].it_kind[j] = box_gene_obj->item_group_spawn[box_kind].it_kind[j + 1];
                                box_gene_obj->item_group_spawn[box_kind].chance[j] = box_gene_obj->item_group_spawn[box_kind].chance[j + 1];
                            }

                            // zero out last entry for neatness
                            box_gene_obj->item_group_spawn[box_kind].it_kind[box_gene_obj->item_group_spawn[box_kind].num] = 0;
                            box_gene_obj->item_group_spawn[box_kind].chance[box_gene_obj->item_group_spawn[box_kind].num] = 0;
                            box_gene_obj->item_group_spawn[box_kind].num--;
                        }

                        item_found = 1;
                        break;
                    }
                }

                if (!item_found && is_enabled)
                {
                    // get its vanilla spawn rate
                    grBoxGeneInfo *box_gene_info = (*stc_grBoxGeneInfo);
                    int spawn_chance = 4;
                    for (int i = 0; i < box_gene_info->item_desc->item_spawn_num; i++)
                    {
                        if (box_gene_info->item_desc->item_spawn[i].it_kind == ITKIND_ALLUP)
                        {
                            spawn_chance = box_gene_info->item_desc->item_spawn[i].fall_chance[Gm_GetCurrentStadiumGroup()];
                            break;
                        }
                    }

                    // OSReport("adding item %d to box_kind %d with change %d\n", it_kind, this_box_kind, spawn_chance);

                    // add it
                    box_gene_obj->item_group_spawn[box_kind].it_kind[box_gene_obj->item_group_spawn[box_kind].num] = it_kind;
                    box_gene_obj->item_group_spawn[box_kind].chance[box_gene_obj->item_group_spawn[box_kind].num] = spawn_chance;
                    box_gene_obj->item_group_spawn[box_kind].num++;
                }
            }
        }
    }
}
void AllUp_AdjustSpawn()
{

    grBoxGeneObj *box_gene_obj = (*stc_grBoxGeneObj);

    CitySettingsSave *city_save = CitySettings_SaveGet();
    if (city_save->settings[CITYSETTING_SAVE_ALLUP] == 1)
    {

        // find the vs dedede all up spawn rate
        grBoxGeneInfo *box_gene_info = (*stc_grBoxGeneInfo);
        int all_up_rate = 0;
        for (int i = 0; i < box_gene_info->item_desc->item_spawn_num; i++)
        {
            if (box_gene_info->item_desc->item_spawn[i].it_kind == ITKIND_ALLUP)
            {
                all_up_rate = box_gene_info->item_desc->item_spawn[i].fall_chance[STGROUP_VSKINGDEDEDE];
                break;
            }
        }

        // search for all up in the current spawn table
        int is_allup_present = 0;
        for (int i = 0; i < box_gene_obj->item_group_spawn[BOXKIND_BLUE].num; i++)
        {
            if (box_gene_obj->item_group_spawn[BOXKIND_BLUE].it_kind[i] == ITKIND_ALLUP)
            {
                if (box_gene_obj->item_group_spawn[BOXKIND_BLUE].chance[i] == 0)
                    box_gene_obj->item_group_spawn[BOXKIND_BLUE].chance[i] = all_up_rate;

                is_allup_present = 1;

                break;
            }
        }

        // if not present, add it
        if (!is_allup_present)
        {
            // add it to the current table
            box_gene_obj->item_group_spawn[BOXKIND_BLUE].it_kind[box_gene_obj->item_group_spawn[BOXKIND_BLUE].num] = ITKIND_ALLUP;
            box_gene_obj->item_group_spawn[BOXKIND_BLUE].chance[box_gene_obj->item_group_spawn[BOXKIND_BLUE].num] = all_up_rate;
            box_gene_obj->item_group_spawn[BOXKIND_BLUE].num++;
        }
    }
}
void LegendaryPiece_AdjustSpawn()
{
    grBoxGeneInfo *box_gene_info = (*stc_grBoxGeneInfo);

    if (box_gene_info)
    {
        CitySettingsSave *city_save = CitySettings_SaveGet();

        // adjust legendary peice spawn rate
        static u8 spawn_chances[] = {0, 30, 100};
        box_gene_info->item_desc->legendary_pieces[0].chance_to_spawn = spawn_chances[city_save->settings[CITYSETTING_SAVE_DRAGOONSPAWN]];
        box_gene_info->item_desc->legendary_pieces[1].chance_to_spawn = spawn_chances[city_save->settings[CITYSETTING_SAVE_HYDRASPAWN]];
    }
}
void Box_CheckIfEnabled()
{
    grBoxGeneObj *box_gene_obj = (*stc_grBoxGeneObj);
    itCommonDataAll *it_common = (*stc_it_common_data);
    if (!box_gene_obj || !it_common)
        return;

    // init all
    for (int i = 0; i < BOXKIND_NUM; i++)
        stc_is_box_enabled[i] = 0;

    // ensure at least one item from each box is enabled
    for (BoxKind box_kind = 0; box_kind < BOXKIND_NUM; box_kind++)
    {
        for (int i = 0; i < box_gene_obj->item_group_spawn[box_kind].num; i++)
        {
            if (box_gene_obj->item_group_spawn[box_kind].chance[i] > 0)
            {
                // OSReport("box %d enabled\n", box_kind);
                stc_is_box_enabled[box_kind] = 1;
                break;
            }
        }
    }
}

////////////////////
// Item Frequency //
////////////////////

static float stc_item_freq_mult[] = {
    0,   // None
    0.5, // Low
    1.0, // Original
    1.5, // High
    3.0, // Very High
};
void ItemFreq_AdjustFallDesc(ItemFallDesc *desc, float mult)
{
    if (mult == 0)
    {
        desc->item_max = 0;
        desc->spawn_time_min = 0;
        desc->spawn_time_max = 0;
    }
    else
    {
        desc->item_max *= mult;
        desc->spawn_time_min /= mult;
        desc->spawn_time_max /= mult;
    }
}
void ItemFreq_AdjustEventFallDesc()
{
    grBoxGeneInfo *box_gene_info = (*stc_grBoxGeneInfo);

    int val = CitySettings_SaveGet()->settings[CITYSETTING_SAVE_ITEMFREQ];
    ItemFreq_AdjustFallDesc(&box_gene_info->cur_event_fall_desc, stc_item_freq_mult[val]);
}
void ItemFreq_Adjust()
{
    grBoxGeneInfo *box_gene_info = (*stc_grBoxGeneInfo);
    if (box_gene_info)
    {
        CitySettingsSave *city_save = CitySettings_SaveGet();

        float mult = stc_item_freq_mult[CitySettings_SaveGet()->settings[CITYSETTING_SAVE_ITEMFREQ]];
        for (int i = 0; i < box_gene_info->fall_timer_desc->desc_num; i++)
            ItemFreq_AdjustFallDesc(&box_gene_info->fall_timer_desc->desc[i], mult);

        (*stc_item_param2)->max_items *= mult;
    }
}

////////////////////
// Hook Functions //
////////////////////

void Hook_BoxGene_Init()
{
    if (Gm_IsGrKindCity(Gm_GetCurrentGrKind()))
    {
        LegendaryPiece_AdjustSpawn();
        ItemFreq_Adjust();
    }
}
void Hook_ItemFall_Init()
{
    AllUp_AdjustSpawn();
    ItemToggle_Apply();
    AroundWorld_EqualizeItemChances();
}
void Hook_ItemFall_ReInit()
{
    ItemFreq_AdjustEventFallDesc();

    AllUp_AdjustSpawn();
    ItemToggle_Apply();
    AroundWorld_EqualizeItemChances();
}
CODEPATCH_HOOKCREATE(0x800ec4b4, "", Hook_BoxGene_Init, "", 0)
CODEPATCH_HOOKCREATE(0x800eb558, "", Hook_ItemFall_Init, "", 0)
CODEPATCH_HOOKCREATE(0x800ed7f0, "", Hook_ItemFall_ReInit, "", 0)

///////////////////////////
// Replacement Functions //
///////////////////////////

int Hook_DetermineBoxType(BoxKind *box_col_kind, int *box_size_kind)
{
    /*
    This is a rewrite of the vanilla game function, in order to support the ability to
    blacklist spawning boxes of a certain type (when no items inside of them are enabled).
    */

    grBoxGeneInfo *box_gene = (*stc_grBoxGeneInfo);
    if (!box_gene)
        return 0;

    u8(*box_chances)[3] = (u8(*)[3])box_gene->item_desc->box_spawn_chances;

    // add up box chances, removing any boxes that contain 0 valid items
    int chance_total = 0;
    for (int box_kind = 0; box_kind < BOXKIND_NUM; box_kind++)
    {
        if (stc_is_box_enabled[box_kind])
        {
            for (int box_size = 0; box_size < 3; box_size++)
                chance_total += box_chances[box_kind][box_size];
        }
    }

    // unwrap and decide on a box to spawn
    int rand_chance = HSD_Randi(chance_total);
    chance_total = 0;
    for (BoxKind box_kind = 0; box_kind < BOXKIND_NUM; box_kind++)
    {
        if (stc_is_box_enabled[box_kind])
        {
            for (int box_size = 0; box_size < 3; box_size++)
            {
                chance_total += box_chances[box_kind][box_size];

                if (chance_total > rand_chance)
                {
                    *box_col_kind = box_kind;
                    *box_size_kind = box_size;

                    return box_kind;
                }
            }
        }
    }
}

///////////////////
// Apply Patches //
///////////////////

void Items_ApplyPatches()
{
    // item frequency
    CODEPATCH_HOOKAPPLY(0x800ec4b4);                      // injection to adjust item spawn rates
    CODEPATCH_HOOKAPPLY(0x800ed7f0);                      // injection to adjust events item spawn rates
    CODEPATCH_HOOKAPPLY(0x800eb558);                      // injection to adjust all up spawn rates
    CODEPATCH_REPLACEINSTRUCTION(0x800ea984, 0x4800000c); // remove hardcoded 4 frame spawn interval
    CODEPATCH_REPLACEINSTRUCTION(0x8024ef24, 0x48000084); // remove hardcoded 100 item limit for sky spawn
    CODEPATCH_REPLACEINSTRUCTION(0x80250c20, 0x38600001); // remove hardcoded 100 item limit for box spawn

    CODEPATCH_REPLACEFUNC(0x800ebc04, Hook_DetermineBoxType); //
}