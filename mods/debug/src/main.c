#include "os.h"
#include "hsd.h"

#include "debug.h"
#include "net.h"
#include "profiler.h"
#include "hoshi/settings.h"

char ModName[] = "Debug";
char ModAuthor[] = "UnclePunch";
char ModVersion[] = "v1.0";

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

void OnBoot(HSD_Archive *archive)
{
    // Profiler_Init();

    if ((*stc_dblevel) < DB_DEVELOP)
        return;

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
void OnSceneChange(HSD_Archive *archive)
{
    Debug_OnSceneChange();

    return;
}