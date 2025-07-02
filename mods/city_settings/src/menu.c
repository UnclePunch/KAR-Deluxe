/*---------------------------------------------------------------------------*
    UI code for the city settings menu.
        - injects its code into memory
        - allocates runtime save struct
        - creates save file on memcard
        - loads save file from memcard
 *---------------------------------------------------------------------------*/

#include "text.h"
#include "useful.h"
#include "hsd.h"
#include "preload.h"
#include "scene.h"
#include "inline.h"
#include "audio.h"

#include "menu.h"
#include "citysettings.h"

static float stc_val_pos[][3] =
    {
        {0, 0, 0},
        {0.2, 0, 0},
        {0.2, 15.5, 0},
        {-2.2, 9.1, 20.0},
};

static CitySettingsMenuAssets menu_assets;

/*---------------------------------------------------------------------*
Name:           Menu_Init

Description:    loads custom assets and creates a lookup of menu assets.

Arguments:      settings_archive        pointer to the MnSelruleAll.dat archive.
*---------------------------------------------------------------------*/
void Menu_Init(HSD_Archive *custom_archive)
{
    menu_assets.option_set = *(JOBJSet **)Archive_GetPublicAddress(custom_archive, "ScMenSelruleFrame_scene_models");
    menu_assets.menu_set = *(JOBJSet **)Archive_GetPublicAddress(custom_archive, "ScMenSelruleFrame2_scene_models");
    menu_assets.value_set_small = *(JOBJSet **)Archive_GetPublicAddress(custom_archive, "ScMenSelruleContents_scene_models");
    menu_assets.value_set_wide = *(JOBJSet **)Archive_GetPublicAddress(custom_archive, "ScMenSelruleStadium_scene_models");
    menu_assets.value_set_num = *(JOBJSet **)Archive_GetPublicAddress(custom_archive, "ScMenSelruleNum_scene_models");
    menu_assets.cursor_set = *(JOBJSet **)Archive_GetPublicAddress(custom_archive, "ScMenSelruleCursor_scene_models");

    // menu_assets.option_set->matanimjoint = (*(JOBJSet **)Archive_GetPublicAddress(custom_archive, "ScMenSelruleFrame_scene_models"))->matanimjoint;
    // menu_assets.menu_set->matanimjoint = (*(JOBJSet **)Archive_GetPublicAddress(custom_archive, "ScMenSelruleFrame2_scene_models"))->matanimjoint;
    // menu_assets.value_set_small->matanimjoint = (*(JOBJSet **)Archive_GetPublicAddress(custom_archive, "ScMenSelruleContents_scene_models"))->matanimjoint;
    // menu_assets.value_set_wide->matanimjoint = (*(JOBJSet **)Archive_GetPublicAddress(custom_archive, "ScMenSelruleStadium_scene_models"))->matanimjoint;
    // menu_assets.value_set_num->matanimjoint = (*(JOBJSet **)Archive_GetPublicAddress(custom_archive, "ScMenSelruleNum_scene_models"))->matanimjoint;
}

