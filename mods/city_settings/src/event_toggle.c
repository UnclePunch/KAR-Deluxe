/*---------------------------------------------------------------------------*
    UI code for the city settings event toggle menu.
 *---------------------------------------------------------------------------*/

#include "text.h"
#include "os.h"
#include "hsd.h"
#include "inline.h"
#include "game.h"
#include "audio.h"

#include "citysettings.h" // needed for the custom archive ptr and p_link enum
#include "event_toggle.h"

#include "text_joint/text_joint.h"

static EventToggleNames stc_event_names[] = {
    {
        "Dyna Blade",
        "The mystery bird Dyna Blade",
        "has appeared! Aim for her head!",
    },
    {
        "Tac",
        "Tac stole items and fled the scene!",
        "He's hiding somewhere!",
    },
    {
        "Meteors",
        "DANGER! DANGER! Huge",
        "meteors are incoming!",
    },
    {
        "Huge Pillar",
        "A huge, unidentified pillar",
        "appeared! Bust it!",
    },
    {
        "Rowdy Charge",
        "All Air Ride machine energy",
        "tanks have run amok!",
    },
    {
        "Restoration Area",
        "A restoration area has",
        "appeared somewhere in the city!",
    },
    {
        "Station Fire",
        "The rail stations are all",
        "burning out of control!",
    },
    {
        "Same Item",
        "No fair! The boxes",
        "all contain the same items!",
    },
    {
        "Lighthouse",
        "The city lighthouse",
        "has turned on!",
    },
    {
        "Secret Chamber",
        "The secret chamber in Castle ",
        "Hall is open! Get some items!",
    },
    {
        "Stadium Prediction",
        "A glimpse into the future!",
        "Could it be true?",
    },
    {
        "Machine Formation",
        "Air Ride machine formation ",
        "approaching!",
    },
    {
        "UFO",
        "A mysterious flying machine ",
        "is approaching!",
    },
    {
        "Item Bounce",
        "The items are getting rubbery!",
        "They're bouncing!",
    },
    {
        "Fog",
        "A dense fog has",
        "covered the city!",
    },
    {
        "Fake Power-Ups",
        "Some power-up items",
        "are fakes. Be careful!",
    },
};

