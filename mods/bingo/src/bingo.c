
#include "text.h"
#include "os.h"
#include "hsd.h"
#include "preload.h"
#include "game.h"
#include "hud.h"
#include "text.h"
#include "weapon.h"
#include "scene.h"
#include "inline.h"
#include "debug.h"

#include "hoshi/func.h"
#include "hoshi/screen_cam.h"

#include "code_patch/code_patch.h"

#include <string.h>

#include "bingo.h"

int is_bingo_mode = 1;

JOBJSet *card_set;
JOBJSet *icon_set;
JOBJSet *notif_set;

GOBJ *bingo_card_gobj[5];
GOBJ *bingo_tracker_gobj[5];
BingoCursor bingo_cursor[5];
BingoCard g_bingo_card;

// action logging
static int g_dmg_log_num;
static DamageLog g_dmg_log[32];
static int g_zone_log_num;
static ZoneLog g_zone_log[32];
static int g_rail_log_num;
static RailLog g_rail_log[5];

static AreaBound g_area_bounds[] = {
    // forest
    MAKE_AREA_BOUND(-537, -295, 
                    20, 200,
                    -190, 212),
    // volcano
    MAKE_AREA_BOUND(-680, -350, 
                    52, 200,
                    -690, -160),
    MAKE_AREA_BOUND(-500, -219, 
                    52, 200,
                    -714, -404),
    // city
    MAKE_AREA_BOUND(-295, 240, 
                    25, 200,
                    -612, 100),
    // wharf
    MAKE_AREA_BOUND(260, 650, 
                    -10, 200,
                    -366, 360),
    // electric lounge
    MAKE_AREA_BOUND(-140, 500, 
                    -10, 200,
                    384, 750),
    // golf course
    MAKE_AREA_BOUND(-530, -250, 
                    -20, 200,
                    212, 900),

    // sky island top
    MAKE_AREA_BOUND(-183, 42, 
                    465, 520,
                    -82, 112),
    // rock flower
    MAKE_AREA_BOUND(-144, -100, 
                    200, 215,
                    -857, -815),
    // castle flower
    MAKE_AREA_BOUND(407, 412, 
                    368, 380,
                    -567, -563),
};
BingoGoalDesc g_difficulty_param[] = {
    // stat get
    {
        .kind = GOAL_STATGET,
        .num = 3,
        .param.stat_get = {
            {.difficulty = DFCLT_EASY, .num_min = 3, .num_max = 5, 
                .items = (1 << ITKIND_ACCEL) |
                        (1 << ITKIND_TOPSPEED) |
                        (1 << ITKIND_OFFENSE) |
                        (1 << ITKIND_DEFENSE) |
                        (1 << ITKIND_TURN) |
                        (1 << ITKIND_CHARGE) |
                        (1 << ITKIND_GLIDE) |
                        (1 << ITKIND_WEIGHT) |
                        (1 << ITKIND_HP)},
            {.difficulty = DFCLT_MEDIUM, .num_min = 6, .num_max = 10, 
                .items = (1 << ITKIND_ACCEL) |
                        (1 << ITKIND_TOPSPEED) |
                        (1 << ITKIND_OFFENSE) |
                        (1 << ITKIND_DEFENSE) |
                        (1 << ITKIND_TURN) |
                        (1 << ITKIND_CHARGE) |
                        (1 << ITKIND_GLIDE) |
                        (1 << ITKIND_WEIGHT) |
                        (1 << ITKIND_HP)},
            {.difficulty = DFCLT_HARD, .num_min = 18, .num_max = 18, 
                .items = (1 << ITKIND_ACCEL) |
                        (1 << ITKIND_TOPSPEED) |
                        (1 << ITKIND_OFFENSE) |
                        (1 << ITKIND_DEFENSE) |
                        (1 << ITKIND_TURN) |
                        (1 << ITKIND_CHARGE) |
                        (1 << ITKIND_GLIDE) |
                        (1 << ITKIND_WEIGHT) |
                        (1 << ITKIND_HP)},
        },
    },
    // food get
    {
        .kind = GOAL_FOODGET,
        .num = 3,
        .param.food_get = {
            {.difficulty = DFCLT_EASY, .num_min = 1, .num_max = 1},
            {.difficulty = DFCLT_MEDIUM, .num_min = 2, .num_max = 2},
            {.difficulty = DFCLT_HARD, .num_min = 3, .num_max = 4},
        },
    },
    // // item fall get
    // {
    //     .kind = GOAL_ITEMFALLGET,
    //     .num = 1,
    //     .param.item_fall_get = {
    //         {.difficulty = DFCLT_HARD, .num_min = 1, .num_max = 1},
    //     },
    // },
    // position
    {
        .kind = GOAL_POSITION,
        .num = 3,
        .param.position = {
            {.difficulty = DFCLT_EASY, .positions = (1 << AREAKIND_ISLAND)},
            {.difficulty = DFCLT_MEDIUM, .positions = (1 << AREAKIND_ROCKFLOWER)},
            {.difficulty = DFCLT_HARD, .positions = (1 << AREAKIND_CITYHALLFLOWER)},
        },
    },
    // break box any
    {
        .kind = GOAL_BREAKBOXANY,
        .num = 3,
        .param.box_any = {
            {.difficulty = DFCLT_EASY, .num_min = 10, .num_max = 10},
            {.difficulty = DFCLT_MEDIUM, .num_min = 20, .num_max = 20},
            {.difficulty = DFCLT_HARD, .num_min = 30, .num_max = 30},
        },
    },
    // break box kind
    {
        .kind = GOAL_BREAKBOXKIND,
        .num = 3,
        .param.box_kind = {
            {.difficulty = DFCLT_EASY, .num_min = 3, .num_max = 3},
            {.difficulty = DFCLT_MEDIUM, .num_min = 10, .num_max = 10},
            {.difficulty = DFCLT_HARD, .num_min = 15, .num_max = 15},
        },
    },
    // break box attack
    {
        .kind = GOAL_BREAKBOXWITHATTACK,
        .num = 3,
        .param.box_attack = {
            {.difficulty = DFCLT_EASY, .num_min = 1, .num_max = 1, 
                .attacks = (1 << ATK_SPIN) |
                           (1 << ATK_FIRESHOOT) | 
                           (1 << ATK_SWORD1) | 
                           (1 << ATK_BOMB) | 
                           (1 << ATK_PLASMA) | 
                           (1 << ATK_BOMB) | 
                           (1 << ATK_NEEDLEHOLD) | 
                           (1 << ATK_TORNADO) |
                           (1 << ATK_TIMEBOMB) | 
                           (1 << ATK_GORDO) | 
                           (1 << ATK_MININADO)},
            {.difficulty = DFCLT_MEDIUM, .num_min = 1, .num_max = 1, 
                .attacks = (1 << ATK_MIKE)},
            {.difficulty = DFCLT_HARD, .num_min = 3, .num_max = 3, 
                .attacks = (1 << ATK_SPIN) |
                           (1 << ATK_FIRESHOOT) | 
                           (1 << ATK_SWORD1) | 
                           (1 << ATK_BOMB) | 
                           (1 << ATK_PLASMA) | 
                           (1 << ATK_BOMB) | 
                           (1 << ATK_NEEDLEHOLD) | 
                           (1 << ATK_TORNADO) |
                           (1 << ATK_TIMEBOMB) | 
                           (1 << ATK_GORDO) | 
                           (1 << ATK_MININADO)},
        },
    },
    // hit player
    {
        .kind = GOAL_HITPLAYER,
        .num = 1,
        .param.hit_ply = {
            {.difficulty = DFCLT_EASY, .num_min = 1, .num_max = 1},
        },
    },
    // hit player with attack
    {
        .kind = GOAL_KOPLAYER,
        .num = 1,
        .param.ko_ply = {
            {.difficulty = DFCLT_HARD, .num_min = 1, .num_max = 1},
        },
    },
    // hit player with attack
    {
        .kind = GOAL_HITPLAYERWITHATTACK,
        .num = 1,
        .param.hit_ply_attack = {
            {.difficulty = DFCLT_MEDIUM, .num_min = 1, .num_max = 1, 
                .attacks = (1 << ATK_SPIN) |
                           (1 << ATK_SWORD1) | 
                           (1 << ATK_PLASMA) | 
                           (1 << ATK_NEEDLEHOLD) | 
                           (1 << ATK_TORNADO) |
                           (1 << ATK_TIMEBOMB) | 
                           (1 << ATK_GORDO) | 
                           (1 << ATK_MININADO)},
        },
    },
    // KO player
    {
        .kind = GOAL_KOPLAYER,
        .num = 1,
        .param.ko_ply = {
            {.difficulty = DFCLT_HARD, .num_min = 1, .num_max = 1},
        },
    },
    // KO machine
    {
        .kind = GOAL_DESTROYMACHINE,
        .num = 3,
        .param.ko_machine = {
            {.difficulty = DFCLT_EASY, .num_min = 1, .num_max = 1},
            {.difficulty = DFCLT_MEDIUM, .num_min = 2, .num_max = 2},
            {.difficulty = DFCLT_HARD, .num_min = 3, .num_max = 5},
        },
    },
    // ride machine
    {
        .kind = GOAL_RIDEMACHINEKIND,
        .num = 1,
        .param.ride_machine_kind = {
            {.difficulty = DFCLT_MEDIUM, .num_min = 1, .num_max = 1,
                .machines = (1 << VCKIND_WARP) |
                            (1 << VCKIND_WINGED) | 
                            (1 << VCKIND_SHADOW) | 
                            (1 << VCKIND_BULK) | 
                            (1 << VCKIND_SLICK) | 
                            (1 << VCKIND_FORMULA) | 
                            (1 << VCKIND_WAGON) | 
                            (1 << VCKIND_ROCKET) | 
                            (1 << VCKIND_SWERVE) | 
                            (1 << VCKIND_TURBO) | 
                            (1 << VCKIND_JET)},
        },
    },
    // rail distance
    {
        .kind = GOAL_RAILDISTANCE,
        .num = 1,
        .param.rail_dist = {
            {.difficulty = DFCLT_EASY, .dist = 100},
        },
    },
    // // rail land
    // {
    //     .kind = GOAL_RAILLAND,
    //     .num = 1,
    //     .param.rail_land = {
    //         {.difficulty = DFCLT_MEDIUM, .num_min = 1, .num_max = 1},
    //     },
    // },
    // glide
    {
        .kind = GOAL_GLIDETIME,
        .num = 3,
        .param.glide_time = {
            {.difficulty = DFCLT_EASY, .num_min = 3, .num_max = 4},
            {.difficulty = DFCLT_MEDIUM, .num_min = 5, .num_max = 6},
            {.difficulty = DFCLT_HARD, .num_min = 7, .num_max = 8},
        },
    },
    // glide
    {
        .kind = GOAL_BOOSTRING,
        .num = 3,
        .param.boost_ring = {
            {.difficulty = DFCLT_EASY, .num_min = 2, .num_max = 3},
            {.difficulty = DFCLT_MEDIUM, .num_min = 5, .num_max = 6},
            {.difficulty = DFCLT_HARD, .num_min = 7, .num_max = 9},
        },
    },
};