/*---------------------------------------------------------------------*
Name:           Menu_Create

Description:    Creates a menu GObj from the provided menu descriptor

Arguments:      desc                    menu descriptor.

Returns:        GOBJ containing the menu.
*---------------------------------------------------------------------*/
GOBJ *Menu_Create(CitySettingsMenuDesc *desc)
{
    GOBJ *gm = GOBJ_EZCreator(37, MENUPLINK_GENERIC, 0,
                              sizeof(CitySettingsMenuData), HSD_Free,
                              HSD_OBJKIND_JOBJ, 0,
                              0, 0,
                              JObj_GX, MENUGX_1, 0);
    CitySettingsMenuData *md = gm->userdata;
    md->desc = desc;
    GObj_AddProc(gm, GOBJ_Anim, 1);

    float max_top = 12;
    float max_bot = -12;
    int opt_num = desc->generic.opt_num;
    float opt_height = 6.5;
    float top = 0 + ((opt_num - 1) * opt_height) / 2;
    if (top > max_top)
    {
        opt_height *= ((float)opt_num / 6) * (max_top / top);
        top = 0 + ((opt_num - 1) * opt_height) / 2;
    }

    // create options
    for (int opt_idx = 0; opt_idx < opt_num; opt_idx++)
    {
        JOBJ *opt_j;

        // create option and its values
        switch (desc->generic.options[opt_idx].kind)
        {
        case (CITYSETTING_OPTKIND_VALUE):
        {

            // create option model
            opt_j = JObj_LoadJoint(menu_assets.option_set->jobj);
            JObj_AddSetAnim(opt_j, 0, menu_assets.option_set, 0, 0);

            // set selected state
            JObj_SetFrameAndRate(JObj_GetIndex(opt_j, 1), 0, 0);

            // set texture
            JObj_SetFrameAndRate(JObj_GetIndex(opt_j, 5), desc->generic.options[opt_idx].texture_frame, 0);

            if (desc->generic.options[opt_idx].u.value.data)
            {
                int val_num = desc->generic.options[opt_idx].u.value.num;

                // create many small values
                if (val_num < 4)
                {
                    // move joints into position according to the amt of values
                    // JObj_SetFrameAndRate(pos_j, val_num, 0);

                    // create each value
                    for (int val_idx = 0; val_idx < val_num; val_idx++)
                    {
                        // create value model, add it to the option model
                        JOBJ *val_j = JObj_LoadJoint(menu_assets.value_set_small->jobj);
                        JObj_AddSetAnim(val_j, 0, menu_assets.value_set_small, 0, 0);
                        JObj_SetFrameAndRate(JObj_GetIndex(val_j, 1), 0, 0);                                                                  // select state
                        JObj_SetFrameAndRate(JObj_GetIndex(val_j, 2), desc->generic.options[opt_idx].u.value.data[val_idx].texture_frame, 0); // option name
                        JObj_AddNext(opt_j, val_j);
                        desc->generic.options[opt_idx].u.value.data[val_idx].j = val_j;

                        // position the value
                        val_j->trans.X = stc_val_pos[val_num][val_idx];
                    }
                }
                // use one wide value
                else
                {
                    // create value model, add it to the option model
                    JOBJ *val_j = JObj_LoadJoint(menu_assets.value_set_wide->jobj);
                    JObj_AddSetAnim(val_j, 0, menu_assets.value_set_wide, 0, 0);
                    JObj_SetFrameAndRate(val_j, 0, 0); // texture name
                    JObj_AddNext(opt_j, val_j);
                    desc->generic.options[opt_idx].u.value.data[0].j = val_j;

                    // position the value
                    val_j->trans.X = 0.2;
                }
            }

            break;
        }
        case (CITYSETTING_OPTKIND_MENU):
        {
            opt_j = JObj_LoadJoint(menu_assets.menu_set->jobj);
            JObj_AddSetAnim(opt_j, 0, menu_assets.menu_set, 0, 0);

            break;
        }
        case (CITYSETTING_OPTKIND_NUM):
        {
            // create option model
            opt_j = JObj_LoadJoint(menu_assets.option_set->jobj);
            JObj_AddSetAnim(opt_j, 0, menu_assets.option_set, 0, 0);
            JObj_SetFrameAndRate(JObj_GetIndex(opt_j, 1), 0, 0);                                            // set selected state
            JObj_SetFrameAndRate(JObj_GetIndex(opt_j, 5), desc->generic.options[opt_idx].texture_frame, 0); // set texture

            // create value model
            JOBJ *val_j = JObj_LoadJoint(menu_assets.value_set_num->jobj);
            JObj_AddSetAnim(val_j, 0, menu_assets.value_set_num, 0, 0);
            JObj_AddNext(opt_j, val_j);
            JObj_GetIndex(val_j, 1)->dobj->mobj->mat->alpha = 0; // hide recommended text
            JObj_SetFrameAndRate(val_j, 1, 0);                   // set anim frame
            desc->generic.options[opt_idx].u.number.j = val_j;

            // position the value
            val_j->trans.X = 0.2;

            break;
        }
        }

        // store jobj to lookup
        desc->generic.options[opt_idx].j = opt_j;

        JObj_AddNext(gm->hsd_object, opt_j);
        opt_j->trans.Y = top - (opt_idx * opt_height);
    }

    // add cursor
    JOBJ *cj = JObj_LoadJoint(menu_assets.cursor_set->jobj);
    JObj_AddSetAnim(cj, 0, menu_assets.cursor_set, 0, 1);
    desc->generic.cursor_j = cj;
    JObj_AddNext(gm->hsd_object, cj);

    Menu_Update(gm);

    return gm;
}

