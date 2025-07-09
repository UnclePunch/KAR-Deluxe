
#include "text.h"
#include "useful.h"
#include "hsd.h"
#include "preload.h"
#include "scene.h"
#include "inline.h"
#include "audio.h"
#include "obj.h"
#include "game.h"

#include "code_patch/code_patch.h"
#include "hoshi/settings.h"

int sd_as_ko_enabled = 1;

void Ply_AddDeathByAll(int ply, MachineData *md)
{
    // set dead by all other present players
    for (int i = 0; i < 4; i++)
    {
        if (Ply_GetPKind(i) == PKIND_NONE || i == ply)
            continue;

        md->dmg_log.attacker_ply = i; // spoof as killed by them

        Ply_AddDeath(ply, &md->dmg_log, md->is_bike, md->kind); // add death
    }

    md->dmg_log.attacker_ply = 5; // restore to 5 in case it happens again
}

// Award KO's upon falling
void Hook_OnFallDeath(MachineData *md)
{
    if (!sd_as_ko_enabled)
        return;

    if (!Gm_IsDestructionDerby())
        return;

    int ply = Machine_GetRiderPly(md);

    if (md->dmg_log.attacker_ply == 5)
        Ply_AddDeathByAll(ply, md);
    else
        Ply_AddDeath(ply, &md->dmg_log, md->is_bike, md->kind);
}
CODEPATCH_HOOKCREATE(0x801e658c, "mr 3, 31\n\t", Hook_OnFallDeath, "", 0)

// Award KO's upon receiving damage
int Hook_OnKOFromDamage(MachineData *md)
{
    if (!sd_as_ko_enabled)
        return 0;

    if (!Gm_IsDestructionDerby())
        return 0;

    // exit if they were killed by another player
    int ply = Machine_GetRiderPly(md);
    if (md->dmg_log.attacker_ply != 5 && md->dmg_log.attacker_ply != ply)
        return 0;

    // add a death from every other player
    Ply_AddDeathByAll(ply, md);
}
CODEPATCH_HOOKCONDITIONALCREATE(0x801e1f60, "mr 3, 30\n\t", Hook_OnKOFromDamage, "", 0, 0x801e1f78)

// Give full HP after respawning from a fall
void Hook_OnFallRespawn(int ply) // only runs during DD
{
    if (!sd_as_ko_enabled)
        return;

    GOBJ *m = Ply_GetMachineGObj(ply);
    if (m)
    {
        MachineData *mp = m->userdata;
        float hp_max = mp->hp_max;

        Ply_SetHP(ply, hp_max);
    }
}
CODEPATCH_HOOKCREATE(0x80010184, "mr 3, 28\n\t", Hook_OnFallRespawn, "", 0x80010190)

void SD_as_KO_Init()
{
    CODEPATCH_HOOKAPPLY(0x801e658c);
    CODEPATCH_HOOKAPPLY(0x801e1f60);
    CODEPATCH_HOOKAPPLY(0x80010184);
}