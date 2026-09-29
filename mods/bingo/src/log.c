#include "hurt.h"
#include "game.h"
#include "rider.h"
#include "machine.h"
#include "weapon.h"
#include "item.h"
#include "log.h"

#include <string.h>

#include "code_patch/code_patch.h"

// action logging
int g_dmg_log_num;
DamageLog g_dmg_log[32];
int g_zone_log_num;
ZoneLog g_zone_log[32];
int g_rail_log_num;
RailLog g_rail_log[5];

void DamageLog_Clear()
{
    g_dmg_log_num = 0;
    memset(g_dmg_log, 0, sizeof(g_dmg_log));
}
void DamageLog_CopyFromHurtData(DamageLogObject *log, HurtData *hurt_data)
{
    GOBJ *gobj = hurt_data->gobj;

    log->hurt_kind = hurt_data->kind;

    log->state2 = -1;

    switch (hurt_data->kind)
    {
        case (HURTKIND_RIDER):
        {
            RiderData *rp = gobj->userdata;
            log->kind = rp->kind;
            log->ply = rp->ply;
            log->state = rp->status;
            log->attack_data = rp->dmg_log.attack_data;

            if (rp->machine_gobj)
                log->is_airborne = Rider_IsMachineAirborne(rp);
            else
                log->is_airborne = rp->is_airborne;

            break;
        }
        case (HURTKIND_MACHINE):
        case (HURTKIND_MACHINE_EMPTY):
        {
            MachineData *mp = gobj->userdata;
            log->kind = mp->kind;
            log->state = mp->status;
            log->state2 = mp->status2;
            log->attack_data = mp->dmg_log.attack_data;
            
            log->ply = Machine_GetRiderPly(mp);
            log->is_airborne = mp->is_airborne;
            
            break;
        }
        case (HURTKIND_WEAPON):
        {
            WeaponData *wp = gobj->userdata;
            log->kind = wp->kind;
            log->state = wp->state;
            log->is_airborne = 1;
            log->attack_data = wp->dmg_log.attack_data;

            if (wp->owner_gobj && wp->owner_gobj->p_link == GAMEPLINK_RIDER)
            {
                // assuming only riders can own weapons...
                RiderData *rp = wp->owner_gobj->userdata;
                log->ply = rp->ply;
            }
            else
                log->ply = -1;
            
            break;
        }
        case (HURTKIND_ITEM):
        {
            ItemData *ip = gobj->userdata;
            log->kind = ip->kind;
            log->state = ip->state;
            log->is_airborne = ip->is_airborne;
            log->ply = -1;
            log->attack_data = (AttackData){0};

            
            break;
        }
        case (HURTKIND_MAP):
        {
            YakumonoData *yp = gobj->userdata;
            log->kind = yp->kind;
            log->state = yp->state;
            log->ply = -1;
            log->is_airborne = 0;
            log->attack_data = (AttackData){0};
            
            break;
        }
    }

}
void DamageLog_Add(HurtData *hurt_data, HurtData *attacker_hurt_data, int ply, int kind)
{
    DamageLog *this_log = &g_dmg_log[g_dmg_log_num++];

    DamageLog_CopyFromHurtData(&this_log->victim, hurt_data);
    DamageLog_CopyFromHurtData(&this_log->attacker, attacker_hurt_data);

    this_log->kb_mag = hurt_data->kb_mag;
    this_log->dmg = hurt_data->dmg_taken;

    // determine KO
    int is_ko = 0;
    switch (hurt_data->kind)
    {
        case (HURTKIND_MACHINE):
        case (HURTKIND_MACHINE_EMPTY):
        {
            MachineData *mp = hurt_data->gobj->userdata;
            if (hurt_data->dmg_taken >= mp->hp)
                is_ko = 1;
            
            break;
        }
        case (HURTKIND_ITEM):
        {
            ItemData *ip = hurt_data->gobj->userdata;
            if (hurt_data->dmg_taken >= (ip->hp - ip->dmg))
                is_ko = 1;
            
            break;
        }
    }
    this_log->is_ko = is_ko;
}
void DamageLog_Machine(MachineData *mp, HitCollLog *log)
{
    DamageLog_Add(mp->hurt_data, log->attacker, Machine_GetRiderPly(mp), mp->kind);
}
CODEPATCH_HOOKCREATE(0x801d7358, "mr 3, 31\n\t" "mr 4, 30\n\t", DamageLog_Machine, "lwz	0, 0 (29)\n\t", 0)
void DamageLog_Rider(RiderData *rp, HitCollLog *log)
{
    DamageLog_Add(rp->hurt_data, log->attacker, rp->ply, rp->kind);
}
CODEPATCH_HOOKCREATE(0x801965a4, "mr 3, 31\n\t" "mr 4, 30\n\t", DamageLog_Rider, "lwz	0, 0 (29)\n\t", 0)