// Bingo Game Mode
void BingoMode_Start()
{
    // distribute difficulty
    u8 difficulty_arr[BINGO_UI_GRID_SIZE * BINGO_UI_GRID_SIZE];
    for (int i = 0; i < 10; i++)
        difficulty_arr[i] = DFCLT_EASY;
    for (int i = 10; i < 22; i++)
        difficulty_arr[i] = DFCLT_MEDIUM;
    for (int i = 22; i < 25; i++)
        difficulty_arr[i] = DFCLT_HARD;
    for (int i = BINGO_UI_GRID_SIZE * BINGO_UI_GRID_SIZE - 1; i > 0; i--)
    {
        int swap_idx = HSD_Randi(i + 1);

        u8 temp = difficulty_arr[swap_idx];
        difficulty_arr[swap_idx] = difficulty_arr[i];
        difficulty_arr[i] = temp;
    }

    // generate bingo card here
    for (int i = 0; i < BINGO_UI_GRID_SIZE * BINGO_UI_GRID_SIZE; i++)
    {
        BingoGoal *gd = &g_bingo_card.goal[i];

        // get random condition
        int is_duplicate_goal;
        do
        {
            is_duplicate_goal = 0;

            int difficulty = difficulty_arr[i];

            BingoMode_GenerateGoal(gd, difficulty);

            for (int j = 0; j < i; j++)
            {
                BingoGoal *that_gd = &g_bingo_card.goal[j];
                if (BingoMode_CheckDuplicateGoal(gd, that_gd))
                {
                    is_duplicate_goal = 1;
                    break;
                }
            }
        } while (is_duplicate_goal);
    }

    for (int i = 0; i < BINGO_UI_GRID_SIZE * BINGO_UI_GRID_SIZE; i++)
    {
        BingoGoal *gd = &g_bingo_card.goal[i];
        
        static char *goal_names[] = {
            "STATGET",
            "FOODGET",
            "ITEMFALLGET",
            "POSITION",
            "BREAKBOXANY",
            "BREAKBOXKIND",
            "BREAKBOXWITHATTACK",
            "HITPLAYER",
            "HITPLAYERWITHATTACK",
            "KOPLAYER",
            "DESTROYMACHINE",
            "RIDEMACHINEKIND",
            "RAILDISTANCE",
            "RAILLAND",
            "GLIDETIME",
            "BOOSTRING",
        };
    
        char s[256];
        Bingo_GetDescriptionForGoal(gd, s);
        OSReport("Bingo Icon #%02d: %s -- %s\n", i + 1, goal_names[gd->kind], s);

    }
}
void BingoMode_GenerateGoal(BingoGoal *gd, BingoDifficultyKind difficulty)
{
    // find candidates for this difficulty
    int valid_goal_num = 0;
    for (BingoGoalKind goal_kind = 0; goal_kind < GetElementsIn(g_difficulty_param); goal_kind++)
    {
        BingoGoalDesc *this_goal_desc = &g_difficulty_param[goal_kind];
        for (int diff_idx = 0; diff_idx < this_goal_desc->num; diff_idx++)
        {
            if (this_goal_desc->param.common[diff_idx].difficulty == difficulty)
                valid_goal_num++;
        }
    }

    if (valid_goal_num == 0)
    {
        OSReport("no valid goals!\n");
        assert("bingo_gen");
    }

    // get random one
    int rand_goal = HSD_Randi(valid_goal_num);

    // find it
    int candidate_idx = 0;
    for (BingoGoalKind goal_kind = 0; goal_kind < GetElementsIn(g_difficulty_param); goal_kind++)
    {
        BingoGoalDesc *this_goal_desc = &g_difficulty_param[goal_kind];

        // check all difficulties
        for (int diff_idx = 0; diff_idx < this_goal_desc->num; diff_idx++)
        {
            if (this_goal_desc->param.common[diff_idx].difficulty == difficulty)
            {
                if (candidate_idx == rand_goal)
                {
                    BingoMode_InitGoal(gd, this_goal_desc, diff_idx);
                    return;
                }

                candidate_idx++;
            }
        }
    }
}
void BingoMode_InitGoal(BingoGoal *gd, BingoGoalDesc *desc, int param_idx)
{
    memset(gd, 0, sizeof(*gd));

    // copy over the kind
    gd->ply_completed = -1;
    gd->difficulty = desc->param.common[param_idx].difficulty;
    gd->kind = desc->kind;

    // handle each goal
    switch (desc->kind)
    {
        case (GOAL_STATGET):
        {
            gd->num = RandomInRange(desc->param.stat_get[param_idx].num_min, desc->param.stat_get[param_idx].num_max);
            gd->param.stat_get.kind = RandomBitInField(desc->param.stat_get[param_idx].items);
            
            // hp max is 16
            if (gd->param.stat_get.kind == ITKIND_HP && gd->num > 16)
                gd->num = 16;
            
            break;
        }
        case (GOAL_FOODGET):
        {
            gd->num = RandomInRange(desc->param.food_get[param_idx].num_min, desc->param.food_get[param_idx].num_max);
            gd->param.food_get.kind = RandomInRange(ITKIND_FOODMAXIMTOMATO, ITKIND_FOODAPPLE);
            break;
        }
        case (GOAL_ITEMFALLGET):
        {
            gd->num = RandomInRange(desc->param.item_fall_get[param_idx].num_min, desc->param.item_fall_get[param_idx].num_max);
            break;
        }
        case (GOAL_POSITION):
        {
            gd->num = 1;
            gd->param.position.kind = RandomBitInField(desc->param.position[param_idx].positions);
            break;
        }
        case (GOAL_BREAKBOXANY):
        {
            gd->num = RandomInRange(desc->param.box_any[param_idx].num_min, desc->param.box_any[param_idx].num_max);
            break;
        }
        case (GOAL_BREAKBOXKIND):
        {
            gd->num = RandomInRange(desc->param.box_kind[param_idx].num_min, desc->param.box_kind[param_idx].num_max);
            gd->param.box_kind.kind = RandomInRange(BOXKIND_BLUE, BOXKIND_RED);
            break;
        }
        case (GOAL_BREAKBOXWITHATTACK):
        {
            gd->num = RandomInRange(desc->param.box_attack[param_idx].num_min, desc->param.box_attack[param_idx].num_max);
            gd->param.box_attack.attack = RandomBitInField(desc->param.box_attack[param_idx].attacks);
            break;
        }
        case (GOAL_HITPLAYER):
        {
            gd->num = RandomInRange(desc->param.hit_ply[param_idx].num_min, desc->param.hit_ply[param_idx].num_max);
            break;
        }
        case (GOAL_HITPLAYERWITHATTACK):
        {
            gd->num = RandomInRange(desc->param.hit_ply_attack[param_idx].num_min, desc->param.hit_ply_attack[param_idx].num_max);
            gd->param.hit_ply_attack.attack = RandomBitInField(desc->param.hit_ply_attack[param_idx].attacks);
            break;
        }
        case (GOAL_KOPLAYER):
        {
            gd->num = RandomInRange(desc->param.ko_ply[param_idx].num_min, desc->param.ko_ply[param_idx].num_max);
            break;
        }
        case (GOAL_DESTROYMACHINE):
        {
            gd->num = RandomInRange(desc->param.ko_machine[param_idx].num_min, desc->param.ko_machine[param_idx].num_max);
            break;
        }
        case (GOAL_RIDEMACHINEKIND):
        {
            gd->num = RandomInRange(desc->param.ride_machine_kind[param_idx].num_min, desc->param.ride_machine_kind[param_idx].num_max);
            gd->param.ride_machine_kind.kind = RandomBitInField(desc->param.ride_machine_kind[param_idx].machines);
            break;
        }
        case (GOAL_RAILDISTANCE):
        {
            gd->num = 1;
            gd->param.rail_dist.dist = desc->param.rail_dist[param_idx].dist;
            break;
        }
        case (GOAL_RAILLAND):
        {
            gd->num = RandomInRange(desc->param.rail_land[param_idx].num_min, desc->param.rail_land[param_idx].num_max);
            break;
        }
        case (GOAL_GLIDETIME):
        {
            gd->num = RandomInRange(desc->param.glide_time[param_idx].num_min, desc->param.glide_time[param_idx].num_max);
            break;
        }
        case (GOAL_BOOSTRING):
        {
            gd->num = RandomInRange(desc->param.boost_ring[param_idx].num_min, desc->param.boost_ring[param_idx].num_max);
            break;
        }
    }
}
int BingoMode_CheckDuplicateGoal(BingoGoal *this, BingoGoal *that)
{
    int is_dupe = 0;

    if (this->kind == that->kind && 
        this->difficulty == that->difficulty && 
        !memcmp(&this->param.common, &that->param.common, sizeof(this->param.common)))
        is_dupe = 1;
    
    return is_dupe;
}

