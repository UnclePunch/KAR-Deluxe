
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

#include "around_world.h"
#include "stadium.h"
#include "../citysettings.h"

#include "code_patch/code_patch.h"

/////////////////////
// Event Frequency //
/////////////////////

// Hook Functions
void StadiumChance_Adjust()
{
    CitySettingsSave *cs = CitySettings_SaveGet();

    // first check to init around the world mode
    if (cs->settings[CITYSETTING_SAVE_STADIUM] == STADSELECT_ALL)
    {
        AroundWorld_Init();
        return;
    }

    int random_stadium_bitfield = cs->random_stadium_bitfield;

    GameData *gd = Gm_GetGameData();
    gmDataAll *gm_data_all = *stc_gmdataall;

    int stadium_num = 0;
    struct StadiumChance stadium_chance[STKIND_NUM];

    // count number of whitelisted stadiums
    int whitelisted_stadium_num = 0;
    for (int i = 0; i < STKIND_NUM; i++)
    {
        if ((random_stadium_bitfield & (1 << i)))
            whitelisted_stadium_num++;
    }
    int history_buffer_size = (whitelisted_stadium_num > GetElementsIn(gd->city.prev_stadium_kind) ? GetElementsIn(gd->city.prev_stadium_kind) : (whitelisted_stadium_num - 1));

    // ensure 1 valid selection exists
    int stadium_selection = gd->city.menu_stadium_selection;
    int valid_stadium_exists = 0;
    for (int i = 0; i < STKIND_NUM; i++)
    {
        if (Gm_StadiumIsDefaultUnlocked(i) || Gm_StadiumIsUnlocked(i)) // ensure stadium is unlocked
        {
            if (stadium_selection == STADSELECT_SHUFFLE ||                                                              // shuffle enabled
                (stadium_selection >= STADSELECT_DRAGRACE && Gm_GetStadiumGroupFromKind(i) == (stadium_selection - 1))) // selected a type of stadium
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
            if (stadium_selection == 0) // shuffle
            {
                // ensure it hasnt appeared among the last X stadium events
                for (int j = 0; j < history_buffer_size; j++)
                {
                    if (gd->city.prev_stadium_kind[j] == i)
                    {
                        is_in_group = 0;
                        break;
                    }
                }
            }
            else // specific stadium group
            {
                // ensure this stadium is in our selected group
                if (Gm_GetStadiumGroupFromKind(i) != (stadium_selection - 1))
                    is_in_group = 0;
            }

            if (is_in_group &&
                (Gm_StadiumIsDefaultUnlocked(i) || Gm_StadiumIsUnlocked(i)))
            {
                stadium_chance[stadium_num].kind = i;
                stadium_chance[stadium_num].chance = gm_data_all->stadium_weights->weights[i];

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
        gd->city.prev_stadium_kind[i] = gd->city.prev_stadium_kind[i - 1];

    // store current stadium
    gd->city.prev_stadium_kind[0] = stadium_kind;
    gd->city.stadium_kind = stadium_kind;
}
char *StadiumName_Get(StadiumSelection stadium)
{
    static char *stadium_names[] = {
        "Shuffle",
        "All",
        "Drag Race",
        "Air Glider",
        "Target Flight",
        "High Jump",
        "Kirby Melee",
        "Destruction Derby",
        "Single Race",
        "VS King Dedede",
    };

    return stadium_names[stadium];
}
CODEPATCH_HOOKCREATE(0x8013c6b4, "mr 3,29\n\t", StadiumName_Get, "mr 5,3\n\t"
                                                                 "lfs 1, -0x6520 (2)\n\t"
                                                                 "lfs 2, -0x652c (2)\n\t",
                     0)
///////////////////
// Apply Patches //
///////////////////

void Stadium_ApplyPatches()
{
    CODEPATCH_REPLACEFUNC(0x8003f808, StadiumChance_Adjust); // stadium chances
    CODEPATCH_HOOKAPPLY(0x8013c6b4);                         //
}