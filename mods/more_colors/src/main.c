#include "os.h"
#include "hsd.h"


#include "colors.h"

char ModName[] = "More Colors";
char ModAuthor[] = "UnclePunch";
char ModVersion[] = "v1.1";

// Callbacks
void OnBoot(HSD_Archive *archive)
{
    Colors_Init(COLORDATA_FILENAME);

    return;
}
void OnSceneChange(HSD_Archive *archive)
{
    return;
}
