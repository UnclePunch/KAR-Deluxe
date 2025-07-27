
#include "text.h"
#include "os.h"
#include "hsd.h"
#include "preload.h"
#include "scene.h"
#include "inline.h"
#include "audio.h"
#include "hoshi/settings.h"
#include "fst/fst.h"

#include "attractmode.h"
#include "brake.h"
#include "quickstats.h"
#include "rearview.h"
#include "startup.h"
#include "unpause.h"

#include "competitive/run.h"
#include "competitive/drop_ability.h"
#include "competitive/sd_as_ko.h"
#include "competitive/intang_after_ko.h"

#define VERSION_MAJOR 1
#define VERSION_MINOR 0

extern StartupKind startup_kind;
extern int quickstats_enabled;
extern int brake_enabled;
extern int rearview_enabled;
extern int camerazoom_kind;
extern int attractmode_enabled;
extern int unpause_delay_enabled;
extern int is_run_enabled;
extern int ability_drop_enabled;
extern int sd_as_ko_enabled;
extern int intang_after_ko_enabled;

char ModName[] = "Game Settings";
char ModAuthor[] = "UnclePunch";
char ModVersion[] = "v" STR(VERSION_MAJOR) "." STR(VERSION_MINOR);

OptionDesc ModSettings = {
    // Controls Menu
    .name = "Game Settings",
    .description = "Finetune gameplay!",
    .pri = MENUPRI_HIGH,
    .kind = OPTKIND_MENU,
    .menu_ptr = &(MenuDesc){
        .option_num = 5,
        .options =
            {
                &(OptionDesc){
                    // Controls Menu
                    .name = "Controls",
                    .description = "Extra controls for enhanced play!",
                    .kind = OPTKIND_MENU,
                    .menu_ptr = &(MenuDesc){
                        .option_num = 5,
                        .options = {
                            &(OptionDesc){
                                .name = "Rear View",
                                .description = "Hold X to quickly see behind you!",
                                .kind = OPTKIND_VALUE,
                                .val = &rearview_enabled,
                                .value_num = 2,
                                .value_names = (char *[]){
                                    "Off",
                                    "On",
                                },
                            },
                            &(OptionDesc){
                                .name = "Quick Stats",
                                .description = "Hold Y to glance at your stats in the city!",
                                .kind = OPTKIND_VALUE,
                                .val = &quickstats_enabled,
                                .value_num = 2,
                                .value_names = (char *[]){
                                    "Off",
                                    "On",
                                },
                            },
                            &(OptionDesc){
                                .name = "Brake",
                                .description = "Press B to brake without charging!",
                                .kind = OPTKIND_VALUE,
                                .val = &brake_enabled,
                                .value_num = 2,
                                .value_names = (char *[]){
                                    "Off",
                                    "On",
                                },
                            },
                            &(OptionDesc){
                                .name = "Run",
                                .description = "Hold B on foot to run faster!",
                                .kind = OPTKIND_VALUE,
                                .val = &is_run_enabled,
                                .value_num = 2,
                                .value_names = (char *[]){
                                    "Off",
                                    "On",
                                },
                            },
                            &(OptionDesc){
                                .name = "Ability Drop",
                                .description = "Press Z to drop Kirby's copy ability!",
                                .kind = OPTKIND_VALUE,
                                .val = &ability_drop_enabled,
                                .value_num = 2,
                                .value_names = (char *[]){
                                    "Off",
                                    "On",
                                },
                            },
                        },
                    },
                },
                &(OptionDesc){
                    // Competitive Menu
                    .name = "Game Balance",
                    .description = "Adjust rules for a more fair fight!",
                    .kind = OPTKIND_MENU,
                    .menu_ptr = &(MenuDesc){
                        .option_num = 2,
                        .options = {
                            &(OptionDesc){
                                .name = "Invincible on Foot",
                                .description = "Kirby's invincible after getting KO'd!",
                                .kind = OPTKIND_VALUE,
                                .val = &intang_after_ko_enabled,
                                .value_num = 2,
                                .value_names = (char *[]){
                                    "Off",
                                    "On",
                                },
                            },
                            &(OptionDesc){
                                .name = "SD's Award Points",
                                .description = "Receive a point if a player self-destructs!",
                                .kind = OPTKIND_VALUE,
                                .val = &sd_as_ko_enabled,
                                .value_num = 2,
                                .value_names = (char *[]){
                                    "Off",
                                    "On",
                                },
                            },
                            // &(OptionDesc){
                            //     .name = "Unpause",
                            //     .description = "Display a countdown after unpausing the game!",
                            //     .kind = OPTKIND_VALUE,
                            //     .val = &unpause_delay_enabled,
                            //     .value_num = 2,
                            //     .value_names = (char *[]){
                            //         "Original",
                            //         "Countdown",
                            //     },
                            // },
                        },
                    },
                },
                &(OptionDesc){
                    .name = "Startup",
                    .description = "Pick which menu appears on startup.",
                    .kind = OPTKIND_VALUE,
                    .val = (int *)&startup_kind,
                    .value_num = 6,
                    .value_names = (char *[]){
                        "Opening Movie",
                        "Title Screen",
                        "Main Menu",
                        "City Trial",
                        "Air Ride",
                        "LAN",
                    },
                },
                &(OptionDesc){
                    .name = "Camera",
                    .description = "Set your default camera zoom level.",
                    .kind = OPTKIND_VALUE,
                    .val = &camerazoom_kind,
                    .value_num = 2,
                    .value_names = (char *[]){
                        "Original",
                        "Zoomed Out",
                    },
                },
                &(OptionDesc){
                    .name = "Attract Mode",
                    .description = "Display the attract mode during inactivity.",
                    .kind = OPTKIND_VALUE,
                    .val = &attractmode_enabled,
                    .value_num = 2,
                    .value_names = (char *[]){
                        "Off",
                        "On",
                    },
                },
            },
    },
};

void OnBoot(HSD_Archive *archive)
{

    return;
}
void OnSceneChange(HSD_Archive *archive)
{

    return;
}
void OnSaveInit(void *save, int req_init)
{
    Startup_Init();
    QuickStat_Init();
    Brake_Init();
    Camera_Init();
    AttractMode_Init();
    // UnpauseDelay_Init();
    Run_Init();
    AbilityDrop_Init();
    SD_as_KO_Init();
    IntangAfterKO_Init();

    return;
}
void On3DLoad()
{
    QuickStat_On3DStart();
    // UnpauseDelay_On3DStart();
    Rearview_InitFlags();
    IntangAfterKO_On3DLoad();
}
void On3DPause(int pause_ply)
{
    QuickStat_OnPause();
}