/*---------------------------------------------------------------------*
Name:           Menu_Update

Description:    Updates the UI state of the menu passed to match its
                current values.

Arguments:      g                    GOBJ containing the menu.
*---------------------------------------------------------------------*/
void Menu_Update(GOBJ *g)
{

    // ensure this is a generic menu gobj
    if (g->p_link != MENUPLINK_GENERIC)
        return;

    CitySettingsMenuData *md = g->userdata;
    CitySettingsMenuDesc *desc = md->desc;

    // option update
    for (int opt_idx = 0; opt_idx < desc->generic.opt_num; opt_idx++)
    {
        JOBJ *opt_j = desc->generic.options[opt_idx].j;

        switch (desc->generic.options[opt_idx].kind)
        {
        case (CITYSETTING_OPTKIND_VALUE):
        {
            int sel_val_idx = desc->generic.options[opt_idx].u.value.cur_selection;

            // if value data exists
            if (desc->generic.options[opt_idx].u.value.data)
            {
                int val_num = desc->generic.options[opt_idx].u.value.num;
                JOBJ *val_first_j = desc->generic.options[opt_idx].u.value.data[0].j;

                // highlight option model
                if (desc->generic.cursor_val == opt_idx)
                    JObj_SetFrameAndRate(JObj_GetIndex(opt_j, 1), 1, 0); // set option as highlighted and expanded
                else
                    JObj_SetFrameAndRate(JObj_GetIndex(opt_j, 1), 0, 0);

                // 3 or less
                if (val_num < 4)
                {
                    if (desc->generic.cursor_val == opt_idx) // show all options
                    {
                        // for each value
                        for (int val_idx = 0; val_idx < val_num; val_idx++)
                        {
                            // show value model
                            JOBJ *val_j = desc->generic.options[opt_idx].u.value.data[val_idx].j;
                            JObj_ClearFlagsAll(val_j, JOBJ_HIDDEN);

                            // position the value
                            val_j->trans.X = stc_val_pos[val_num][val_idx];

                            // update the texture
                            JObj_SetFrameAndRate(JObj_GetIndex(val_j, 2),
                                                 desc->generic.options[opt_idx].u.value.data[val_idx].texture_frame,
                                                 0);

                            // check to highlight
                            if (val_idx == sel_val_idx)
                            {
                                JObj_SetFrameAndRate(JObj_GetIndex(val_j, 1), 1, 0); // selected state

                                // cursor update
                                Menu_CursorUpdate(desc->generic.cursor_j, val_j, 0);

                                // update description
                                CitySettings_UpdateDescription(desc->generic.options[opt_idx].u.value.data[val_idx].description);
                            }
                            else
                                JObj_SetFrameAndRate(JObj_GetIndex(val_j, 1), 0, 0); // de-selected state
                        }
                    }
                    else // hide all but the first
                    {
                        // for each value
                        for (int val_idx = 0; val_idx < val_num; val_idx++)
                        {
                            // hide value model
                            JOBJ *val_j = desc->generic.options[opt_idx].u.value.data[val_idx].j;
                            JObj_SetFlagsAll(val_j, JOBJ_HIDDEN);
                        }

                        // show first value
                        JObj_ClearFlagsAll(val_first_j, JOBJ_HIDDEN);                                                                                   // show main value
                        JObj_SetFrameAndRate(JObj_GetIndex(val_first_j, 1), 1, 0);                                                                      // set as selected
                        JObj_SetFrameAndRate(JObj_GetIndex(val_first_j, 2), desc->generic.options[opt_idx].u.value.data[sel_val_idx].texture_frame, 0); // update texture

                        // position it
                        val_first_j->trans.X = 0.2;
                    }
                }
                // 4 or more
                else
                {
                    // show first value and update texture
                    JObj_ClearFlagsAll(val_first_j, JOBJ_HIDDEN);                                                                                   // show main value
                    JObj_SetFrameAndRate(JObj_GetIndex(val_first_j, 1), desc->generic.options[opt_idx].u.value.data[sel_val_idx].texture_frame, 0); // update texture

                    if (desc->generic.cursor_val == opt_idx) // update description and move cursor
                    {
                        // update description
                        CitySettings_UpdateDescription(desc->generic.options[opt_idx].u.value.data[sel_val_idx].description);

                        // cursor update
                        Menu_CursorUpdate(desc->generic.cursor_j, val_first_j, 1);
                    }
                }
            }

            break;
        }
        case (CITYSETTING_OPTKIND_MENU):
        {
            // check if selected
            if (desc->generic.cursor_val == opt_idx)
            {
                JObj_AddSetAnim(opt_j, 1, menu_assets.menu_set, 0, 1);
                JObj_SetFlagsAll(desc->generic.cursor_j, JOBJ_HIDDEN); // hide cursor

                // update description
                CitySettings_UpdateDescription(desc->generic.options[opt_idx].u.menu.description);
            }
            else
                JObj_AddSetAnim(opt_j, 0, menu_assets.menu_set, 0, 1);

            // set texture name
            JObj_ReqAnim(opt_j, desc->generic.options[opt_idx].texture_frame);
            JObj_GetIndex(opt_j, 4)->dobj->mobj->tobj->aobj->framerate = 0.0f;

            break;
        }
        case (CITYSETTING_OPTKIND_NUM):
        {
            JOBJ *val_j = desc->generic.options[opt_idx].u.number.j;

            if (desc->generic.cursor_val == opt_idx)
            {
                // set option as highlighted and expanded
                JObj_SetFrameAndRate(JObj_GetIndex(opt_j, 1), 1, 0);

                // cursor update
                Menu_CursorUpdate(desc->generic.cursor_j, val_j, 0);

                // update description
                CitySettings_UpdateDescription(desc->generic.options[opt_idx].u.number.description);
            }
            else
            {
                // set option as de-selected
                JObj_SetFrameAndRate(JObj_GetIndex(opt_j, 1), 0, 0);
            }

            // update digits
            int val = desc->generic.options[opt_idx].u.number.cur_selection;
            JObj_SetFrameAndRate(JObj_GetIndex(val_j, 2), val % 10, 0);
            JObj_SetFrameAndRate(JObj_GetIndex(val_j, 3), (val / 10) % 10, 0);

            // update unit texture
            JObj_SetFrameAndRate(JObj_GetIndex(val_j, 4), desc->generic.options[opt_idx].u.number.texture_frame, 0);

            break;
        }
        }
    }
}

