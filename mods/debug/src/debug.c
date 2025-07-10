
#include "text.h"
#include "useful.h"
#include "hsd.h"
#include "preload.h"
#include "scene.h"
#include "inline.h"
#include "audio.h"
#include "game.h"
#include <stdint.h>
#include <stddef.h>

#include "debug.h"
#include "net.h"
#include "fst/fst.h"
#include "code_patch/code_patch.h"
#include "hoshi/settings.h"

char ModName[] = "Debug";
char ModAuthor[] = "UnclePunch";
char ModVersion[] = "v1.0";

static OSAlarm alarm;
int debug_enabled;

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

    if ((*stc_dblevel) < DB_DEBUG_DEVELOP)
        return;

    // output vanilla preload files
    for (int i = 0; i < 82; i++)
        OSReport("file #%d (%p):\n  %s\n  kind: %d\n",
                 i,
                 &stc_preload_entry_descs[i],
                 stc_preload_entry_descs[i].file_name,
                 stc_preload_entry_descs[i].file_kind);

    Net_Init();

    return;
}
void OnSceneChange(HSD_Archive *archive)
{
    if ((*stc_dblevel) < DB_DEBUG_DEVELOP)
        return;

    GOBJ *g = GOBJ_EZCreator(0, 0, 0,
                             0, HSD_Free,
                             0, 0,
                             Debug_Think, 0,
                             0, 0, 0);

    GOBJ_EZCreator(0, 0, 0,
                   0, HSD_Free,
                   0, 0,
                   Net_Think, 0,
                   0, 0, 0);

    return;
}

void Debug_ChangeSetting(int val)
{
    if (val == 0)
        *stc_dblevel = DB_MASTER;
    else if (val == 1)
        *stc_dblevel = DB_DEVELOP;
}
void Debug_Think()
{
    if (Pad_GetDown(20) & PAD_BUTTON_DPAD_DOWN)
    {
        Preload *p = stc_preload_table;

        OSReport("~~~~~~~~~~~~~~~~~~~~~~~~~\n");
        OSReport("[Preload Data]\n\n");

        OSReport("Kind: %d\n", p->kind);

        OSReport("\nHeap Data (%p):\n", &stc_preload_heaps_lookup->heap_arr);
        for (int i = 0; i < GetElementsIn(stc_preload_heaps_lookup->heap_arr); i++)
        {
            PreloadHeap *ph = &stc_preload_heaps_lookup->heap_arr[i];

            int free = 0;
            if (ph->heap_index != -1)
                free = OSCheckHeap(ph->heap_index);
            else if ((ph->handle != -1))
            {
                PreloadHandle *handle = ph->handle;

                uintptr_t heap_start = (uintptr_t)handle->heap_start;
                uintptr_t heap_end = (uintptr_t)handle->heap_end;

                PreloadAlloc *curr = handle->alloc;

                if (!curr)
                {
                    // No allocations at all: full heap is free
                    free = heap_end - heap_start;
                }
                else
                {
                    // Space before first allocation
                    uintptr_t first_block_start = (uintptr_t)curr->start;
                    if (first_block_start > heap_start)
                    {
                        free += first_block_start - heap_start;
                    }

                    // Space between allocations
                    while (curr && curr->next)
                    {
                        uintptr_t curr_end = (uintptr_t)curr->start + curr->size;
                        uintptr_t next_start = (uintptr_t)curr->next->start;

                        if (next_start > curr_end)
                            free += next_start - curr_end;

                        curr = curr->next;
                    }

                    // Space after last allocation
                    if (curr)
                    {
                        uintptr_t last_end = (uintptr_t)curr->start + curr->size;
                        if (heap_end > last_end)
                            free += heap_end - last_end;
                    }
                }
            }

            OSReport("Heap %d (%p):\n  %s\n  addr_start: %x\n  mem: %.2fkb / %.2fkb\n  pri: %d\n\n",
                     i,
                     ph,
                     ph->is_disabled ? "Disabled" : "Enabled",
                     ph->addr_start,
                     BytesToKB(free),
                     BytesToKB(ph->size),
                     ph->pri);

            // output files in this heap
            for (int j = 0; j < GetElementsIn(p->entry); j++)
            {
                PreloadEntry *pe = &p->entry[j];

                if (pe->heap_kind == i &&
                    pe->load_state > PRELOADENTRYSTATE_NONE)
                {
                    // get filename
                    char *file_name;
                    if (pe->file_kind == PRELOADFILEKIND_ALLOC)
                        file_name = "Alloc";
                    else
                        file_name = FST_GetFilenameFromEntrynum(pe->file_entry_num);

                    static char *load_state_names[] = {
                        "NONE",
                        "QUEUED",
                        "LOADING",
                        "LOADED",
                        "ACCESSED",
                        "5",
                        "6",
                    };

                    OSReport("\t%s (%p)\n\tStatus: %s\n\tLoad State: %s\n\tSize: %.2fkb\n\tPtr: %p\n\n",
                             file_name,
                             pe,
                             (pe->status > 0) ? "Needed" : "Unneeded",
                             load_state_names[p->entry[i].load_state],
                             BytesToKB(pe->file_size),
                             pe->file_data);
                }
            }
        }

        OSReport("\n");
        OSReport("~~~~~~~~~~~~~~~~~~~~~~~~~\n");

        SFX_Play(FGMMENU_CS_KETTEI);
    }
}
