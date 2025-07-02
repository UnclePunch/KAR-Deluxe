#ifndef CITYSETTING_ITEMTOGGLE
#define CITYSETTING_ITEMTOGGLE

#include "obj.h"
#include "text.h"

#define ITEMTOGGLE_OPTNUM_X (8)
#define ITEMTOGGLE_OPTNUM_Y (7)
#define ITEMTOGGLE_OPTNUM (52)

#define ITEMTOGGLE_OPTION_JOINT_BG (1)
#define ITEMTOGGLE_OPTION_JOINT_ICON (2)
#define ITEMTOGGLE_OPTION_JOINT_TOGGLE (3)

#define ITEMTOGGLE_BG_JOINT_ITEM (2)
#define ITEMTOGGLE_BG_JOINT_TOGGLEALL (3)
#define ITEMTOGGLE_BG_JOINT_NAME (5)
#define ITEMTOGGLE_BG_JOINT_DESCRIPTION (6)
#define ITEMTOGGLE_BG_JOINT_OPTIONS (9)

typedef enum ItemToggleKind
{
    ITEMTOGGLE_DRAG1,
    ITEMTOGGLE_DRAG2,
    ITEMTOGGLE_DRAG3,
    ITEMTOGGLE_DRAG4,
    ITEMTOGGLE_DESTRUCTION1,
    ITEMTOGGLE_DESTRUCTION2,
    ITEMTOGGLE_DESTRUCTION3,
    ITEMTOGGLE_DESTRUCTION4,
    ITEMTOGGLE_DESTRUCTION5,
    ITEMTOGGLE_AIRGLIDER,
    ITEMTOGGLE_TARGETFLIGHT,
    ITEMTOGGLE_HIGHJUMP,
    ITEMTOGGLE_SINGLERACE1,
    ITEMTOGGLE_SINGLERACE2,
    ITEMTOGGLE_SINGLERACE3,
    ITEMTOGGLE_SINGLERACE4,
    ITEMTOGGLE_SINGLERACE5,
    ITEMTOGGLE_SINGLERACE6,
    ITEMTOGGLE_SINGLERACE7,
    ITEMTOGGLE_SINGLERACE8,
    ITEMTOGGLE_SINGLERACE9,
    ITEMTOGGLE_MELEE1,
    ITEMTOGGLE_MELEE2,
    ITEMTOGGLE_VSKINGDEDEDE,
} ItemToggleKind;

typedef struct ItemToggleNames
{
    char *name;
    char *desc[2]; // top and bottom lines
} ItemToggleNames;

typedef struct ItemToggleData
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
        JOBJ *icon_j;
    } option[ITEMTOGGLE_OPTNUM];
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
} ItemToggleData;

GOBJ *ItemToggle_Create(int (**input_cb)(GOBJ *));
int ItemToggle_Input(GOBJ *g);
void ItemToggle_Update(GOBJ *g);
void ItemToggle_Destroy(ItemToggleData *gp);

#endif