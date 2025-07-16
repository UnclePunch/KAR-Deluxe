#include "os.h"
#include "inline.h"
#include "game.h"

#include "../citysettings.h"

#include "code_patch/code_patch.h"

static char *stc_vckind_names[] = {
    "VCKIND_WARP",
    "VCKIND_COMPACT",
    "VCKIND_WINGED",
    "VCKIND_SHADOW",
    "VCKIND_HYDRA",
    "VCKIND_BULK",
    "VCKIND_SLICK",
    "VCKIND_FORMULA",
    "VCKIND_DRAGOON",
    "VCKIND_WAGON",
    "VCKIND_ROCKET",
    "VCKIND_SWERVE",
    "VCKIND_TURBO",
    "VCKIND_JET",
    "VCKIND_FLIGHT",
    "VCKIND_FREE",
    "VCKIND_STEER",
    "VCKIND_WINGKIRBY",
    "VCKIND_WINGMETAKNIGHT",
    "VCKIND_WHEELNORMAL",
    "VCKIND_WHEELKIRBY",
    "VCKIND_WHEELIEBIKE",
    "VCKIND_REXWHEELIE",
    "VCKIND_WHEELIESCOOTER",
    "VCKIND_WHEELDEDEDE",
    "VCKIND_WHEELVSDEDEDE",
};

static CharacterKind stc_random_character = -1;
static u8 stc_menu_to_character[] = {
    CKIND_COMPACT,
    CKIND_WARP,
    CKIND_FLIGHT,
    CKIND_WAGON,
    CKIND_SWERVE,
    CKIND_WINGED,
    CKIND_SHADOW,
    CKIND_BULK,
    CKIND_ROCKET,
    CKIND_JET,
    CKIND_TURBO,
    CKIND_SLICK,
    CKIND_FORMULA,
    CKIND_HYDRA,
    CKIND_DRAGOON,
    CKIND_WHEELIEBIKE,
    CKIND_WHEELIESCOOTER,
    CKIND_REXWHEELIE,
};
static u8 stc_vckind_to_character[] = {
    CKIND_WARP,
    CKIND_COMPACT,
    CKIND_WINGED,
    CKIND_SHADOW,
    CKIND_HYDRA,
    CKIND_BULK,
    CKIND_SLICK,
    CKIND_FORMULA,
    CKIND_DRAGOON,
    CKIND_WAGON,
    CKIND_ROCKET,
    CKIND_SWERVE,
    CKIND_TURBO,
    CKIND_JET,
    CKIND_FLIGHT,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    CKIND_WHEELIEBIKE,
    CKIND_REXWHEELIE,
    CKIND_WHEELIESCOOTER,
    -1,
    -1,
};
CharacterKind Machines_MenuIDToCharacterKind(int menu_id)
{
    return stc_menu_to_character[menu_id];
}
CharacterKind Machines_RollRandom()
{
    CitySettingsSave *cs = CitySettings_SaveGet();
    int whitelisted_machines = cs->random_machine_bitfield;
    u8 whitelist_lookup[VCKIND_NUM];

    // count amount of enabled vehicles
    int whitelisted_vehicle_num = 0;
    for (int i = 0; i < VCKIND_NUM; i++)
    {
        if ((whitelisted_machines & (1 << i)))
        {
            whitelist_lookup[whitelisted_vehicle_num] = i;
            whitelisted_vehicle_num++;
        }
    }

    if (whitelisted_vehicle_num > 0)
    {
        int vc_kind = whitelist_lookup[HSD_Randi(whitelisted_vehicle_num)];
        return stc_vckind_to_character[vc_kind];
    }
    else
        return Machines_MenuIDToCharacterKind(HSD_Randi(GetElementsIn(stc_menu_to_character)));
}

// Starting Machine
void Machines_AdjustStarting()
{

    GameData *gd = Gm_GetGameData();

    // ensure its main city trial mode
    if (gd->city.mode == CITYMODE_TRIAL)
    {
        CitySettingsSave *cs = CitySettings_SaveGet();

        for (int i = 0; i < GetElementsIn(gd->ply_data); i++)
        {
            CharacterKind ckind;
            switch (cs->settings[CITYSETTING_SAVE_MACHINESTART])
            {
            // single random machine
            case (18):
            {
                ckind = stc_random_character;

                // OSReport("starting all on machine_kind %d\n", Character_GetDesc(ckind)->machine);
                break;
            }
            // all random machine
            case (19):
            {
                ckind = Machines_RollRandom();
                break;
            }
            default:
            {
                ckind = Machines_MenuIDToCharacterKind(cs->settings[CITYSETTING_SAVE_MACHINESTART]);
                break;
            }
            }

            CharacterDesc *c_desc = Character_GetDesc(ckind);

            gd->ply_data[i].machine_kind = c_desc->machine;
            gd->ply_data[i].is_bike = c_desc->is_bike;
            gd->ply_data[i].rider_kind = c_desc->rider_kind;
            gd->city.machine_kind[i] = c_desc->machine;
            gd->city.is_bike[i] = c_desc->is_bike;
        }
    }
    return;
}
CODEPATCH_HOOKCREATE(0x8002df5c, "", Machines_AdjustStarting, "", 0)