GOBJ *EventToggle_Create(int (**input_cb)(GOBJ *))
{
    // get menu assets
    HSD_Archive *custom_archive = CitySettings_GetCustomMenuArchive();
    JOBJSet **set = Archive_GetPublicAddress(custom_archive, "ScMenToggleStadium_scene_models");

    // get save data
    CitySettingsSave *cs = CitySettings_SaveGet();

    // create menu gobj
    GOBJ *g = JObj_LoadSet_SetPri(0, set[4], 0, 0, MENUPLINK_CUSTOM, MENUGX_1, 1, EventToggle_Update, 0);

    // alloc data
    EventToggleData *gp = HSD_MemAlloc(sizeof(EventToggleData));
    GObj_AddUserData(g, 14, EventToggle_Destroy, gp);

    // get jobj to attach options to
    JOBJ *attach_to = JObj_GetIndex(g->hsd_object, EVENTTOGGLE_BG_JOINT_OPTIONS);

    // create each option
    int opt_idx = 0;
    for (int col_idx = 0; col_idx < EVENTTOGGLE_OPTNUM_X; col_idx++)
    {
        for (int row_idx = 0; row_idx < EVENTTOGGLE_OPTNUM_Y; row_idx++)
        {
            // create jobj
            JOBJ *opt_j = JObj_LoadJoint(set[5]->jobj);                                           // create jobj
            JObj_AddSetAnim(opt_j, 0, set[5], 0, 0);                                              // add anims
            JObj_SetFrameAndRate(JObj_GetIndex(opt_j, 2), 1, 0);                                  // set on/off state frame
            JObj_AddNext(attach_to, opt_j);                                                       // add to menu gobj
            gp->option[opt_idx].j = opt_j;                                                        // store ptr to lookup
            gp->option[opt_idx].text_j = JObj_GetIndex(opt_j, EVENTTOGGLE_OPTION_JOINT_TEXT);     // store ptr to lookup
            gp->option[opt_idx].background_j = JObj_GetIndex(opt_j, EVENTTOGGLE_OPTION_JOINT_BG); // store ptr to lookup
            gp->option[opt_idx].toggle_j = JObj_GetIndex(opt_j, EVENTTOGGLE_OPTION_JOINT_TOGGLE); // store ptr to lookup

            // init enabled state
            gp->option[opt_idx].is_on = (cs->random_event_bitfield & (1 << opt_idx)) ? 1 : 0;

            // init icon frame
            JObj_SetFrameAndRate(JObj_GetIndex(opt_j, EVENTTOGGLE_OPTION_JOINT_THUMBNAIL), opt_idx, 0);

            // create text
            Text *text = Text_CreateText(0, stc_scene_menu_common->canvas_idx);
            text->kerning = 1;
            text->align = 0;
            text->use_aspect = 1;

            text->color = (GXColor){0, 0, 0, 255};
            text->aspect = (Vec2){280, 32};
            text->viewport_scale = (Vec2){0.043 * 0.95, 0.043 * 0.95};
            Text_AddSubtext(text, 0, -15, stc_event_names[opt_idx].name);
            gp->option[opt_idx].text = text;

            opt_idx++;

            // position the value
            opt_j->trans.X = -(55 / 2) + (col_idx * 55);
            opt_j->trans.Y = ((EVENTTOGGLE_OPTNUM_Y / 2) * 6.6) - (row_idx * 6.6);
        }
    }
    JObj_SetMtxDirtySub(g->hsd_object);

    Text *text;

    // create name text
    text = Text_CreateText(0, stc_scene_menu_common->canvas_idx);
    text->kerning = 1;
    text->align = 0;
    text->use_aspect = 1;

    text->color = (GXColor){255, 255, 255, 255};
    text->aspect = (Vec2){385, 32};
    Text_AddSubtext(text, 0, -15, "");
    gp->name.text = text;
    gp->name.text_j = JObj_GetIndex(g->hsd_object, EVENTTOGGLE_BG_JOINT_NAME);

    // create description text
    for (int i = 0; i < GetElementsIn(gp->description); i++)
    {
        text = Text_CreateText(0, stc_scene_menu_common->canvas_idx);
        text->kerning = 1;
        text->align = 0;
        text->use_aspect = 1;

        text->color = (GXColor){0, 0, 0, 255};
        // text->trans = (Vec3){-21, -0.4 + 17.1, 0};
        text->aspect = (Vec2){600, 32};
        text->viewport_scale = (Vec2){0.043 * 1.2, 0.05 * 1.2};
        // text->hidden = 1;
        Text_AddSubtext(text, 0, -15, "");

        gp->description[i].text = text;
        gp->description[i].text_j = JObj_GetIndex(g->hsd_object, i + EVENTTOGGLE_BG_JOINT_DESCRIPTION);
    }

    // init cursor
    gp->cursor.x = 0;
    gp->cursor.y = 0;

    // request hide background panel
    CitySettings_HideDescription();

    // store input update callback
    *input_cb = EventToggle_Input;

    return g;
}

