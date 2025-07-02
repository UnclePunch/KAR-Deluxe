#ifndef CITYSETTING_MACHINETOGGLE
#define CITYSETTING_MACHINETOGGLE

#include "obj.h"
#include "text.h"

#define MCHNTOGGLE_OPTNUM_X (2)
#define MCHNTOGGLE_OPTNUM_Y (10)

#define MCHNTOGGLE_OPTION_JOINT_BG (1)
#define MCHNTOGGLE_OPTION_JOINT_TOGGLE (2)
#define MCHNTOGGLE_OPTION_JOINT_ICON (5)
#define MCHNTOGGLE_OPTION_JOINT_TEXT (6)

#define MCHNTOGGLE_BG_JOINT_TOGGLEALL (1)
#define MCHNTOGGLE_BG_JOINT_OPTIONS (3)

typedef struct MachineToggleNames
{
    char *name;
    char *desc[2]; // top and bottom lines
} MachineToggleNames;

typedef struct MachineToggleData
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
    } option[MCHNTOGGLE_OPTNUM_Y * MCHNTOGGLE_OPTNUM_X];
    struct
    {
        Text *text;
        JOBJ *text_j;
    } description[2]; // one for each line
} MachineToggleData;

GOBJ *MachineToggle_Create(int (**input_cb)(GOBJ *));
int MachineToggle_Input(GOBJ *g);
void MachineToggle_Update(GOBJ *g);
void MachineToggle_Destroy(MachineToggleData *gp);

#endif