// Machine Max Speed Patch
void Machines_AdjustMaxSpeed(MachineData *md)
{
    if (CitySettings_SaveGet()->settings[CITYSETTING_SAVE_MACHINESPEED] == 0)
        return;

    switch (md->is_bike)
    {
    case (0):
    {
        md->attr.star->max_speed = (*stc_vcDataKindStar)->attr->max_speed * 1.5;

        // OSReport("val %f @ %p\n",
        //          md->attr.star->max_speed,
        //          &md->attr.star->max_speed);

        break;
    }
    case (1):
    {
        md->attr.wheel->handling.max_speed = md->vcData->handling_attr->max_speed * 1.5;

        // OSReport("val %f @ %p\n",
        //          md->attr.wheel->handling.max_speed,
        //          &md->attr.wheel->handling.max_speed);

        break;
    }
    }
}
CODEPATCH_HOOKCREATE(0x801c73b4, "mr 3,31\n\t", Machines_AdjustMaxSpeed, "", 0)

// Machine Spawn Rate Patch
int Machines_AdjustSpawnChance(MachineSpawnData *msd, float match_progress)
{
    CitySettingsSave *cs = CitySettings_SaveGet();
    vcDataCommon *vc_data_common = (*stc_vcDataCommon);

    int whitelisted_machines = cs->random_machine_bitfield;

    // get vehicle spawns for the current point in the match
    int spawn_table_idx = 0;
    while (match_progress > vc_data_common->x20->spawn_desc[spawn_table_idx].match_progress)
    {
        spawn_table_idx++;
    }

    // make local copy of spawn table
    float spawn_chances[VCKIND_NUM];
    memcpy(&spawn_chances, &vc_data_common->x20->spawn_desc[spawn_table_idx].chance, sizeof(spawn_chances));

    // count amount of enabled vehicles
    int whitelisted_vehicle_num = 0;
    for (int i = 0; i < VCKIND_NUM; i++)
    {
        if ((whitelisted_machines & (1 << i)))
            whitelisted_vehicle_num++;
    }

    if (whitelisted_vehicle_num > 0)
    {
        // modify spawn table based on whitelist
        for (int i = 0; i < GetElementsIn(spawn_chances); i++)
        {
            if (!(whitelisted_machines & (1 << i))) // is disabled, zero it out
                spawn_chances[i] = 0;               //
            else if (spawn_chances[i] == 0)         // is enabled and zero'd, give it some chance
                spawn_chances[i] = 10;
        }
    }

    int history_size = (whitelisted_vehicle_num <= 4) ? (whitelisted_vehicle_num - 1) : 4;

    // remove recently spawned vehicles
    for (int i = 0; i < GetElementsIn(spawn_chances); i++)
    {
        for (int j = 0; j < history_size; j++)
        {
            if (i == msd->prev_machine_kind[j])
                spawn_chances[i] = 0;
        }
    }

    // determine machine to spawn
    int machine_kind;
    float chance_total = 0;
    for (int i = 0; i < GetElementsIn(spawn_chances); i++)
        chance_total += spawn_chances[i];
    float random_chance = HSD_Randf() * chance_total;
    chance_total = 0;
    for (int i = 0; i < GetElementsIn(spawn_chances); i++)
    {
        chance_total += spawn_chances[i];
        if (random_chance < chance_total)
        {
            machine_kind = i;
            break;
        }
    }

    // update buffer
    if (history_size > 0)
    {
        msd->prev_machine_kind[msd->prev_machine_index] = machine_kind;
        msd->prev_machine_index++;
        if (msd->prev_machine_index >= history_size)
            msd->prev_machine_index = 0;
    }

    return machine_kind;
}
CODEPATCH_HOOKCREATE(0x801df00c, "mr 3,30\n\t", Machines_AdjustSpawnChance, "mr 31,3\n\t", 0x801df220)
CODEPATCH_HOOKCREATE(0x801df44c, "mr 3,30\n\t", Machines_AdjustSpawnChance, "mr 31,3\n\t", 0x801df630)