void Bingo_GetDescriptionForGoal(BingoGoal *gd, char *out)
{
    static char *item_names[] = {
        "Blue Box",
        "Green Box",
        "Red Box",
        "Boost Up",
        "Boost Down",
        "Top Speed Up",
        "Top Speed Down",
        "Offense Up",
        "Offense Down",
        "Defense Up",
        "Defense Down",
        "Turn Up",
        "Turn Down",
        "Glide Up",
        "Glide Down",
        "Charge Up",
        "Charge Down",
        "Weight Up",
        "Weight Down",
        "HP Up",
        "All Up",
        "Speed Up",
        "Speed Down",
        "Attack Up",
        "Defense Up",
        "Max Speed",
        "No Charge",
        "Candy",
        "Bomb Panel",
        "Fire Panel",
        "Freeze Panel",
        "Sleep Panel",
        "Wheel Panel",
        "Wing Panel",
        "Plasma Panel",
        "Tornado Panel",
        "Sword Panel",
        "Needle Panel",
        "Mike Panel",
        "Maxim Tomato",
        "Energy Drink",
        "Ice Cream",
        "Riceball",
        "Roast Chicken",
        "Curry",
        "Ramen",
        "Omelet",
        "Hamburger",
        "Sushi",
        "Hot Dog",
        "Apple",
        "Fireworks",
        "Panic Spin",
        "Sensor Bomb",
        "Gold Spike",
    };
    static char *machine_names[] = {
        "Compact Star",
        "Warp Star",
        "Turbo Star",
        "Formula Star",
        "Slick Star",
        "Swerve Star",
        "Wagon Star",
        "Bulk Star",
        "Shadow Star",
        "Winged Star",
        "Jet Star",
        "Rocket Star",
        "Wheelie Scooter",
        "Wheelie Bike",
        "Rex Wheelie",
        "Dragoon",
        "Hydra",
        "Flight Star",
    };
    static char *attack_names[] = {
        "0",
        "Fire Start",
        "Fire",
        "3",
        "Sleep",
        "Sword",
        "Sword 2",
        "7",
        "Bomb",
        "Plasma",
        "Needle End",
        "Needle",
        "Mike",
        "13",
        "Tornado",
        "15",
        "Quick Spin",
        "17",
        "Time Bomb",
        "Gordo",
        "Panic Spin",
    };
    static char *area_names[] = {
        "Sky Island",
        "Rock Flower",
        "City Hall Flower",
    };

    switch(gd->kind)
    {
        case (GOAL_STATGET):
        {
            sprintf(out, "Collect %d %s.", gd->num, item_names[gd->param.stat_get.kind]);
            break;
        }
        case (GOAL_FOODGET):
        {
            sprintf(out, "Collect %d %s.", gd->num, item_names[gd->param.food_get.kind]);
            break;
        }
        case (GOAL_ITEMFALLGET):
        {
            sprintf(out, "Collect any item falling from the sky %d times.", gd->num);
            break;
        }
        case (GOAL_POSITION):
        {
            sprintf(out, "Reach the %s.", area_names[gd->param.position.kind - AREAKIND_ISLAND]);
            break;
        }
        case (GOAL_BREAKBOXANY):
        {
            sprintf(out, "Break %d boxes.", gd->num);
            break;
        }
        case (GOAL_BREAKBOXKIND):
        {
            sprintf(out, "Break %d %s\x81\x69s\x81\x6a.", gd->num, item_names[gd->param.box_kind.kind]);
            break;
        }
        case (GOAL_BREAKBOXWITHATTACK):
        {
            sprintf(out, "Break %d box\x81\x69s\x81\x6a with %s.", gd->num, attack_names[gd->param.box_attack.attack]);
            break;
        }
        case (GOAL_HITPLAYER):
        {
            sprintf(out, "Attack a player %d time\x81\x69s\x81\x6a.", gd->num);
            break;
        }
        case (GOAL_HITPLAYERWITHATTACK):
        {
            sprintf(out, "Attack a player with %s %d time\x81\x69s\x81\x6a.", attack_names[gd->param.hit_ply_attack.attack], gd->num);
            break;
        }
        case (GOAL_KOPLAYER):
        {
            sprintf(out, "KO a player %d time\x81\x69s\x81\x6a.", gd->num);
            break;
        }
        case (GOAL_DESTROYMACHINE):
        {
            sprintf(out, "Destroy a machine %d time\x81\x69s\x81\x6a.", gd->num);
            break;
        }
        case (GOAL_RIDEMACHINEKIND):
        {
            sprintf(out, "Ride %s %d time\x81\x69s\x81\x6a.", machine_names[gd->param.ride_machine_kind.kind], gd->num);
            break;
        }
        case (GOAL_RAILDISTANCE):
        {
            sprintf(out, "Ride rails for %d distance.", gd->param.rail_dist.dist);
            break;
        }
        case (GOAL_RAILLAND):
        {
            sprintf(out, "Land on a rail %d times.", gd->num);
            break;
        }
        case (GOAL_GLIDETIME):
        {
            sprintf(out, "Glide for %d seconds.", gd->num);
            break;
        }
        case (GOAL_BOOSTRING):
        {
            sprintf(out, "Fly through %d boost rings.", gd->num);
            break;
        }
    }

    return;
}
void Bingo_SetIconForGoal(BingoGoal *goal, JOBJ *icon_j, int single_icon_idx, int multi_icon_idx, int single_digit_idx, int double_digit_idx)
{
    static u8 attack_to_frame[] = {
        0,
        BINGOICONFRAME_ITEMSTART + ITKIND_COPYFIRE,
        BINGOICONFRAME_ITEMSTART + ITKIND_COPYFIRE,
        0,
        BINGOICONFRAME_ITEMSTART + ITKIND_COPYSLEEP,
        BINGOICONFRAME_ITEMSTART + ITKIND_COPYSWORD,
        BINGOICONFRAME_ITEMSTART + ITKIND_COPYSWORD,
        0,
        BINGOICONFRAME_ITEMSTART + ITKIND_COPYBOMB,
        BINGOICONFRAME_ITEMSTART + ITKIND_COPYPLASMA,
        BINGOICONFRAME_ITEMSTART + ITKIND_COPYNEEDLE,
        BINGOICONFRAME_ITEMSTART + ITKIND_COPYNEEDLE,
        BINGOICONFRAME_ITEMSTART + ITKIND_COPYMIKE,
        0,
        BINGOICONFRAME_ITEMSTART + ITKIND_COPYTORNADO,
        0,
        BINGOICONFRAME_PLAYER,
        0,
        BINGOICONFRAME_ITEMSTART + ITKIND_TIMEBOMB,
        BINGOICONFRAME_ITEMSTART + ITKIND_GORDO,
        BINGOICONFRAME_ITEMSTART + ITKIND_MININADO,
    };

    JOBJ *single_icon = JObj_GetIndex(icon_j, single_icon_idx);
    JOBJ *multi_icon = JObj_GetIndex(icon_j, multi_icon_idx);
    JOBJ *single_digit = JObj_GetIndex(icon_j, single_digit_idx);
    JOBJ *double_digit = JObj_GetIndex(icon_j, double_digit_idx);

    // display icon
    {
        int single_frame;
        int multi_frame[3];
        switch (goal->kind)
        {
            // single icon
            case (GOAL_STATGET):
                single_frame = BINGOICONFRAME_ITEMSTART + goal->param.stat_get.kind;
                goto DISPLAY_SINGLE_ICON;
            case (GOAL_FOODGET):
                single_frame = BINGOICONFRAME_ITEMSTART + goal->param.food_get.kind;
                goto DISPLAY_SINGLE_ICON;
            case (GOAL_POSITION):
                single_frame = BINGOICONFRAME_AREASTART + (goal->param.position.kind - AREAKIND_ISLAND);
                goto DISPLAY_SINGLE_ICON;
            case (GOAL_BREAKBOXANY):
                single_frame = BINGOICONFRAME_ALLBOXES;
                goto DISPLAY_SINGLE_ICON;
            case (GOAL_BREAKBOXKIND):
                single_frame = BINGOICONFRAME_ITEMSTART + goal->param.box_kind.kind;
                goto DISPLAY_SINGLE_ICON;
            case (GOAL_RIDEMACHINEKIND):
                single_frame = BINGOICONFRAME_MACHINESTART + goal->param.ride_machine_kind.kind;
                goto DISPLAY_SINGLE_ICON;
            case (GOAL_RAILDISTANCE):
                single_frame = BINGOICONFRAME_GRIND;
                goto DISPLAY_SINGLE_ICON;
            case (GOAL_GLIDETIME):
                single_frame = BINGOICONFRAME_GLIDE;
                goto DISPLAY_SINGLE_ICON;
            case (GOAL_BOOSTRING):
                single_frame = BINGOICONFRAME_BOOSTRING;
                goto DISPLAY_SINGLE_ICON;
            DISPLAY_SINGLE_ICON:
                JObj_SetFlagsAll(multi_icon, JOBJ_HIDDEN);          // hide multi icons
                JObj_ClearFlagsAll(single_icon, JOBJ_HIDDEN);       // show icon
                JObj_SetFrameAndRate(single_icon, single_frame, 0);
                break;

            // multi icon
            case (GOAL_BREAKBOXWITHATTACK):
                multi_frame[2] = attack_to_frame[goal->param.box_attack.attack];
                multi_frame[1] = BINGOICONFRAME_KO;
                multi_frame[0] = BINGOICONFRAME_ALLBOXES;
                goto DISPLAY_MULTI_ICON;
            case (GOAL_HITPLAYER):
                multi_frame[2] = BINGOICONFRAME_PLAYER;
                multi_frame[1] = BINGOICONFRAME_HIT;
                multi_frame[0] = BINGOICONFRAME_PLAYER;
                goto DISPLAY_MULTI_ICON;
            case (GOAL_HITPLAYERWITHATTACK):
                multi_frame[2] = attack_to_frame[goal->param.hit_ply_attack.attack];
                multi_frame[1] = BINGOICONFRAME_HIT;
                multi_frame[0] = BINGOICONFRAME_PLAYER;
                goto DISPLAY_MULTI_ICON;
            case (GOAL_KOPLAYER):
                multi_frame[2] = BINGOICONFRAME_PLAYER;
                multi_frame[1] = BINGOICONFRAME_KO;
                multi_frame[0] = BINGOICONFRAME_PLAYER;
                goto DISPLAY_MULTI_ICON;
            case (GOAL_DESTROYMACHINE):
                multi_frame[2] = BINGOICONFRAME_PLAYER;
                multi_frame[1] = BINGOICONFRAME_KO;
                multi_frame[0] = BINGOICONFRAME_MACHINESTART + VCKIND_WARP;
                goto DISPLAY_MULTI_ICON;

            DISPLAY_MULTI_ICON:
                JObj_SetFlagsAll(single_icon, JOBJ_HIDDEN);          // hide single icons
                for (int i = 0; i < 3; i++)
                    JObj_SetFrameAndRate(JObj_GetIndex(multi_icon, 1 + i), multi_frame[i], 0);
                break;
        }
    }

    // display number
    if (!(goal->kind == GOAL_RAILDISTANCE || goal->kind == GOAL_RAILDISTANCE || goal->kind == GOAL_POSITION))
    {
        int goal_num = goal->num;
        JOBJ *disabled_num_j, *enabled_num_j;
        if (goal_num > 9) {
            enabled_num_j = double_digit;
            disabled_num_j = single_digit;
        }
        else {
            enabled_num_j = single_digit;
            disabled_num_j = double_digit;
        }

        // hide other digits
        JObj_SetFlagsAll(disabled_num_j, JOBJ_HIDDEN);

        // display digits
        JOBJ *digit_j = enabled_num_j;
        int num = goal_num;
        int loop_num = 0;
        while (num > 0)
        {
            int digit = num % 10;
            
            JOBJ *num_j = JObj_GetIndex(digit_j, 1 + loop_num);
            JObj_SetFrameAndRate(num_j, digit, 0);

            num = num / 10;
            loop_num++;
        }
    }
    else
    {
        JObj_SetFlagsAll(single_digit, JOBJ_HIDDEN);
        JObj_SetFlagsAll(double_digit, JOBJ_HIDDEN);
    }

}
void Bingo_UpdateIconProgress(BingoGoal *goal, int progress, JOBJ *icon_j, int progres_joint_idx)
{
    // update progress
    int fill_frame = ((float)progress / (float)goal->num) * 100.0f;
    JOBJ *fill_icon_j = JObj_GetIndex(icon_j, progres_joint_idx);
    JObj_SetFrameAndRate(fill_icon_j, fill_frame, 0);
}