void EventToggle_Destroy(EventToggleData *gp)
{
    CitySettingsSave *cs = CitySettings_SaveGet();

    for (int i = 0; i < GetElementsIn(gp->option); i++)
    {
        Text_Destroy(gp->option[i].text);

        // write to save data struct
        if (gp->option[i].is_on)
            cs->random_event_bitfield |= (1 << i);
        else
            cs->random_event_bitfield &= ~(1 << i);
    }

    // destroy text
    Text_Destroy(gp->name.text);

    for (int i = 0; i < GetElementsIn(gp->description); i++)
        Text_Destroy(gp->description[i].text);

    HSD_Free(gp);

    return;
}
int EventToggle_Input(GOBJ *g)
{
    EventToggleData *gp = g->userdata;
    int down = Pad_GetDown(20);
    int rapid_held = Pad_GetRapidHeld(20);

    if (rapid_held & (PAD_BUTTON_UP | PAD_BUTTON_DPAD_UP))
    {
        if (gp->cursor.y > 0)
            gp->cursor.y--;
        else
            gp->cursor.y = EVENTTOGGLE_OPTNUM_Y - 1;

        SFX_Play(FGMMENU_CS_MV);
    }
    else if (rapid_held & (PAD_BUTTON_DOWN | PAD_BUTTON_DPAD_DOWN))
    {
        if (gp->cursor.y < EVENTTOGGLE_OPTNUM_Y - 1)
            gp->cursor.y++;
        else
            gp->cursor.y = 0;

        SFX_Play(FGMMENU_CS_MV);
    }
    else if (rapid_held & (PAD_BUTTON_LEFT | PAD_BUTTON_DPAD_LEFT))
    {
        if (gp->cursor.x > 0)
            gp->cursor.x--;
        else
            gp->cursor.x = EVENTTOGGLE_OPTNUM_X - 1;

        SFX_Play(FGMMENU_CS_MV);
    }
    else if (rapid_held & (PAD_BUTTON_RIGHT | PAD_BUTTON_DPAD_RIGHT))
    {
        if (gp->cursor.x < EVENTTOGGLE_OPTNUM_X - 1)
            gp->cursor.x++;
        else
            gp->cursor.x = 0;

        SFX_Play(FGMMENU_CS_MV);
    }
    else if (down & PAD_TRIGGER_Z)
    {
        // ensure at least 1 is enabled
        int is_one_disabled = 0;
        for (int i = 0; i < GetElementsIn(gp->option); i++)
        {
            if (!gp->option[i].is_on)
            {
                is_one_disabled = 1;
                break;
            }
        }

        if (is_one_disabled) // enable all
        {
            for (int i = 0; i < GetElementsIn(gp->option); i++)
                gp->option[i].is_on = 1;
        }
        else // disable all
        {
            for (int i = 0; i < GetElementsIn(gp->option); i++)
                gp->option[i].is_on = 0;
        }

        SFX_Play(FGMMENU_CS_MV);
    }
    else if (down & PAD_BUTTON_A)
    {
        int sel_idx = (gp->cursor.x * EVENTTOGGLE_OPTNUM_Y) + gp->cursor.y;
        gp->option[sel_idx].is_on ^= 1;

        SFX_Play(FGMMENU_CS_MV);
    }
    else if (down & PAD_BUTTON_B)
    {
        // ensure at least 1 is enabled
        int is_one_enabled = 0;
        for (int i = 0; i < GetElementsIn(gp->option); i++)
        {
            if (gp->option[i].is_on)
            {
                is_one_enabled = 1;
                break;
            }
        }

        if (is_one_enabled)
        {
            CitySettings_ShowDescription();
            SFX_Play(FGMMENU_CS_CANCEL);
            return 1;
        }
        else
            SFX_Play(FGMMENU_CS_BEEP1);
    }

    return 0;
}
void EventToggle_Update(GOBJ *g)
{
    EventToggleData *gp = g->userdata;
    JOBJ *j = g->hsd_object;
    int sel_idx = (gp->cursor.x * EVENTTOGGLE_OPTNUM_Y) + gp->cursor.y;

    // update name
    char buf[256];
    Text_Sanitize(stc_event_names[sel_idx].name, buf, sizeof(buf));
    Text_SetText(gp->name.text, 0, buf);                    // update text contents
    Text_CopyJointPosition(gp->name.text, gp->name.text_j); // update text position

    // update options
    for (int i = 0; i < GetElementsIn(gp->option); i++)
    {
        // update text position
        Text_CopyJointPosition(gp->option[i].text, gp->option[i].text_j);

        // update selected status
        int is_selected = (i == sel_idx) ? (1) : (0);
        JObj_SetFrameAndRate(gp->option[i].background_j, is_selected, 0);

        // update toggle status
        JObj_SetFrameAndRate(gp->option[i].toggle_j, gp->option[i].is_on, 0);
    }

    // update description text
    for (int i = 0; i < GetElementsIn(gp->description); i++)
    {
        Text_Sanitize(stc_event_names[sel_idx].desc[i], buf, sizeof(buf));
        Text_SetText(gp->description[i].text, 0, buf);                              // update text contents
        Text_CopyJointPosition(gp->description[i].text, gp->description[i].text_j); // update text position
    }

    // set event image
    JObj_SetFrameAndRate(JObj_GetIndex(j, 2), sel_idx, 0);

    JObj_AnimAll(g->hsd_object);
    return;
}