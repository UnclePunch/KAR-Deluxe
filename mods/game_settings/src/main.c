
#include "text.h"
#include "useful.h"
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
#include "run.h"
#include "startup.h"
#include "unpause.h"

extern StartupKind startup_kind;
extern int quickstats_enabled;
extern int brake_enabled;
extern int rearview_enabled;
extern int camerazoom_kind;
extern int attractmode_enabled;
extern int unpause_delay_enabled;

char ModName[] = "Game Settings";
char ModAuthor[] = "UnclePunch";
char ModVersion[] = "v" STR(VERSION_MAJOR) "." STR(VERSION_MINOR);

MenuDesc ModMenu = {
    .name = "Game Settings",
    .option_num = 5,
    .options = (OptionDesc[]){
        {
            // Controls Menu
            .kind = OPTKIND_MENU,
            .menu_ptr = &(MenuDesc){
                .name = "Controls",
                .option_num = 3,
                .options = (OptionDesc[]){
                    {
                        .name = "Brake",
                        .kind = OPTKIND_VALUE,
                        .val = &brake_enabled,
                        .value_num = 2,
                        .value_names = (char *[]){
                            "Off",
                            "On",
                        },
                    },
                    {
                        .name = "Rear View",
                        .kind = OPTKIND_VALUE,
                        .val = &rearview_enabled,
                        .value_num = 2,
                        .value_names = (char *[]){
                            "Off",
                            "On",
                        },
                    },
                    {
                        .name = "Quick Stats",
                        .kind = OPTKIND_VALUE,
                        .val = &quickstats_enabled,
                        .value_num = 2,
                        .value_names = (char *[]){
                            "Off",
                            "On",
                        },
                    },
                },
            },
        },
        {
            .name = "Startup",
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
        {
            .name = "Camera",
            .kind = OPTKIND_VALUE,
            .val = &camerazoom_kind,
            .value_num = 2,
            .value_names = (char *[]){
                "Original",
                "Zoomed Out",
            },
        },
        {
            .name = "Unpause",
            .kind = OPTKIND_VALUE,
            .val = &unpause_delay_enabled,
            .value_num = 2,
            .value_names = (char *[]){
                "Original",
                "Countdown",
            },
        },
        {
            .name = "Attract Mode",
            .kind = OPTKIND_VALUE,
            .val = &attractmode_enabled,
            .value_num = 2,
            .value_names = (char *[]){
                "Off",
                "On",
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
    UnpauseDelay_Init();
    Run_Init();

    return;
}
void On3DLoad()
{
    QuickStat_On3DStart();
    UnpauseDelay_On3DStart();
    Rearview_InitFlags();
}
void On3DPause(int pause_ply)
{
    QuickStat_OnPause();
}