// Bingo UI
int text_canvas_idx = 0;
GOBJ *BingoUI_Create(int ply)
{
    int tick_start = OSGetTick();
    int tick;

    Vec3 text_pos;
    Text *t;

    // create gobj and model
    GOBJ *b = GOBJ_EZCreator(0, GAMEPLINK_CAMHUD, 0,
                            sizeof(BingoUIData), BingoUI_Destroy,
                            HSD_OBJKIND_JOBJ, card_set->jobj, 
                            BingoUI_Think, 22, 
                            JObj_GX, GAMEGX_HUD, 3);

    BingoUIData *bd = b->userdata;
    bd->ply = ply;

    // create text
    {
        int x_pos;

        // details 
        JObj_GetChildPosition(b->hsd_object, BINGO_UI_DESCRIPTION_TEXT_JOINT, &text_pos);
        t = Text_CreateText(BINGO_SIS_INDEX, text_canvas_idx);
        // t->viewport_color = (GXColor){255, 0, 0, 128};
        t->kerning = 1;
        t->align = 0;
        t->viewport_scale = (Vec2){0.045, 0.055};
        t->use_aspect = 1;
        t->aspect = (Vec2){370, 64};
        t->trans.X = text_pos.X;
        t->trans.Y = -text_pos.Y;
        x_pos = 0;
        Text_AddSubtext(t, x_pos, 0, "Lorem ipsum dolor sit amet");
        Text_AddSubtext(t, x_pos, 30, "sed do eiusmod tempor incididunt.");
        bd->text.details = t;

        // create scoreboard
        JObj_GetChildPosition(b->hsd_object, BINGO_UI_SCOREBOARD_TEXT_JOINT, &text_pos);

        // labels
        t = Text_CreateText(BINGO_SIS_INDEX, text_canvas_idx);
        // t->viewport_color = (GXColor){255, 0, 0, 128};
        t->kerning = 1;
        t->align = 0;
        t->viewport_scale = (Vec2){0.045 * 0.8, 0.055 * 0.8};
        t->use_aspect = 1;
        t->aspect = (Vec2){250, 160};
        t->trans.X = text_pos.X;
        t->trans.Y = -text_pos.Y;
        Text_AddSubtext(t, 0, 0, "Game Info");
        Text_AddSubtext(t, 330, 0, "Leaderboard");
        bd->text.scoreboard.label = t;

        // game info
        t = Text_CreateText(BINGO_SIS_INDEX, text_canvas_idx);
        // t->viewport_color = (GXColor){255, 0, 0, 128};
        t->kerning = 1;
        t->align = 0;
        t->viewport_scale = (Vec2){0.045 * 0.8, 0.055 * 0.8};
        t->use_aspect = 1;
        t->aspect = (Vec2){250, 160};
        t->trans.X = text_pos.X;
        t->trans.Y = -text_pos.Y;
        x_pos = 0;
        Text_AddSubtext(t, x_pos, 50, "Mode: " "\x0C\xFF\xFF\x50" "Standard");
        Text_AddSubtext(t, x_pos, 80, "Goal: " "\x0C\xFF\xFF\x50" "%d items", 10);
        Text_AddSubtext(t, x_pos, 110, "Time Limit: Off");
        Text_AddSubtext(t, x_pos, 140, "Team: " "\x0C\x26\x26\xD9" "Blue");
        bd->text.scoreboard.game_info = t;

        // leaderboard numbers
        t = Text_CreateText(BINGO_SIS_INDEX, text_canvas_idx);
        // t->viewport_color = (GXColor){255, 0, 0, 128};
        t->kerning = 1;
        t->align = 1;
        t->viewport_scale = (Vec2){0.045 * 0.8, 0.055 * 0.8};
        t->use_aspect = 1;
        t->aspect = (Vec2){250, 160};
        t->trans.X = text_pos.X;
        t->trans.Y = -text_pos.Y;
        x_pos = 340;
        Text_AddSubtext(t, x_pos, 50, "1.");
        Text_AddSubtext(t, x_pos, 80, "2.");
        Text_AddSubtext(t, x_pos, 110, "3.");
        Text_AddSubtext(t, x_pos, 140, "4.");
        bd->text.scoreboard.leaderboard_nums = t;
        
        // teams
        t = Text_CreateText(BINGO_SIS_INDEX, text_canvas_idx);
        // t->viewport_color = (GXColor){255, 0, 0, 128};
        t->kerning = 1;
        t->align = 0;
        t->viewport_scale = (Vec2){0.045 * 0.8, 0.055 * 0.8};
        t->use_aspect = 1;
        t->aspect = (Vec2){100, 160};
        t->trans.X = text_pos.X;
        t->trans.Y = -text_pos.Y;
        x_pos = 375;
        Text_AddSubtext(t, x_pos, 50, "\x0C\x26\x26\xD9" "Blue:" );
        Text_AddSubtext(t, x_pos, 80, "\x0C\xFF\xB2\xCC" "Pink:");
        Text_AddSubtext(t, x_pos, 110, "\x0C\x40\xFF\x40" "Green:");
        Text_AddSubtext(t, x_pos, 140, "\x0C\xFF\x40\x40" "Red:");
        bd->text.scoreboard.leaderboard_teams = t;

        // team scores
        t = Text_CreateText(BINGO_SIS_INDEX, text_canvas_idx);
        // t->viewport_color = (GXColor){255, 0, 0, 128};
        t->kerning = 1;
        t->align = 0;
        t->viewport_scale = (Vec2){0.045 * 0.8, 0.055 * 0.8};
        t->use_aspect = 1;
        t->aspect = (Vec2){140, 160};
        t->trans.X = text_pos.X;
        t->trans.Y = -text_pos.Y;
        x_pos = 490;
        Text_AddSubtext(t, x_pos, 50, "12 items");
        Text_AddSubtext(t, x_pos, 80, "0 items");
        Text_AddSubtext(t, x_pos, 110, "0 items");
        Text_AddSubtext(t, x_pos, 140, "0 items");
        bd->text.scoreboard.leaderboard_scores = t;

    }

    // create bingo icons
    JOBJ *card_j = JObj_GetIndex(b->hsd_object, 1);
    float width = 18.5;
    for (int x = 0; x < BINGO_UI_GRID_SIZE; x++)
    {
        for (int y = 0; y < BINGO_UI_GRID_SIZE; y++)
        {
            int goal_idx = (x * BINGO_UI_GRID_SIZE) + y;
            BingoGoal *goal = &g_bingo_card.goal[goal_idx];


            JOBJ *icon_j = JObj_LoadJoint(icon_set->jobj);
            JObj_AddSetAnim(icon_j, 0, icon_set, 0, 0);
            JObj_AddNext(card_j, icon_j);

            Bingo_SetIconForGoal(goal, icon_j, BINGO_UI_JOINT_SINGLE_ICON,
                                               BINGO_UI_JOINT_MULTI_ICON,
                                               BINGO_UI_JOINT_SINGLE_DIGIT,
                                               BINGO_UI_JOINT_DOUBLE_DIGIT);

            // position on grid
            icon_j->trans.X = -(width / 2) + (width * ((float)x / (float)(BINGO_UI_GRID_SIZE - 1)));
            icon_j->trans.Y = -(width / 2) + (width * ((float)y / (float)(BINGO_UI_GRID_SIZE - 1)));
            JObj_SetMtxDirtySub(icon_j);

            int index = (x * BINGO_UI_GRID_SIZE) + y;
            bd->icon_arr[index] = icon_j;
        }
    }
    JObj_SetMtxDirtySub(card_j);

    bingo_card_gobj[ply] = b;

    // update UI
    BingoUI_Think(b);

    OSReport("Created Bingo UI in %.2fms\n", MillisecondsSinceTick(tick_start));

    return b;
}
void BingoUI_Think(GOBJ *g)
{
    BingoUIData *gp = g->userdata;
    BingoTrackerData *tp = bingo_tracker_gobj[gp->ply]->userdata;

    // update cursor
    HSD_Pad *pad = &stc_engine_pads[Ply_GetControllerIndex(gp->ply)];
    if (pad->down & PAD_BUTTON_DPAD_RIGHT)
        if (++bingo_cursor[gp->ply].x > BINGO_UI_GRID_SIZE - 1) bingo_cursor[gp->ply].x = 0;
    if (pad->down & PAD_BUTTON_DPAD_LEFT)
        if (--bingo_cursor[gp->ply].x < 0) bingo_cursor[gp->ply].x = BINGO_UI_GRID_SIZE - 1;
    if (pad->down & PAD_BUTTON_DPAD_UP)
        if (++bingo_cursor[gp->ply].y > BINGO_UI_GRID_SIZE - 1) bingo_cursor[gp->ply].y = 0;
    if (pad->down & PAD_BUTTON_DPAD_DOWN)
        if (--bingo_cursor[gp->ply].y < 0) bingo_cursor[gp->ply].y = BINGO_UI_GRID_SIZE - 1;

    int sel_icon_idx = (bingo_cursor[gp->ply].x * BINGO_UI_GRID_SIZE) + bingo_cursor[gp->ply].y;
    BingoGoal *sel_goal = &g_bingo_card.goal[sel_icon_idx];

    // update icons 
    for (int x = 0; x < BINGO_UI_GRID_SIZE; x++)
    {
        for (int y = 0; y < BINGO_UI_GRID_SIZE; y++)
        {
            int index = (x * BINGO_UI_GRID_SIZE) + y;

            // update highlight
            int outline_frame = (x == bingo_cursor[gp->ply].x && y == bingo_cursor[gp->ply].y) ? 1 : 0;
            JOBJ *outline_icon_j = JObj_GetIndex(gp->icon_arr[index], BINGO_UI_JOINT_BACKGROUND_OUTLINE);
            JObj_SetFrameAndRate(outline_icon_j, outline_frame, 0);
            
            BingoGoal *goal = &g_bingo_card.goal[index];
            Bingo_UpdateIconProgress(goal, tp->progress[index], gp->icon_arr[index], BINGO_UI_JOINT_BACKGROUND_FILL);
        }
    }

    // update description
    char s[128];
    Bingo_GetDescriptionForGoal(sel_goal, s);
    Text_SetText(gp->text.details, 0, s);
    
    Text_SetText(gp->text.details, 1, "Progress: %d \x81\x5E %d.", tp->progress[sel_icon_idx], sel_goal->num);

}
void BingoUI_Destroy(BingoUIData *bd)
{
    Text_Destroy(bd->text.details);
    Text_Destroy(bd->text.scoreboard.game_info);
    Text_Destroy(bd->text.scoreboard.label);
    Text_Destroy(bd->text.scoreboard.leaderboard_nums);
    Text_Destroy(bd->text.scoreboard.leaderboard_teams);
    Text_Destroy(bd->text.scoreboard.leaderboard_scores);

    bingo_card_gobj[bd->ply] = 0;

    HSD_Free(bd);
}
void Bingo_DestroyOnPause()
{
    // destroys any existing bingo UI when pausing
    if (!is_bingo_mode || !(Gm_IsInCity() && Gm_GetCityMode() == CITYMODE_TRIAL))
        return;

    Game3dData *g3d = Gm_Get3dData();

    // destroy any existing stat screens
    for (int ply = 0; ply < GetElementsIn(bingo_card_gobj); ply++)
    {
        if (Ply_GetPKind(ply) == PKIND_NONE)
            continue;

        if (bingo_card_gobj[ply])
            GObj_Destroy(bingo_card_gobj[ply]);
    }
}

