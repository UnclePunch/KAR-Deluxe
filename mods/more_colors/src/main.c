#include "hoshi/mod.h"

#include "colors.h"

// Callbacks
void OnBoot()
{
    Colors_Init(COLORDATA_FILENAME);

    return;
}
void OnSceneChange()
{
    return;
}

ModDesc mod_desc = {
    .name = "More Colors",
    .author = "UnclePunch",
    .version.major = 1,
    .version.minor = 1,
    .OnBoot = OnBoot,
    .OnSceneChange = OnSceneChange,
};