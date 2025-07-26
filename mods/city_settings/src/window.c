#include "text.h"
#include "os.h"
#include "hsd.h"
#include "preload.h"
#include "scene.h"
#include "inline.h"
#include "audio.h"

#include "window.h"
#include "citysettings.h"
#include "text_joint/text_joint.h"

static CitySettingsWindowAssets window_assets;

/*---------------------------------------------------------------------*
Name:           Window_Init

Description:    loads custom assets and creates a lookup of assets.

Arguments:      settings_archive        pointer to the MnSelruleCustom.dat archive.
*---------------------------------------------------------------------*/
void Window_Init(HSD_Archive *custom_archive)
{
    window_assets.window_set = *(JOBJSet **)Archive_GetPublicAddress(custom_archive, "ScMenOpdelwin_scene_models");
}

/*---------------------------------------------------------------------*
Name:           Window_Create

Description:    Creates a window gobj

Arguments:      desc                    window descriptor.

Returns:        GOBJ containing the window.
*---------------------------------------------------------------------*/
GOBJ *Window_Create(char *string, void (*cb_confirm)())
{
    GOBJ *gw = GOBJ_EZCreator(37, MENUPLINK_WINDOW, 0,
                              sizeof(CitySettingsWindowData), Window_Destroy,
                              HSD_OBJKIND_JOBJ, window_assets.window_set[0].jobj,
                              0, 0,
                              JObj_GX, MENUGX_1, 0);
    CitySettingsWindowData *wd = gw->userdata;
    JOBJ *wj = gw->hsd_object;

    JObj_AddSetAnim(wj, 0, window_assets.window_set, 0, 0);

    wd->cursor = 0;
    wd->text = TextJoint_Create(wj, 2,
                                stc_scene_menu_common->canvas_idx,
                                string,
                                1,
                                wj->scale.X * 700, 24,
                                &(GXColor){255, 255, 255, 255});
    wd->cb_confirm = cb_confirm;

    return gw;
}

/*---------------------------------------------------------------------*
Name:           Window_Update

Description:    Updates the UI state of the window passed to match its
                current values.

Arguments:      g                    GOBJ containing the window.
*---------------------------------------------------------------------*/
void Window_Update(GOBJ *g)
{
    CitySettingsWindowData *gp = g->userdata;
    JObj_SetFrameAndRate(g->hsd_object, gp->cursor, 0);
}

/*---------------------------------------------------------------------*
Name:           Window_Input

Description:    Processes inputs for this window.

Arguments:      desc                  CitySettingsMenuDesc describing the window.

Returns:        action_kind           Value that describes what occured this tick.
*---------------------------------------------------------------------*/
CitySettingsMenuAction Window_Input(GOBJ *g)
{
    CitySettingsWindowData *gp = g->userdata;

    int down = Pad_GetDown(20);
    int rapid_held = Pad_GetRapidHeld(20);
    CitySettingsMenuAction action_kind = CITYSETTING_MENUACT_NONE;

    // determine action from input
    if (rapid_held & (PAD_BUTTON_RIGHT | PAD_BUTTON_DPAD_RIGHT))
    {
        gp->cursor++;
        if (gp->cursor > 1)
            gp->cursor = 0;

        action_kind = CITYSETTING_MENUACT_CHANGE;
    }
    else if (rapid_held & (PAD_BUTTON_LEFT | PAD_BUTTON_DPAD_LEFT))
    {
        gp->cursor--;
        if (gp->cursor < 0)
            gp->cursor = 1;

        action_kind = CITYSETTING_MENUACT_CHANGE;
    }
    else if (down & (PAD_BUTTON_A | PAD_BUTTON_START))
    {
        // exec callback when selecting yes
        if (gp->cursor == 0)
        {
            gp->cb_confirm();
            action_kind = CITYSETTING_MENUACT_ADVANCE;
        }
        else if (gp->cursor == 1)
            action_kind = CITYSETTING_MENUACT_REGRESS;
    }
    else if (down & PAD_BUTTON_B)
    {
        action_kind = CITYSETTING_MENUACT_REGRESS;
    }

    switch (action_kind)
    {
    case (CITYSETTING_MENUACT_CHANGE):
    {
        SFX_Play(FGMMENU_CS_MV);
        break;
    }
    case (CITYSETTING_MENUACT_ADVANCE):
    case (CITYSETTING_MENUACT_EXIT):
    {
        SFX_Play(FGMMENU_CS_KETTEI);
        break;
    }
    case (CITYSETTING_MENUACT_REGRESS):
    {
        SFX_Play(FGMMENU_CS_CANCEL);
        break;
    }
    }

    return action_kind;
}

/*---------------------------------------------------------------------*
Name:           Window_Destroy

Description:    Handles cleaning up the window's text object.

Arguments:      gp                    CitySettingsWindowData
*---------------------------------------------------------------------*/
void Window_Destroy(CitySettingsWindowData *gp)
{
    TextJoint_Destroy(gp->text);
    HSD_Free(gp);
}