void BingoInput_Create()
{
    // add proc to rider that checks to create the bingo UI
    if (!is_bingo_mode || Gm_GetCityMode() != CITYMODE_TRIAL)
        return;

    // create text canvas
    text_canvas_idx = Text_CreateCanvas(BINGO_SIS_INDEX, -1, 28, GAMEPLINK_CAMHUD, 0, GAMEGX_HUD, 3, 0);

    // add new proc to each rider with a viewport
    for (int i = 0; i < GetElementsIn(bingo_card_gobj); i++)
    {
        if (Ply_GetPKind(i) == PKIND_NONE || Gm_Get3dData()->plyview_lookup[i] == -1)
            continue;

        GOBJ *r = Ply_GetRiderGObj(i);
        GObj_AddProc(r, BingoInput_Think, 3);
    }
}
void BingoInput_Think(GOBJ *r)
{
    // not during the intro
    if (Gm_GetIntroState() != GMINTRO_END)
        return;

    RiderData *rp = r->userdata;
    Game3dData *g3d = Gm_Get3dData();

    GOBJ *b = bingo_card_gobj[rp->ply];
    
    if (!b && rp->input.held & PAD_BUTTON_Y)
    {
        BingoUI_Create(rp->ply);        
        Gm_HideHUD();
    }
    else if (b)
    {
        // destroy if released button
        if (!(rp->input.held & PAD_BUTTON_Y))
        {
            GObj_Destroy(b);
            Gm_ShowHUD();
        }
        // update UI
        else
        {
            // 
        }
    }
}

