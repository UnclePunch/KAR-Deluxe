/*---------------------------------------------------------------------------*
    Entrypoint for the Bingo module.

 *---------------------------------------------------------------------------*/

#include "hoshi/mod.h"

#include "bingo.h"

#include "wide/wide.h"

WideExport *g_wide_export = 0;

int g_is_lockout = 0;
int g_is_teams = 0;
int g_goals_to_win = 25;

OptionDesc mod_settings = {
    // Menu
    .name = "Bingo Settings",
    .description = "Tweak Bingo mode.",
    .pri = MENUPRI_NORMAL,
    .kind = OPTKIND_MENU,
    .menu_ptr = &(MenuDesc){
        .option_num = 4,
        .options = {
            &(OptionDesc){
                .name = "Enter Bingo",
                .description = "gogogogo!",
                .kind = OPTKIND_SCENE,
            },
            &(OptionDesc){
                .name = "Lockout",
                .description = "Opponents cannot claim completed goals.",
                .kind = OPTKIND_VALUE,
                .val = &g_is_lockout,
                .value_num = 2,
                .value_names = (char *[]){
                    "Off",
                    "On",
                },
            },
            &(OptionDesc){
                .name = "Goal Amount",
                .description = "Goals required to win.",
                .kind = OPTKIND_NUM,
                .val = &g_goals_to_win,
                .min = 1,
                .max = 25,
            },
            &(OptionDesc){
                .name = "Teams",
                .description = "Players with the same color share a team.",
                .kind = OPTKIND_VALUE,
                .val = &g_is_teams,
                .value_num = 2,
                .value_names = (char *[]){
                    "Off",
                    "On",
                },
            },
        },
    },
};

void OnBoot()
{
    Bingo_Init();
    mod_settings.menu_ptr->options[0]->major_idx = Bingo_InstallScene();
    
    return;
}

void OnSaveLoaded()
{
    g_wide_export = Hoshi_ImportMod("Widescreen", 1, 0);
    Bingo_OnSaveLoaded();
    return;
}

void On3DLoadStart()
{
    Bingo_On3DLoadStart();
    return;
}

void On3DLoadEnd()
{
    Bingo_On3DLoadEnd();
    return;
}

void On3DPauseStart(int pause_ply)
{
    Bingo_On3DPauseStart(pause_ply);
    return;
}

void On3DUnpause(int pause_ply)
{
    Bingo_On3DUnpause(pause_ply);
    return;
}

void OnPlayerSelectLoad()
{
    Bingo_OnPlayerSelectLoad();
    return;
}

void OnReset()
{
    Bingo_OnReset();
}

ModDesc mod_desc = {
    .name = "Bingo",
    .author = "UnclePunch",
    .version.major = 1,
    .version.minor = 0,
    .affects_gameplay = false,
    .option_desc = &mod_settings,
    .OnBoot = OnBoot,
    .OnSaveLoaded = OnSaveLoaded,
    .On3DLoadStart = On3DLoadStart,
    .On3DLoadEnd = On3DLoadEnd,
    .On3DPauseStart = On3DPauseStart,
    .On3DUnpause = On3DUnpause,
    .OnPlayerSelectLoad = OnPlayerSelectLoad,
    .OnReset = OnReset,
};