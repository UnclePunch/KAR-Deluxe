
#include "text.h"
#include "os.h"
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
#include "profiler.h"
#include "hoshi/screen_cam.h"
#include "fst/fst.h"
#include "code_patch/code_patch.h"

static OSAlarm alarm;
Text *heap_text;
int heap_is_visible = 0;
int debug_enabled;

void Debug_OnSceneChange()
{
    GOBJ *g = GOBJ_EZCreator(0, 0, 0,
                             0, HSD_Free,
                             0, 0,
                             Debug_Think, 0,
                             0, 0, 0);

    // GOBJ_EZCreator(0, 0, 0,
    //                0, HSD_Free,
    //                0, 0,
    //                Net_Think, 0,
    //                0, 0, 0);

    // heap display
    Text *t = Hoshi_CreateScreenText();
    t->kerning = 0;
    t->use_aspect = 1;
    t->trans = (Vec3){10, 30, 0};
    t->viewport_scale = (Vec2){0.5, 0.5};
    t->aspect = (Vec2){560, 32};
    t->viewport_color = (GXColor){0, 0, 0, 128};
    Text_AddSubtext(t, 0, 0, "");
    heap_text = t;
}
void Debug_ChangeSetting(int val)
{
    if (val == 0)
        (*stc_dblevel) = DB_MASTER;
    else if (val == 1)
        (*stc_dblevel) = DB_DEVELOP;
}
void Debug_Think()
{
    int is_hide_heap = ((*stc_dblevel) < DB_DEVELOP) ? 1 : !heap_is_visible;

    // update text
    heap_text->hidden = is_hide_heap;

    if ((*stc_dblevel) >= DB_DEVELOP)
    {
        if (!heap_text->hidden)
        {
            PreloadHeap *ph = &stc_preload_heaps_lookup->heap_arr[0];
            Text_SetText(heap_text, 0, "%d: %.2fk \x81\x5e %.2fk", ph->heap_index, BytesToKB(OSCheckHeap(ph->heap_index)), BytesToKB(ph->size));
        }

        // output current state of all preloaded files
        if (Pad_GetHeld(20) & PAD_BUTTON_Y && Pad_GetDown(20) & PAD_BUTTON_DPAD_DOWN)
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
                else if ((ph->handle != (PreloadHandle *)-1))
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
        }

        if (Pad_GetHeld(20) & PAD_BUTTON_Y && Pad_GetDown(20) & PAD_BUTTON_DPAD_UP)
            heap_is_visible ^= 1;
    }
}

// void Rider_OnDeath(RiderData *rd)
// {
//     OSReport("p%d just died on their machine\n", rd->ply + 1);
// }
// CODEPATCH_HOOKCREATE(0x801a06d0, "mr 3, 31\n\t", Rider_OnDeath, "", 0)


int timer = 0;
void Debug3D_Think(GOBJ *g)
{
    if (++timer >= 120)
    {
        timer = 0;

        // make all compact stars play a sound
        for (GOBJ *g = (*stc_gobj_lookup)[GAMEPLINK_MACHINE]; g; g = g->next)
        {
            MachineData *md = g->userdata;
            int audio_source = Machine_AllocAudioSource(128);
            int audio_track = AudioTrack_Alloc();

            AudioSource_SetPosition(audio_source, &md->pos, 0);
            AudioSource_InitUnk(audio_source);
            AudioSource_Play(0x130025, audio_track, audio_source);
            OSReport("played sound with source %d and track %d %p\n", audio_source, audio_track, &audio_source_table->sources[audio_source]);
                
            // free it
            if (AudioSource_CheckUnk(audio_source) == 0)
            {
                AudioSource_Free(audio_source);
                AudioTrack_Free(audio_track);
            }
            else
                OSReport("ERROR: IT DIDNT WANT TO FREE\n");

            ItemDesc desc;
            Vec3 pos = {
                .X = md->pos.X + md->forward.X * 30, 
                .Y = md->pos.Y + md->forward.Y * 30,
                .Z = md->pos.Z + md->forward.Z * 30, 
            };
            Item_InitDesc(&desc, ITKIND_ALLUP, 1.0, 0, &pos, &(Vec3){0,1,0}, &md->forward, -1, -1);
            desc.is_airborne = 1;
            Item_Create(&desc);
        }
    }
}
void Debug_On3DLoadEnd()
{
    return;
    
    GOBJ_EZCreator(0,0,0,
                    0, 0,
                    0, 0,
                    Debug3D_Think, 10,
                    0, 0, 0);
    timer = 0;
}

void Debug_Init()
{
    // CODEPATCH_HOOKAPPLY(0x801a06d0);
}