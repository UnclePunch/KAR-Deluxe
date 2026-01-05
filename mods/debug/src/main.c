#include "hoshi/mod.h"
#include "hoshi/settings.h"

#include "hsd.h"

#include "debug.h"
#include "net.h"
#include "profiler.h"

extern int debug_enabled;

OptionDesc ModSettings = {
    .name = "Debug Mode",
    .description = "Enable debug functionality.",
    .on_change = Debug_ChangeSetting,
    .pri = MENUPRI_LOW,
    .kind = OPTKIND_VALUE,
    .val = &debug_enabled,
    .value_num = 2,
    .value_names = (char *[]){
        "Off",
        "On",
    },
};

void OnBoot()
{
    // Profiler_Init();

    if ((*stc_dblevel) < DB_DEVELOP)
        return;

    Debug_Init();

    // // output vanilla preload files
    // for (int i = 0; i < 82; i++)
    //     OSReport("file #%d (%p):\n  %s\n  kind: %d\n",
    //              i,
    //              &stc_preload_entry_descs[i],
    //              stc_preload_entry_descs[i].file_name,
    //              stc_preload_entry_descs[i].file_kind);

    // Net_Init();

    return;
}
void OnSceneChange()
{
    Debug_OnSceneChange();

    return;
}

void On3DLoadEnd()
{
    Debug_On3DLoadEnd();
}

ModDesc mod_desc = {
    .name = "Debug",
    .author = "UnclePunch",
    .version.major = 1,
    .version.minor = 0,
    .OnBoot = OnBoot,
    .OnSceneChange = OnSceneChange,
    .On3DLoadEnd = On3DLoadEnd,
};