/*---------------------------------------------------------------------*
Name:           Menu_Input

Description:    Processes inputs for this menu.

Arguments:      g                     GOBJ containing the menu.

Returns:        action_kind           Value that describes what occured this tick.
*---------------------------------------------------------------------*/
CitySettingsMenuAction Menu_Input(GOBJ *g)
{
    CitySettingsMenuData *md = g->userdata;
    CitySettingsMenuDesc *desc = md->desc;

    int down = Pad_GetDown(20);
    int rapid_held = Pad_GetRapidHeld(20);
    CitySettingsMenuAction action_kind = CITYSETTING_MENUACT_NONE;

    // determine action from input
    if (rapid_held & PAD_BUTTON_DOWN)
    {
        if (desc->generic.cursor_val < (desc->generic.opt_num - 1))
        {
            desc->generic.cursor_val++;
            action_kind = CITYSETTING_MENUACT_CHANGE;
        }
    }
    else if (rapid_held & PAD_BUTTON_UP)
    {
        if (desc->generic.cursor_val > 0)
        {
            desc->generic.cursor_val--;
            action_kind = CITYSETTING_MENUACT_CHANGE;
        }
    }
    else if (rapid_held & PAD_BUTTON_RIGHT)
    {
        switch (desc->generic.options[desc->generic.cursor_val].kind)
        {
        case (CITYSETTING_OPTKIND_VALUE):
        {
            if (desc->generic.options[desc->generic.cursor_val].u.value.cur_selection < desc->generic.options[desc->generic.cursor_val].u.value.num - 1)
            {
                desc->generic.options[desc->generic.cursor_val].u.value.cur_selection++;
                action_kind = CITYSETTING_MENUACT_CHANGE;
            }

            break;
        }
        case (CITYSETTING_OPTKIND_MENU):
        {
            break;
        }
        case (CITYSETTING_OPTKIND_NUM):
        {
            if (desc->generic.options[desc->generic.cursor_val].u.number.cur_selection < desc->generic.options[desc->generic.cursor_val].u.number.max)
                desc->generic.options[desc->generic.cursor_val].u.number.cur_selection++;
            else
                desc->generic.options[desc->generic.cursor_val].u.number.cur_selection = desc->generic.options[desc->generic.cursor_val].u.number.min;

            action_kind = CITYSETTING_MENUACT_CHANGE;
            break;
        }
        }
    }
    else if (rapid_held & PAD_BUTTON_LEFT)
    {
        switch (desc->generic.options[desc->generic.cursor_val].kind)
        {
        case (CITYSETTING_OPTKIND_VALUE):
        {
            if (desc->generic.options[desc->generic.cursor_val].u.value.cur_selection > 0)
            {
                desc->generic.options[desc->generic.cursor_val].u.value.cur_selection--;
                action_kind = CITYSETTING_MENUACT_CHANGE;
            }

            break;
        }
        case (CITYSETTING_OPTKIND_MENU):
        {
            break;
        }
        case (CITYSETTING_OPTKIND_NUM):
        {
            if (desc->generic.options[desc->generic.cursor_val].u.number.cur_selection > desc->generic.options[desc->generic.cursor_val].u.number.min)
                desc->generic.options[desc->generic.cursor_val].u.number.cur_selection--;
            else
                desc->generic.options[desc->generic.cursor_val].u.number.cur_selection = desc->generic.options[desc->generic.cursor_val].u.number.max;

            action_kind = CITYSETTING_MENUACT_CHANGE;
            break;
        }
        }
    }
    else if (down & PAD_BUTTON_A)
    {
        switch (desc->generic.options[desc->generic.cursor_val].kind)
        {
        case (CITYSETTING_OPTKIND_VALUE):
        {
            break;
        }
        case (CITYSETTING_OPTKIND_MENU):
        {
            action_kind = CITYSETTING_MENUACT_ADVANCE;
            break;
        }
        case (CITYSETTING_OPTKIND_NUM):
        {
            break;
        }
        }
    }
    else if (down & PAD_BUTTON_START)
    {
        action_kind = CITYSETTING_MENUACT_EXIT;
    }
    else if (down & PAD_BUTTON_B)
    {
        action_kind = CITYSETTING_MENUACT_REGRESS;
    }

    // act on action
    if (action_kind != CITYSETTING_MENUACT_NONE)
    {
        Menu_Update(g);

        if (action_kind == CITYSETTING_MENUACT_CHANGE)
            SFX_Play(FGMMENU_CS_MV);

        else if (action_kind == CITYSETTING_MENUACT_ADVANCE || action_kind == CITYSETTING_MENUACT_EXIT)
            SFX_Play(FGMMENU_CS_KETTEI);

        else if (action_kind == CITYSETTING_MENUACT_REGRESS)
            SFX_Play(FGMMENU_CS_CANCEL);
    }

    return action_kind;
}

/*---------------------------------------------------------------------*
Name:           Menu_CursorUpdate

Description:    Updates the cursor's position and width to conform
                to the value being focused.

Arguments:      cursor_j                    cursor's jobj.
                val_j                       focused value's jobj.
                is_wide                     bool indicating if the cursor should be widened.
*---------------------------------------------------------------------*/
void Menu_CursorUpdate(JOBJ *cursor_j, JOBJ *val_j, int is_wide)
{
    JObj_ClearFlagsAll(cursor_j, JOBJ_HIDDEN);                                                  // show cursor
    JObj_GetWorldPosition(val_j, 0, &cursor_j->trans);                                          // move cursor to selected value
    JObj_AddSetAnim(cursor_j, is_wide, menu_assets.cursor_set, JObj_GetAOBJFrame(cursor_j), 1); // change cursor size
}
