
#include "text.h"
#include "os.h"
#include "hsd.h"
#include "preload.h"
#include "scene.h"
#include "inline.h"
#include "audio.h"
#include "obj.h"
#include "game.h"

#include "intang_after_ko.h"

#include "code_patch/code_patch.h"
#include "hoshi/settings.h"

int intang_after_ko_enabled = 1;
u8 intang_queue[5];

// init extra variables to determine if a player should receive intang next machine mount
void IntangAfterKO_On3DLoad()
{
    if (!intang_after_ko_enabled)
        return;

    // init queue
    for (int i = 0; i < GetElementsIn(intang_queue); i++)
        intang_queue[i] = 0;

    // add new callback to apply walk intang every frame post machine KO
    for (int i = 0; i < 4; i++)
    {
        if (Ply_GetPKind(i) == PKIND_NONE)
            continue;

        GObj_AddProc(Ply_GetRiderGObj(i), Rider_ApplyWalkIntang, RDPRI_HITCOLL - 1);
    }
}

// Queue intangibility when being KO'd
void Hook_QueueIntangOnKO(MachineData *md)
{
    if (!intang_after_ko_enabled ||
        !(Gm_IsInCity() && Gm_GetCityMode() == CITYMODE_TRIAL))
        return;

    int ply = Machine_GetRiderPly(md);
    intang_queue[ply] = 1;
}
CODEPATCH_HOOKCREATE(0x801e5758, "mr 3, 31\n\t", Hook_QueueIntangOnKO, "", 0)

// Grant intangibility when mounting a machine after being KO'd
void Hook_MachineIntangAfterKO(RiderData *rd)
{
    if (!intang_after_ko_enabled ||
        !rd->machine_gobj ||
        intang_queue[rd->ply] == 0)
        return;

    MachineData *md = rd->machine_gobj->userdata;
    Machine_GiveIntangibility(md, INTANG_TIME);
    intang_queue[rd->ply] = 0;
}
CODEPATCH_HOOKCREATE(0x801a8c68, "mr 3, 30\n\t", Hook_MachineIntangAfterKO, "", 0)

void Rider_ApplyWalkIntang(GOBJ *r)
{
    RiderData *rd = r->userdata;

    if (intang_queue[rd->ply] == 0)
        return;

    Rider_GiveIntangibility(rd, 1);
}

void IntangAfterKO_Init()
{
    CODEPATCH_HOOKAPPLY(0x801a8c68);
    CODEPATCH_HOOKAPPLY(0x801e5758);
}