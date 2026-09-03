#include "hoshi/mod.h"
#include "hoshi/settings.h"

#include "motion.h"

#define VERSION_MAJOR 1
#define VERSION_MINOR 0

extern int tilt_disabled;
extern int fovspeed_disabled;
extern int shake_disabled;
extern int fov_level;

OptionDesc mod_settings = {
    // Controls Menu
    .name = "Motion Settings",
    .description = "Reduce camera motion.",
    .pri = MENUPRI_HIGH,
    .kind = OPTKIND_MENU,
    .menu_ptr = &(MenuDesc){
        .option_num = 4,
        .options =
            {
                &(OptionDesc){
                    .name = "Camera Tilt",
                    .description = "Steering tilts the camera.",
                    .kind = OPTKIND_VALUE,
                    .val = &tilt_disabled,
                    .value_num = 2,
                    .value_names = (char *[]){
                        "On",
                        "Off",
                    },
                },
                &(OptionDesc){
                    .name = "Camera Shake",
                    .description = "Camera shakes during gameplay.",
                    .kind = OPTKIND_VALUE,
                    .val = &shake_disabled,
                    .value_num = 2,
                    .value_names = (char *[]){
                        "On",
                        "Off",
                    },
                },
                &(OptionDesc){
                    .name = "Speed Effects",
                    .description = "Field of view changes during gameplay.",
                    .kind = OPTKIND_VALUE,
                    .val = &fovspeed_disabled,
                    .value_num = 2,
                    .value_names = (char *[]){
                        "On",
                        "Off",
                    },
                },
                &(OptionDesc){
                    .name = "Field of View",
                    .description = "Adjust the width of your view.",
                    .kind = OPTKIND_VALUE,
                    .val = &fov_level,
                    .value_num = 3,
                    .value_names = (char *[]){
                        "Low",
                        "Normal",
                        "High",
                    },
                },
            },
    },
};

void OnBoot()
{
    return;
}
void OnSceneChange()
{

    return;
}
void OnSaveLoaded()
{
    Motion_Init();

    return;
}
void On3DLoad()
{
}
void On3DPause(int pause_ply)
{
}

ModDesc mod_desc = {
    .name = "Motion Settings",
    .author = "UnclePunch",
    .version.major = VERSION_MAJOR,
    .version.minor = VERSION_MINOR,
    .affects_gameplay = true,
    .option_desc = &mod_settings,
    .OnBoot = OnBoot,
    .OnSceneChange = OnSceneChange,
    .OnSaveLoaded = OnSaveLoaded,
    .On3DLoadEnd = On3DLoad,
    .On3DPause = On3DPause,
};