Text *debug_text;
void BingoTracker_Create()
{
    // add proc to rider that updates bingo progress
    if (!is_bingo_mode || Gm_GetCityMode() != CITYMODE_TRIAL)
        return;

    // create a new gobj for each player
    for (int i = 0; i < GetElementsIn(bingo_card_gobj); i++)
    {
        if (Ply_GetPKind(i) == PKIND_NONE)
            continue;

        GOBJ *t = GOBJ_EZCreator(0, GAMEPLINK_11, 0,
                                sizeof(BingoTrackerData), HSD_Free,
                                HSD_OBJKIND_NONE, 0, 
                                BingoTracker_Think, 21, 
                                BingoTracker_GX, GAMEGX_MAP, 1);

        bingo_tracker_gobj[i] = t;

        BingoTrackerData *tp = t->userdata;
        tp->ply = i;

        // init data
        for (int j = 0; j < BINGO_UI_GRID_SIZE * BINGO_UI_GRID_SIZE; j++)
            tp->progress[j] = 0;

        if (Ply_IsViewOn(i))
        {
            ;
        }
    }

    // create gobj proc to clear the damage log before updating hitcoll
    GOBJ *d = GOBJ_EZCreator(0, GAMEPLINK_11, 0,
                            0, 0,
                            HSD_OBJKIND_NONE, 0, 
                            Log_Clear, 0, 
                            0, 0, 0);

    // debug_text = Hoshi_CreateScreenText();
    // for (int i = 0; i < 2; i++)
    //     Text_AddSubtext(debug_text, 0, i * 30, "");

}
void BingoTracker_Think(GOBJ *t)
{
    BingoTrackerData *tp = t->userdata;
    RiderData *rd = Ply_GetRiderGObj(tp->ply)->userdata;
    MachineData *md = (rd->machine_gobj) ? rd->machine_gobj->userdata : 0;
    int is_view_on = Ply_IsViewOn(tp->ply);

    // update misc stats
    if (md && (md->is_airborne && (md->status == VCSTATE_FLY || md->status == VCSTATE_FLYPUSH)))
        tp->stats.glide_frames++;
    else
        tp->stats.glide_frames = 0;

    BingoGoalSFXKind sfx = GOALSFX_NONE;

    // update progress
    for (int goal_idx = 0; goal_idx < BINGO_UI_GRID_SIZE * BINGO_UI_GRID_SIZE; goal_idx++)
    {
        BingoGoal *goal = &g_bingo_card.goal[goal_idx];

        // skip if goal is already completed
        if (goal->ply_completed != -1)
            continue;

        int progress = Bingo_UpdateProgress(tp, goal_idx);

        if (progress != tp->progress[goal_idx])
        {   
            if (is_view_on)
            {
                BingoNotif_Create(goal, progress, tp->ply);

                // check if completed
                if (progress >= goal->num)
                {
                    goal->ply_completed = tp->ply;
                    sfx = GOALSFX_COMPLETE;
                }
                else
                {
                    // play sound if progress went up
                    if (progress > tp->progress[goal_idx])
                        sfx = GOALSFX_UP;
                    else
                        sfx = GOALSFX_DOWN;
                }
            }

            // update result
            tp->progress[goal_idx] = progress;
        }

    }

    if (sfx != GOALSFX_NONE)
    {
        static int goal_sfx_lookup[] = {
            FGMMENU_CS_BEEP1,
            FGMMENU_CS_KETTEI_PRE,
            FGMMENU_CS_KETTEI,
        };
        
        SFX_Play(goal_sfx_lookup[sfx]);
    }



    for (int i = 0; i < GetElementsIn(g_dmg_log); i++)
    {
        DamageLog *this_log = &g_dmg_log[i];
        if (this_log->dmg > 0)
        {
            static char *hurt_kind_names[] = {
                "rider",
                "machine",
                "machine_empty",
                "3",
                "item",
                "weapon",
                "map",
            };
            static char *action_names[] = {
                "hurt",
                "KO'd",
            };


            // to-do: this crashes when taking damage from picking up a fake item
            OSReport("%s (ply %d, kind %d, state: %d/%d, attack_kind: %d, is_airborne: %d) %s %s (ply %d, kind %d, state: %d/%d, is_airborne: %d) with %.2f damage\n", 
                                                        hurt_kind_names[this_log->attacker.hurt_kind],
                                                        this_log->attacker.ply,
                                                        this_log->attacker.kind,
                                                        this_log->attacker.state,
                                                        this_log->attacker.state2,
                                                        this_log->attacker.attack_data.kind,
                                                        this_log->attacker.is_airborne,
                                                        action_names[this_log->is_ko],
                                                        hurt_kind_names[this_log->victim.hurt_kind],
                                                        this_log->victim.ply,
                                                        this_log->victim.kind,
                                                        this_log->victim.state,
                                                        this_log->victim.state2,
                                                        this_log->victim.is_airborne,
                                                        this_log->dmg
                                                        );
        }
    }
    for (int i = 0; i < GetElementsIn(g_rail_log); i++)
    {
        RailLog *this_log = &g_rail_log[i];
        if (this_log->ply != -1)
            OSReport("ply %d entered rail %d with progress %.2f in status %d with ground state %d\n", this_log->ply, this_log->idx, this_log->progress, this_log->status, this_log->is_airborne);
    }
    for (int i = 0; i < GetElementsIn(g_zone_log); i++)
    {
        ZoneLog *this_log = &g_zone_log[i];
        if (this_log->ply != -1)
            OSReport("ply %d entered zone index %d with kind %d\n", this_log->ply, this_log->idx, this_log->kind);
    }

    // RiderData *rp = Ply_GetRiderGObj(tp->ply)->userdata;
    // Text_SetText(debug_text, 0, "%.2f, %.2f, %.2f", rp->pos.X, rp->pos.Y, rp->pos.Z);
    // Text_SetText(debug_text, 1, "", rp->pos.X, rp->pos.Y, rp->pos.Z);

    // for (int i = 0; i < GetElementsIn(g_area_bounds); i++)
    // {
    //     float hx = g_area_bounds[i].size.X * 0.5f;
    //     float hy = g_area_bounds[i].size.Y * 0.5f;
    //     float hz = g_area_bounds[i].size.Z * 0.5f;

    //     float x_min = g_area_bounds[i].pos.X - hx;
    //     float x_max = g_area_bounds[i].pos.X + hx;
    //     float y_min = g_area_bounds[i].pos.Y - hy;
    //     float y_max = g_area_bounds[i].pos.Y + hy;
    //     float z_min = g_area_bounds[i].pos.Z - hz;
    //     float z_max = g_area_bounds[i].pos.Z + hz;

    //     if (rp->pos.X >= x_min && rp->pos.X <= x_max && 
    //         rp->pos.Y >= y_min && rp->pos.Y <= y_max && 
    //         rp->pos.Z >= z_min && rp->pos.Z <= z_max)
    //         {
    //             Text_SetText(debug_text, 1, "area: %d", i);
    //             break;
    //         }
    // }

}
void BingoTracker_GX(GOBJ *t, int pass)
{
    return;

    if (pass != 2)
        return;

    // draw areas
    for (int i = 0; i < GetElementsIn(g_area_bounds); i++)
        GX_DrawBox(&g_area_bounds[i].pos, &g_area_bounds[i].size, &(GXColor){255,0,0,128});
}

