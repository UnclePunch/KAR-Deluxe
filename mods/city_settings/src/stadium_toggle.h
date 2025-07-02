#ifndef CITYSETTING_STADIUMTOGGLE
#define CITYSETTING_STADIUMTOGGLE

#include "obj.h"
#include "text.h"

#define STADTOGGLE_OPTNUM_X (2)
#define STADTOGGLE_OPTNUM_Y (12)

#define STADTOGGLE_OPTION_JOINT_BG (1)
#define STADTOGGLE_OPTION_JOINT_TOGGLE (2)
#define STADTOGGLE_OPTION_JOINT_TEXT (5)

#define STADTOGGLE_BG_JOINT_TOGGLEALL (3)
#define STADTOGGLE_BG_JOINT_NAME (5)
#define STADTOGGLE_BG_JOINT_DESCRIPTION (6)
#define STADTOGGLE_BG_JOINT_OPTIONS (9)

typedef enum StadiumToggleKind
{
    STADTOGGLE_DRAG1,
    STADTOGGLE_DRAG2,
    STADTOGGLE_DRAG3,
    STADTOGGLE_DRAG4,
    STADTOGGLE_DESTRUCTION1,
    STADTOGGLE_DESTRUCTION2,
    STADTOGGLE_DESTRUCTION3,
    STADTOGGLE_DESTRUCTION4,
    STADTOGGLE_DESTRUCTION5,
    STADTOGGLE_AIRGLIDER,
    STADTOGGLE_TARGETFLIGHT,
    STADTOGGLE_HIGHJUMP,
    STADTOGGLE_SINGLERACE1,
    STADTOGGLE_SINGLERACE2,
    STADTOGGLE_SINGLERACE3,
    STADTOGGLE_SINGLERACE4,
    STADTOGGLE_SINGLERACE5,
    STADTOGGLE_SINGLERACE6,
    STADTOGGLE_SINGLERACE7,
    STADTOGGLE_SINGLERACE8,
    STADTOGGLE_SINGLERACE9,
    STADTOGGLE_MELEE1,
    STADTOGGLE_MELEE2,
    STADTOGGLE_VSKINGDEDEDE,
} StadiumToggleKind;

typedef struct StadiumToggleNames
{
    char *name;
    char *desc[2]; // top and bottom lines
} StadiumToggleNames;

typedef struct StadiumToggleData
{
    struct
    {
        u8 x;
        u8 y;
    } cursor;
    struct
    {
        int is_on;
        JOBJ *j;
        Text *text;
        JOBJ *text_j;
        JOBJ *background_j;
        JOBJ *toggle_j;
    } option[STADTOGGLE_OPTNUM_X * STADTOGGLE_OPTNUM_Y];
    struct
    {
        Text *text;
        JOBJ *text_j;
    } name;
    struct
    {
        Text *text;
        JOBJ *text_j;
    } description[2]; // one for each line
} StadiumToggleData;

GOBJ *StadiumToggle_Create(int (**input_cb)(GOBJ *));
int StadiumToggle_Input(GOBJ *g);
void StadiumToggle_Update(GOBJ *g);
void StadiumToggle_Destroy(StadiumToggleData *gp);

#endif