HitCollLog *DamageLog_Box(HitCollLog *log, ItemData *ip)
{
    DamageLog_Add(ip->hurt_data, log->attacker, -1, ip->kind);

    return log;
}
CODEPATCH_HOOKCREATE(0x80252434, "mr 4, 31\n\t", DamageLog_Box, "", 0)
void DamageLog_Yakumono(GOBJ *y, int ply)
{
    DamageLog *this_log = &g_dmg_log[g_dmg_log_num++];
    
    YakumonoData *yp = y->userdata;

    GOBJ *m = Ply_GetMachineGObj(ply);
    MachineData *mp = m->userdata;
    
    this_log->attacker.ply = ply;
    this_log->attacker.hurt_kind = HURTKIND_MACHINE;
    this_log->attacker.kind = mp->kind;
    this_log->attacker.state = mp->status;
    this_log->attacker.is_airborne = mp->is_airborne;

    this_log->victim.hurt_kind = HURTKIND_MAP;
    this_log->victim.ply = -1;
    this_log->victim.kind = yp->kind;
    this_log->victim.state = yp->state;
    this_log->victim.is_airborne = 0;

    this_log->is_ko = 1;
    this_log->dmg = 1;
    this_log->kb_mag = 0;
}
CODEPATCH_HOOKCREATE(0x80105d90, "stwu	1, -40 (1)\n\t"
                                 "mflr 0\n\t"
                                 "stw 0, 44 (1)\n\t"
                                 "stw 3, 8 (1)\n\t"
                                 "stw 4, 12 (1)\n\t", 
                                 DamageLog_Yakumono,
                                 "lwz 3, 8 (1)\n\t"
                                 "lwz 4, 12 (1)\n\t"
                                 "lwz 0, 44 (1)\n\t"
                                 "mtlr 0\n\t"
                                 "addi 1, 1, 40\n\t", 
                                 0)

void ZoneLog_Clear()
{
    g_zone_log_num = 0;
    memset(g_zone_log, -1, sizeof(g_zone_log));
}
void ZoneLog_Add(int ply, int zone_idx, ZoneKind kind)
{
    ZoneLog *this_log = &g_zone_log[g_zone_log_num++];

    this_log->ply = ply;
    this_log->idx = zone_idx;
    this_log->kind = kind;
}
void ZoneLog_Boost(MachineData *mp, int zone_idx)
{
    ZoneLog_Add(Machine_GetRiderPly(mp), zone_idx, ZONEKIND_BOOST);
}
CODEPATCH_HOOKCREATE(0x801cf6b0, "mr 3, 31\n\t" "mr 4, 30\n\t", ZoneLog_Boost, "", 0)
CODEPATCH_HOOKCREATE(0x801cf71c, "mr 3, 31\n\t" "mr 4, 30\n\t", ZoneLog_Boost, "", 0)
void ZoneLog_Lift(MachineData *mp, int zone_idx)
{
    ZoneLog_Add(Machine_GetRiderPly(mp), zone_idx, ZONEKIND_LIFT);
}
CODEPATCH_HOOKCREATE(0x801e3fa8, "mr 3, 30\n\t" "mr 4, 31\n\t", ZoneLog_Lift, "", 0)
CODEPATCH_HOOKCREATE(0x801e4014, "mr 3, 30\n\t" "mr 4, 31\n\t", ZoneLog_Lift, "", 0)

void RailLog_Clear()
{
    g_rail_log_num = 0;
    memset(g_rail_log, -1, sizeof(g_rail_log));
}
void RailLog_Add(MachineData *mp, int rail_idx, float rail_progress)
{
    RailLog *this_log = &g_rail_log[g_rail_log_num++];

    this_log->ply = Machine_GetRiderPly(mp);
    this_log->idx = rail_idx;
    this_log->progress = rail_progress;
    this_log->status = mp->status;
    this_log->status2 = mp->status2;
    this_log->is_airborne = mp->is_airborne;
}
void RailLog_Enter(MachineData *mp, int rail_idx, float rail_progress)
{
    RailLog_Add(mp, rail_idx, rail_progress);
}
CODEPATCH_HOOKCREATE(0x801e468c, "mr 3, 30\n\t" "mr 4, 31\n\t" "lfs 1, 12 (1)\n\t", RailLog_Enter, "", 0)

void Log_Clear()
{
    DamageLog_Clear();
    ZoneLog_Clear();
    RailLog_Clear();
}

void BingoLog_Init()
{
    // damage logging
    CODEPATCH_HOOKAPPLY(0x801d7358);
    CODEPATCH_HOOKAPPLY(0x801965a4);
    CODEPATCH_HOOKAPPLY(0x80252434);
    
    // zone logging
    CODEPATCH_HOOKAPPLY(0x801cf6b0);
    CODEPATCH_HOOKAPPLY(0x801cf71c);
    CODEPATCH_HOOKAPPLY(0x801e3fa8);
    CODEPATCH_HOOKAPPLY(0x801e4014);
    
    // rail logging
    CODEPATCH_HOOKAPPLY(0x801e468c);
}