GOBJ *BingoNotif_Create(BingoGoal *goal, int progress, int ply)
{
    // find an existing notif for this goal
    GOBJ *notif_g = 0;
    for (GOBJ *g = (*stc_gobj_lookup)[GAMEPLINK_HUD]; g;)
    {
        GOBJ *next = g->next;

        if (g->entity_class == 100)
        {
            BingoNotifData *gp = g->userdata;

            // if a notif for this exact goal is already onscreen, use it
            if (gp->goal == goal)
            {
                notif_g = g;
                break;
            }

            // if this notif was just created
            else if (gp->timer == 0)
            {
                if (gp->goal->kind == goal->kind)
                {
                    // when there are similar goals, prioritize the one with a lower number requirement
                    if (goal->num < gp->goal->num)
                        GObj_Destroy(g);    // keep only the lower number goal notif
                    else
                        return 0;           // dont make a new notif if a lower num one exists
                }
                // refer to the priority
                else if (gp->goal->kind > goal->kind)
                    GObj_Destroy(g);        // destroy this notif and create a new one
                else
                    return 0;               // dont make a new notif if its a lower priority
            }
            else
                GObj_Destroy(g);    // out with the old, in with the new
        }

        g = next;
    }

    // create new notif
    if (!notif_g)
    {
        notif_g = GOBJ_EZCreator(100, GAMEPLINK_HUD, 0,
                            sizeof(BingoNotifData), BingoNotif_Destroy,
                            HSD_OBJKIND_JOBJ, notif_set->jobj, 
                            BingoNotif_Think, 22, 
                            BingoNotif_GX, GAMEGX_HUD, 1);

        BingoNotifData *notif_data = notif_g->userdata;
        notif_data->goal = goal;
        notif_data->ply = ply;
        notif_data->timer = 0;

        JOBJ *notif_j = notif_g->hsd_object;
        JObj_AddSetAnim(notif_j, 0, notif_set, 0, 0);
        JObj_SetFrameAndRate(notif_j, 0, 0);

        // start movement anim
        JObj_SetFrameAndRate(JObj_GetIndex(notif_j, BINGO_NOTIF_JOINT_MOVE), 0, 1);
        
        // set goal icon
        Bingo_SetIconForGoal(goal, notif_j, BINGO_NOTIF_JOINT_SINGLE_ICON, 
                                            BINGO_NOTIF_JOINT_MULTI_ICON,
                                            BINGO_NOTIF_JOINT_SINGLE_DIGIT,
                                            BINGO_NOTIF_JOINT_DOUBLE_DIGIT);

        char s[64];
        Bingo_GetDescriptionForGoal(goal, s);
        // sprintf("Progress: %d \x81\x5E %d.", progress, goal->num);
        
        // add text
        Text *t = Text_CreateText(BINGO_SIS_INDEX, text_canvas_idx);
        t->gobj->gx_cb = 0; // remove gx callback and render from notif gx
        // t->viewport_color = (GXColor){255, 0, 0, 128};
        t->color = (GXColor){255, 255, 255, 255};
        t->kerning = 1;
        t->align = 0;
        t->viewport_scale = (Vec2){0.045, 0.055};
        t->use_aspect = 1;
        t->aspect = (Vec2){350, 64};
        Text_AddSubtext(t, 0, 0, s);
        notif_data->t = t;
    }
    else
    {
        // refresh timer
        BingoNotifData *notif_data = notif_g->userdata;
        notif_data->timer = 0;
    }

    // update progress
    Bingo_UpdateIconProgress(goal, progress, notif_g->hsd_object, BINGO_NOTIF_JOINT_BACKGROUND_FILL);

    return notif_g;
}
void BingoNotif_Destroy(BingoNotifData *gp)
{
    Text_Destroy(gp->t);
    HSD_Free(gp);
}
void BingoNotif_Think(GOBJ *g)
{
    BingoNotifData *gp = g->userdata;
    JOBJ *j = g->hsd_object;

    JObj_AnimAll(j);

    if (!JObj_CheckAObjPlaying(JObj_GetIndex(j, BINGO_NOTIF_JOINT_MOVE)))
    {
        if (++gp->timer > BINGO_NOTIF_PARAM_TIMER)
        {
            GObj_Destroy(g);
            return;
        }
    }

    // update text position
    Vec3 text_pos;
    JObj_GetChildPosition(j, BINGO_NOTIF_JOINT_TEXT, &text_pos);
    gp->t->trans.X = text_pos.X;
    gp->t->trans.Y = -text_pos.Y;

}
void BingoNotif_GX(GOBJ *g, int pass)
{
    BingoNotifData *gp = g->userdata;

    // dont render when bingo card is up or game is paused
    if (*g_hud_is_hidden)
        return;

    JObj_GX(g, pass);
    Text_GX(gp->t->gobj, pass);
}

