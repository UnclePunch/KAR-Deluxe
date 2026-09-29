
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

#include "bingo.h"
#include "log.h"
#include "notif.h"
#include "card.h"
#include "tracker.h"

int is_bingo_mode = 1;

BingoCard g_bingo_card;

AreaBound g_area_bounds[] = {
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
    // ride machine any
    {
        .kind = GOAL_RIDEMACHINEANY,
        .num = 3,
        .param.ride_machine_any = {
            {.difficulty = DFCLT_EASY, .num_min = 1, .num_max = 1,},
            {.difficulty = DFCLT_MEDIUM, .num_min = 2, .num_max = 3,},
            {.difficulty = DFCLT_HARD, .num_min = 4, .num_max = 5,},
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

static u8 vckind_to_character[] = {
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
            "BREAKBOXWITHATTACK",
            "BREAKBOXKIND",
            "BREAKBOXANY",
            "HITPLAYER",
            "HITPLAYERWITHATTACK",
            "KOPLAYER",
            "DESTROYMACHINE",
            "RIDEMACHINEKIND",
            "RIDEMACHINEANY",
            "RAILDISTANCE",
            "RAILLAND",
            "BOOSTRING",
            "GLIDETIME",
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
        case (GOAL_RIDEMACHINEANY):
        {
            bp();
            gd->num = RandomInRange(desc->param.ride_machine_any[param_idx].num_min, desc->param.ride_machine_any[param_idx].num_max);
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
        case (GOAL_RIDEMACHINEANY):
        {
            sprintf(out, "Ride %d different machine\x81\x69s\x81\x6a.", gd->num);
            break;
        }
        case (GOAL_RIDEMACHINEKIND):
        {
            sprintf(out, "Ride %s.", machine_names[vckind_to_character[gd->param.ride_machine_kind.kind]]);
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
            case (GOAL_RIDEMACHINEANY):
                single_frame = BINGOICONFRAME_MACHINESTART + CKIND_COMPACT;
                goto DISPLAY_SINGLE_ICON;            
            case (GOAL_RIDEMACHINEKIND):
                single_frame = BINGOICONFRAME_MACHINESTART + vckind_to_character[goal->param.ride_machine_kind.kind];
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
    if (!(goal->kind == GOAL_RAILDISTANCE || goal->kind == GOAL_RAILDISTANCE || goal->kind == GOAL_POSITION || goal->kind == GOAL_RIDEMACHINEKIND))
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
    BingoLog_Init();
    BingoUI_OnBoot();

    // yakumono logging
    // CODEPATCH_HOOKAPPLY(0x80105d90);     // events give all yakubreak hits to the lowest port player @ 80108334. disabling for now i guess
    
    // Hoshi_AddPreloadGameFile(BINGO_ASSET_FILENAME, PRELOADHEAPKIND_ALLM);
}
void Bingo_On3DLoadStart()
{
    BingoUI_On3DLoadStart();    
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
    BingoUI_DestroyOnPause();
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