#include "datatypes.h"
#include "hsd.h"
#include "game.h"
#include "enemy.h"
#include "item.h"
#include "rider.h"
#include <string.h>

u32 Hash_GameState(u32 kind)
{
    int object_num;

    typedef struct
    {
        u16 kind;
        u16 state;
        u16 frame;
        Vec3 pos;
        Vec3 forward;
    } ObjectState;
    
    typedef struct
    {
        int rng_seed;
        ObjectState objects[];
    } GameState;

    // count number of objects to backup
    object_num = 0;

    if (kind & (1 << GAMEPLINK_RIDER))
    {
        for (GOBJ *r = (*stc_gobj_lookup)[GAMEPLINK_RIDER]; r; r = r->next)
            object_num++;
    }

    if (kind & (1 << GAMEPLINK_MACHINE))
    {
        for (GOBJ *m = (*stc_gobj_lookup)[GAMEPLINK_MACHINE]; m; m = m->next)
        object_num++;
    }

    if (kind & (1 << GAMEPLINK_ENEMY))
    {
        for (GOBJ *e = (*stc_gobj_lookup)[GAMEPLINK_ENEMY]; e; e = e->next)
            object_num++;
    }

    if (kind & (1 << GAMEPLINK_ITEM))
    {
        for (GOBJ *i = (*stc_gobj_lookup)[GAMEPLINK_ITEM]; i; i = i->next)
        object_num++;
    }

    if (object_num == 0 && !(kind & (1 << GAMEPLINK_SYS)))
        return 0;

    // alloc temp buffer
    int state_size = sizeof(GameState) + sizeof(ObjectState) * object_num;
    GameState *state = HSD_MemAlloc(state_size);
    memset(state, 0, state_size);
    object_num = 0;

    if (kind & (1 << GAMEPLINK_SYS))
        state->rng_seed = *hsd_rand_seed;

    // begin collecting data
    if (kind & (1 << GAMEPLINK_RIDER))
    {
        for (GOBJ *g = (*stc_gobj_lookup)[GAMEPLINK_RIDER]; g; g = g->next)
        {
            RiderData *rd = g->userdata;
            ObjectState *this_state = &state->objects[object_num++];
            this_state->kind = rd->kind;
            this_state->state = rd->state_idx;
            this_state->frame = rd->state_frame;
            this_state->pos = rd->pos;
            this_state->forward = rd->forward;
        }
    }
    if (kind & (1 << GAMEPLINK_MACHINE))
    {
        for (GOBJ *g = (*stc_gobj_lookup)[GAMEPLINK_MACHINE]; g; g = g->next)
        {
            MachineData *gp = g->userdata;
            ObjectState *this_state = &state->objects[object_num++];
            this_state->kind = gp->kind;
            this_state->state = 0;
            this_state->frame = 0;
            this_state->pos = gp->pos;
            this_state->forward = gp->forward;
        }
    }
    if (kind & (1 << GAMEPLINK_ENEMY))
    {
        for (GOBJ *g = (*stc_gobj_lookup)[GAMEPLINK_ENEMY]; g; g = g->next)
        {
            EnemyData *gp = g->userdata;
            ObjectState *this_state = &state->objects[object_num++];
            this_state->kind = gp->kind;
            this_state->state = gp->state_idx;
            this_state->frame = gp->state_frame;
            this_state->pos = gp->pos;
            this_state->forward = gp->pos;
        }
    }
    if (kind & (1 << GAMEPLINK_ITEM))
    {
        for (GOBJ *g = (*stc_gobj_lookup)[GAMEPLINK_ITEM]; g; g = g->next)
        {
            ItemData *gp = g->userdata;
            ObjectState *this_state = &state->objects[object_num++];
            this_state->kind = gp->kind;
            this_state->state = gp->state;
            this_state->frame = gp->state_frame;
            this_state->pos = gp->pos;
            this_state->forward = gp->forward;
        }
    }

    u32 hash = hash_32(state, state_size);
    HSD_Free(state);

    return hash;
}