int Bingo_UpdateProgress(BingoTrackerData *tp, int goal_idx)
{
    int ply = tp->ply;
    int progress = tp->progress[goal_idx];
    BingoGoal *gd = &g_bingo_card.goal[goal_idx];

    RiderData *rp = Ply_GetRiderGObj(ply)->userdata;
    u8 *stats = (u8 *)Ply_GetStats(ply);
    MachineData *mp = (rp->machine_gobj) ? (rp->machine_gobj->userdata) : 0;

    switch(gd->kind)
    {
        case (GOAL_STATGET):
        {
            int *item_collect_arr = (int *)&stats[0x4c8];
            progress = item_collect_arr[gd->param.stat_get.kind];
            break;
        }
        case (GOAL_FOODGET):
        {
            int *item_collect_arr = (int *)&stats[0x4c8];
            progress = item_collect_arr[gd->param.food_get.kind];
            break;
        }
        case (GOAL_ITEMFALLGET):
        {
            break;
        }
        case (GOAL_POSITION):
        {
            AreaBound *bound = &g_area_bounds[gd->param.position.kind];

            float hx = bound->size.X * 0.5f;
            float hy = bound->size.Y * 0.5f;
            float hz = bound->size.Z * 0.5f;

            float x_min = bound->pos.X - hx;
            float x_max = bound->pos.X + hx;
            float y_min = bound->pos.Y - hy;
            float y_max = bound->pos.Y + hy;
            float z_min = bound->pos.Z - hz;
            float z_max = bound->pos.Z + hz;

            if (rp->pos.X >= x_min && rp->pos.X <= x_max && 
                rp->pos.Y >= y_min && rp->pos.Y <= y_max && 
                rp->pos.Z >= z_min && rp->pos.Z <= z_max)
                {
                    progress++;
                }

            break;
        }
        case (GOAL_BREAKBOXANY):
        {
            for (int i = 0; i < GetElementsIn(g_dmg_log); i++)
            {
                DamageLog *this_log = &g_dmg_log[i];
                if (this_log->dmg > 0 && this_log->attacker.ply == ply && 
                    this_log->is_ko &&
                    this_log->victim.hurt_kind == HURTKIND_ITEM && 
                    this_log->victim.kind >= ITKIND_BOXBLUE && this_log->victim.kind <= ITKIND_BOXRED)
                {
                    progress++;
                }
            }

            break;
        }
        case (GOAL_BREAKBOXKIND):
        {
            for (int i = 0; i < GetElementsIn(g_dmg_log); i++)
            {
                DamageLog *this_log = &g_dmg_log[i];
                if (this_log->dmg > 0 && this_log->attacker.ply == ply && 
                    this_log->is_ko &&
                    this_log->victim.hurt_kind == HURTKIND_ITEM && 
                    this_log->victim.kind == gd->param.box_kind.kind)
                {
                    progress++;
                }
            }
            break;
        }
        case (GOAL_BREAKBOXWITHATTACK):
        {
            for (int i = 0; i < GetElementsIn(g_dmg_log); i++)
            {
                DamageLog *this_log = &g_dmg_log[i];
                if (this_log->dmg > 0 && this_log->attacker.ply == ply && 
                    this_log->is_ko &&
                    this_log->victim.hurt_kind == HURTKIND_ITEM && 
                    this_log->victim.kind >= ITKIND_BOXBLUE && this_log->victim.kind <= ITKIND_BOXRED && 
                    this_log->attacker.attack_data.kind == gd->param.box_attack.attack)
                {
                    progress++;
                }
            }
            break;
        }
        case (GOAL_HITPLAYER):
        {
            for (int i = 0; i < GetElementsIn(g_dmg_log); i++)
            {
                DamageLog *this_log = &g_dmg_log[i];
                if (this_log->dmg > 0 && this_log->attacker.ply == ply && this_log->victim.ply != ply &&
                    (this_log->victim.hurt_kind == HURTKIND_RIDER || this_log->victim.hurt_kind == HURTKIND_MACHINE))
                {
                    progress++;
                }
            }
            break;
        }
        case (GOAL_HITPLAYERWITHATTACK):
        {
            for (int i = 0; i < GetElementsIn(g_dmg_log); i++)
            {
                DamageLog *this_log = &g_dmg_log[i];
                if (this_log->dmg > 0 && this_log->attacker.ply == ply && 
                    (this_log->victim.hurt_kind == HURTKIND_RIDER || this_log->victim.hurt_kind == HURTKIND_MACHINE) && 
                    this_log->attacker.attack_data.kind == gd->param.hit_ply_attack.attack)
                {
                    progress++;
                }
            }
            break;
        }
        case (GOAL_DESTROYMACHINE):
        {
            for (int i = 0; i < GetElementsIn(g_dmg_log); i++)
            {
                DamageLog *this_log = &g_dmg_log[i];
                if (this_log->dmg > 0 && this_log->attacker.ply == ply && 
                    (this_log->victim.hurt_kind == HURTKIND_MACHINE || this_log->victim.hurt_kind == HURTKIND_MACHINE_EMPTY) && 
                    this_log->is_ko)
                {
                    progress++;
                }
            }
            break;
        }
        case (GOAL_RIDEMACHINEKIND):
        {
            ;
            break;
        }
        case (GOAL_RAILDISTANCE):
        {   
            float rail_dist = *(float *)&stats[0x614];

            if ((int)rail_dist > gd->param.rail_dist.dist)
                progress++;

            break;
        }
        case (GOAL_RAILLAND):
        {
            for (int i = 0; i < GetElementsIn(g_rail_log); i++)
            {
                RailLog *this_log = &g_rail_log[i];
                if (this_log->ply == ply && this_log->is_airborne && this_log->progress > 0.2)
                    progress++;
            }
            break;
        }
        case (GOAL_GLIDETIME):
        {
            progress = tp->stats.glide_frames / 60;

            break;
        }
        case (GOAL_BOOSTRING):
        {
            static u8 boost_zone_ids[] = {26, 27, 28, 29, 58, 59, 60, 61, 62, 63, 64, 65, 66, 67, 69};

            u8 *zone_bits = &stats[0x661];
            int ring_num = 0;

            for (int i = 0; i < GetElementsIn(boost_zone_ids); i++)
            {
                int this_boost_idx = boost_zone_ids[i];

                int byte_offset = this_boost_idx / 8;
                int bit_idx = 1 << (this_boost_idx % 8);

                if (zone_bits[byte_offset] & bit_idx)
                    ring_num++;
            }

            progress = ring_num;

            break;
        }
    }

    return progress;
}

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

int Bingo_OnDPadToStick(RiderData *rd)
{
    // int button_mask = ~(PAD_BUTTON_DPAD_DOWN | PAD_BUTTON_DPAD_UP | PAD_BUTTON_DPAD_LEFT | PAD_BUTTON_DPAD_RIGHT);

    if (bingo_card_gobj[rd->ply])
        return 1;

    return 0;

}
CODEPATCH_HOOKCONDITIONALCREATE(0x8018f0bc, "mr 3, 31\n\t", Bingo_OnDPadToStick, "", 0, 0x8018f0cc)

void GOBJProc_Log(GOBJProc *proc, int level)
{
    while (proc)
    {
        int s_link = (proc->gobj) ? proc->gobj->p_link : -1;
        OSReport(" %*sProc %p s_link %d. GOBJ %p (%d) callback %p\n", level, "", proc, proc->s_link, proc->gobj, s_link, proc->cb);

        // if (proc->child)
        //     GOBJProc_Log(proc->child, level + 1);

        proc = proc->next;
    }

    return;
}

void Bingo_Init()
{
    // apply our code patches
    CODEPATCH_HOOKAPPLY(0x8018f0bc);

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

    // yakumono logging
    // CODEPATCH_HOOKAPPLY(0x80105d90);     // events give all yakubreak hits to the lowest port player @ 80108334. disabling for now i guess
    
    // Hoshi_AddPreloadGameFile(BINGO_ASSET_FILENAME, PRELOADHEAPKIND_ALLM);
}
void Bingo_On3DLoadStart()
{
    // init bingo data
    for (int i = 0; i < GetElementsIn(bingo_card_gobj); i++)
    {
        bingo_card_gobj[i] = 0;
        bingo_cursor[i].x = (BINGO_UI_GRID_SIZE - 1) / 2;
        bingo_cursor[i].y = (BINGO_UI_GRID_SIZE - 1) / 2;;
    }

    // get our file
    HSD_Archive *archive;
    Gm_LoadGameFile(&archive, BINGO_ASSET_FILENAME);
    card_set = ((JOBJSet**)Archive_GetPublicAddress(archive, "IfBingoCard_scene_models"))[0];
    icon_set = ((JOBJSet**)Archive_GetPublicAddress(archive, "IfBingoIcon_scene_models"))[0];
    notif_set = ((JOBJSet**)Archive_GetPublicAddress(archive, "IfBingoNotif_scene_models"))[0];
}
void Bingo_On3DLoadEnd()
{
    BingoInput_Create();
    BingoTracker_Create();

    // u8 proc_num = 26; // *stc_gobj_proc_num;
    // for (int i = 0; i < proc_num; i++)
    // {
    //     OSReport("Checking proc %d\n", i);

    //     GOBJProc *proc = (*stc_gobjproc_lookup)[i];
    //     GOBJProc_Log(proc, 0);
    //     OSReport("\n");
    // }
    
}
void Bingo_On3DPause(int pause_ply)
{
    Bingo_DestroyOnPause();
}
void Bingo_On3DUnpause(int pause_ply)
{
}
void Bingo_OnPlayerSelectLoad()
{
    BingoMode_Start();

    Text *t = Hoshi_CreateScreenText();
    t->kerning = 1;
    t->use_aspect = 1;
    t->trans = (Vec3){640, 0, 0};
    t->align = TEXTALIGN_RIGHT;
    t->viewport_scale = (Vec2){0.5, 0.5};
    t->aspect = (Vec2){550, 32};
    t->viewport_color = (GXColor){0, 0, 0, 128};
    Text_AddSubtext(t, 0, 0, "Bingo " __DATE__ " " __TIME__);
}