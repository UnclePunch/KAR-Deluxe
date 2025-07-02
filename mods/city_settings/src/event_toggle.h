#ifndef CITYSETTING_EVENTTOGGLE
#define CITYSETTING_EVENTTOGGLE

#include "obj.h"
#include "text.h"

#define EVENTTOGGLE_OPTNUM_X (2)
#define EVENTTOGGLE_OPTNUM_Y (8)

#define EVENTTOGGLE_OPTION_JOINT_BG (1)
#define EVENTTOGGLE_OPTION_JOINT_TOGGLE (2)
#define EVENTTOGGLE_OPTION_JOINT_THUMBNAIL (5)
#define EVENTTOGGLE_OPTION_JOINT_TEXT (6)

#define EVENTTOGGLE_BG_JOINT_TOGGLEALL (3)
#define EVENTTOGGLE_BG_JOINT_NAME (5)
#define EVENTTOGGLE_BG_JOINT_DESCRIPTION (6)
#define EVENTTOGGLE_BG_JOINT_OPTIONS (9)

typedef struct EventToggleNames
{
    char *name;
    char *desc[2]; // top and bottom lines
} EventToggleNames;

typedef struct EventToggleData
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
    } option[EVENTTOGGLE_OPTNUM_X * EVENTTOGGLE_OPTNUM_Y];
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
} EventToggleData;

GOBJ *EventToggle_Create(int (**input_cb)(GOBJ *));
int EventToggle_Input(GOBJ *g);
void EventToggle_Update(GOBJ *g);
void EventToggle_Destroy(EventToggleData *gp);

#endif