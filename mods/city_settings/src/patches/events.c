#include "os.h"
#include "inline.h"
#include "event.h"

#include "../citysettings.h"

#include "code_patch/code_patch.h"

// Hook Functions
void EventFreq_Adjust()
{
    int val = CitySettings_SaveGet()->settings[CITYSETTING_SAVE_EVENTFREQ];

    // skip code if events are set to none (vanilla code handles this), or original (default behvaior)
    if (val == 0 || val == 2)
        return;

    static float freq_mult[] = {
        -1,
        1.5,
        -1,
        0.5,
        0,
    };

    EventCheckData *ep = (*stc_eventcheck_gobj)->userdata;
    ep->data->param->event_delay_min *= freq_mult[val];
    ep->data->param->event_delay_max *= freq_mult[val];
}
CODEPATCH_HOOKCREATE(0x800edc28, "", EventFreq_Adjust, "", 0)

void EventChance_Adjust(int *chance_arr)
{

    int val = CitySettings_SaveGet()->settings[CITYSETTING_SAVE_EVENTFREQ];
    GameData *gd = Gm_GetGameData();

    int whitelisted_events = CitySettings_SaveGet()->random_event_bitfield;
    int whitelisted_event_num = 0;

    // remove blacklisted events from the event pool
    for (int i = 0; i < EVKIND_NUM; i++)
    {
        if (!(whitelisted_events & (1 << i)))
            chance_arr[i] = 0;
        else
        {
            whitelisted_event_num++;
            // OSReport("ev_kind %d enabled with chance %d\n", i, chance_arr[i]);
        }
    }

    // vanilla game tracks the most recent 62.5% of total events
    // we allow users to effectively lower the total number of events,
    // so in that case we should intervene and track fewer recent events.
    EventCheckData *ed = (*stc_eventcheck_gobj)->userdata;
    int history_num = whitelisted_event_num * 0.625;
    if (ed->prev_kind_num > history_num)
        ed->prev_kind_num = history_num;
}
CODEPATCH_HOOKCREATE(0x800ede24, "addi 3, 1, 0x8\n\t", EventChance_Adjust, "", 0)

///////////////////
// Apply Patches //
///////////////////

void Events_ApplyPatches()
{
    CitySettingsSave *city_save = CitySettings_SaveGet();

    // event frequency
    CODEPATCH_HOOKAPPLY(0x800edc28);

    // repeat event
    if (city_save->settings[CITYSETTING_SAVE_EVENTREPEAT])
        CODEPATCH_REPLACEINSTRUCTION(0x800edf28, 0x48000034);
    else
        CODEPATCH_REPLACEINSTRUCTION(0x800edf28, 0x38c00000);

    CODEPATCH_HOOKAPPLY(0x800ede24);
}