// Respawn Patches
int MachineRespawn_CheckIfEnabledInCity(RiderData *rd)
{
    GameData *gd = Gm_GetGameData();
    CitySettingsSave *cs = CitySettings_SaveGet();

    // ensure its main city trial mode
    if (Gm_IsInCity() &&                 // playing city trial
        gd->city.mode == CITYMODE_TRIAL) // in the main mode
    {
        // change our machine to the one we started with
        int machine_kind = gd->city.machine_kind[rd->ply];
        int is_bike = gd->city.is_bike[rd->ply];

        // OSReport("spawning p%d on machine_kind %d\n", rd->ply + 1, gd->machine_kind[rd->ply]);
        Ply_SetMachineKind(rd->ply, machine_kind);
        Ply_SetMachineIsBike(rd->ply, is_bike);

        // signal to exit injected function
        return 1;
    }
    else
        return 0;
}
CODEPATCH_HOOKCONDITIONALCREATE(0x801952c8, "mr 3,31\n\t", MachineRespawn_CheckIfEnabledInCity, "", 0, 0x801952e0)
int MachineRespawn_CheckToRespawnRider(RiderData *rd)
{
    // this hook runs after destruction derby check fails

    GameData *gd = Gm_GetGameData();
    CitySettingsSave *cs = CitySettings_SaveGet();

    if (cs->settings[CITYSETTING_SAVE_MACHINERESPAWN] == 1 &&
        gd->city_kind == 5 &&
        !rd->is_walk_after_dead)
    {
        // enter respawn
        Rider_RespawnEnter(rd);
        rd->is_walk_after_dead = 1;
        return 1;
    }

    return 0;
}
CODEPATCH_HOOKCONDITIONALCREATE(0x801a5424, "mr 3,31\n\t", MachineRespawn_CheckToRespawnRider, "", 0, 0x801a5470)
void MachineRespawn_DisableWalkAfterDead(RiderData *rd)
{
    GameData *gd = Gm_GetGameData();
    CitySettingsSave *cs = CitySettings_SaveGet();

    if (cs->settings[CITYSETTING_SAVE_MACHINERESPAWN] == 1 &&
        gd->city_kind == 5)
    {
        rd->is_walk_after_dead = 0;
    }

    return;
}
CODEPATCH_HOOKCREATE(0x801a36c8, "mr 3,31\n\t", MachineRespawn_DisableWalkAfterDead, "", 0)

// Damage Ratio Patch
float MachineDamageRatio_Apply(float dmg_taken)
{
    CitySettingsSave *cs = CitySettings_SaveGet();

    if (Scene_GetCurrentMajor() == MJRKIND_CITY)
    {
        static float dmg_mult[] = {0.5, 0.75, 1, 1.25, 1.5, 1.75, 2, 2.5, 3, 5};
        dmg_taken *= dmg_mult[cs->settings[CITYSETTING_SAVE_DAMAGE]];
    }

    return dmg_taken;
}
CODEPATCH_HOOKCREATE(0x801c68b4, "fmr 1,30\n\t", MachineDamageRatio_Apply, "fmr 30,1\n\t", 0)

///////////////////
// Apply Patches //
///////////////////

void Machines_ApplyPatches()
{
    // roll a random machine for all players to use
    stc_random_character = Machines_RollRandom();

    CODEPATCH_HOOKAPPLY(0x8002df5c); // injection to adjust starting machine
    CODEPATCH_HOOKAPPLY(0x801c73b4); // injection to adjust max speed

    // adjust vehicle spawn code
    CODEPATCH_HOOKAPPLY(0x801df00c);                      // support less than 4 vehicles being enabled
    CODEPATCH_HOOKAPPLY(0x801df44c);                      // blacklist disabled machines in formation
    CODEPATCH_REPLACEINSTRUCTION(0x801df254, 0x60000000); // ignore original buffer index increment code
    CODEPATCH_REPLACEINSTRUCTION(0x801df234, 0x60000000); // ignore original buffer index increment code

    CODEPATCH_REPLACEINSTRUCTION(0x801df9e8, 0x60000000); // remove hardcoded limit of 10 vehicles on the field

    // machine respawn in city
    CODEPATCH_HOOKAPPLY(0x801952c8); // skip changing the players machine when dying
    CODEPATCH_HOOKAPPLY(0x801a5424); // enter the rider into respawn when enabled
    CODEPATCH_HOOKAPPLY(0x801a36c8);

    // machine dmg ratio
    CODEPATCH_HOOKAPPLY(0x801c68b4);
}