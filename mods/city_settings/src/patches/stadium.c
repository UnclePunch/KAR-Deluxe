
#include "text.h"
#include "useful.h"
#include "hsd.h"
#include "preload.h"
#include "scene.h"
#include "inline.h"
#include "audio.h"
#include "game.h"
#include "item.h"
#include "string.h"

#include "../citysettings.h"

#include "code_patch/code_patch.h"

/////////////////////
// Event Frequency //
/////////////////////

// Hook Functions
void StadiumChance_Adjust(int *chance_arr)
{
    CitySettingsSave *cs = CitySettings_SaveGet();

    int random_stadium_bitfield = cs->random_stadium_bitfield;

    GameData *gd = Gm_GetGameData();
    gmDataAll *gm_data_all = *stc_gmdataall;

    struct StadiumChance
    {
        StadiumKind kind;
        int chance;
    };

    // create array of stadium kinds and chances
    int stadium_num = 0;
    struct StadiumChance stadium_chance[STKIND_NUM];

    // count number of whitelisted stadiums
    int whitelisted_stadium_num = 0;
    for (int i = 0; i < STKIND_NUM; i++)
    {
        if ((random_stadium_bitfield & (1 << i)))
            whitelisted_stadium_num++;
    }
    int history_buffer_size = (whitelisted_stadium_num > GetElementsIn(gd->prev_stadium_kind) ? GetElementsIn(gd->prev_stadium_kind) : (whitelisted_stadium_num - 1));

    // ensure 1 valid selection exists
    int valid_stadium_exists = 0;
    for (int i = 0; i < STKIND_NUM; i++)
    {

        if (Gm_StadiumIsDefaultUnlocked(i) || Gm_StadiumIsUnlocked(i)) // ensure stadium is unlocked
        {
            if (gd->menu_stadium_selection == 0 ||                                                             // shuffle enabled
                (gd->menu_stadium_selection > 0 && Gm_GetStadiumGroup(i) == (gd->menu_stadium_selection - 1))) // selected a type of stadium
            {
                // is whitelisted
                if (random_stadium_bitfield & (1 << i))
                {
                    valid_stadium_exists = 1;
                    break;
                }
            }
        }
    }

    // if none exist, spoof as all enabled
    if (!valid_stadium_exists)
        random_stadium_bitfield = -1;

    // create array of valid stadium events
    int chance_total = 0;
    for (int i = 0; i < STKIND_NUM; i++)
    {
        int is_in_group = 1;

        // is whitelisted
        if (random_stadium_bitfield & (1 << i))
        {
            if (gd->menu_stadium_selection == 0) // shuffle
            {
                // ensure it hasnt appeared among the last X stadium events
                for (int j = 0; j < history_buffer_size; j++)
                {
                    if (gd->prev_stadium_kind[j] == i)
                    {
                        is_in_group = 0;
                        break;
                    }
                }
            }
            else // specific stadium group
            {
                // ensure this stadium is in our selected group
                if (Gm_GetStadiumGroup(i) != (gd->menu_stadium_selection - 1))
                    is_in_group = 0;
            }

            if (is_in_group &&
                (Gm_StadiumIsDefaultUnlocked(i) || Gm_StadiumIsUnlocked(i)))
            {
                stadium_chance[stadium_num].kind = i;
                stadium_chance[stadium_num].chance = gm_data_all->stadium->weights[i];

                chance_total += stadium_chance[stadium_num].chance;

                stadium_num++;
            }
        }
    }

    // determine random stadium
    int random_chance = HSD_Randi(chance_total);
    int stadium_kind = -1;
    chance_total = 0;
    for (int i = 0; i < stadium_num; i++)
    {
        chance_total += stadium_chance[i].chance;

        if (random_chance < chance_total)
        {
            stadium_kind = stadium_chance[i].kind;
            break;
        }
    }

    // shift history
    for (int i = history_buffer_size - 1; i > 0; i--)
        gd->prev_stadium_kind[i] = gd->prev_stadium_kind[i - 1];

    // store current stadium
    gd->prev_stadium_kind[0] = stadium_kind;
    gd->stadium_kind = stadium_kind;
}

///////////////////
// Apply Patches //
///////////////////

void Stadium_ApplyPatches()
{
    // CitySettingsSave *city_save = CitySettings_SaveGet();

    // stadium chances
    CODEPATCH_REPLACEFUNC(0x8003f808, StadiumChance_Adjust);
}