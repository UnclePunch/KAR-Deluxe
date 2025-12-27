/*---------------------------------------------------------------------------*
    UI code for the city settings stadium toggle menu.
 *---------------------------------------------------------------------------*/

#include "text.h"
#include "os.h"
#include "hsd.h"
#include "preload.h"
#include "scene.h"
#include "inline.h"
#include "game.h"
#include "audio.h"

#include "citysettings.h" // needed for the custom archive ptr and p_link enum
#include "stadium_toggle.h"

#include "text_joint/text_joint.h"

static StadiumToggleNames stc_stadium_names[] = {
    {
        "Drag Race: Straight",
        "A straight-up speed battle!",
        "",
    },
    {
        "Drag Race: Split",
        "A straight-up speed battle!",
        "",
    },
    {
        "Drag Race: Bumpers",
        "A straight-up speed battle!",
        "",
    },
    {
        "Drag Race: Ramps",
        "A straight-up speed battle!",
        "",
    },
    {
        "Destruction Derby: King Dedede",
        "Defeat your rivals and",
        "seize victory!",
    },
    {
        "Destruction Derby: Volcano",
        "Defeat your rivals and",
        "seize victory!",
    },
    {
        "Destruction Derby: UFO",
        "Defeat your rivals and",
        "seize victory!",
    },
    {
        "Destruction Derby: Inner City",
        "Defeat your rivals and",
        "seize victory!",
    },
    {
        "Destruction Derby: Outer City",
        "Defeat your rivals and",
        "seize victory!",
    },
    {
        "Air Glider",
        "Jump from the platform and",
        "fly as far as you can!",
    },
    {
        "Target Flight",
        "Fly into the number board",
        "and earn points! [2 tries]",
    },
    {
        "High Jump",
        "Tilt the Control Stick to",
        "pull off the best jump!",
    },
    {
        "Race: Fantasy Meadows",
        "Race for victory!",
        "",
    },
    {
        "Race: Magma Flows",
        "Race for victory!",
        "",
    },
    {
        "Race: Sky Sands",
        "Race for victory!",
        "",
    },
    {
        "Race: Frozen Hillside",
        "Race for victory!",
        "",
    },
    {
        "Race: Beanstalk Park",
        "Race for victory!",
        "",
    },
    {
        "Race: Celestial Valley",
        "Race for victory!",
        "",
    },
    {
        "Race: Machine Passage",
        "Race for victory!",
        "",
    },
    {
        "Race: Checker Knights",
        "Race for victory!",
        "",
    },
    {
        "Race: Nebula Belt",
        "Race for victory!",
        "",
    },
    {
        "Kirby Melee: Small",
        "Defeat as many enemies",
        "as you can!",
    },
    {
        "Kirby Melee: Large",
        "Defeat as many enemies",
        "as you can!",
    },
    {
        "VS. King Dedede",
        "Featuring King Dedede!",
        "Defeat him quickly!",
    },
};
static u8 stc_togglekind_to_stadiumkind[] = {
    STKIND_DRAG1,
    STKIND_DRAG2,
    STKIND_DRAG3,
    STKIND_DRAG4,
    STKIND_DESTRUCTION1,
    STKIND_DESTRUCTION2,
    STKIND_DESTRUCTION3,
    STKIND_DESTRUCTION4,
    STKIND_DESTRUCTION5,
    STKIND_AIRGLIDER,
    STKIND_TARGETFLIGHT,
    STKIND_HIGHJUMP,
    STKIND_SINGLERACE1,
    STKIND_SINGLERACE2,
    STKIND_SINGLERACE3,
    STKIND_SINGLERACE4,
    STKIND_SINGLERACE5,
    STKIND_SINGLERACE6,
    STKIND_SINGLERACE7,
    STKIND_SINGLERACE8,
    STKIND_SINGLERACE9,
    STKIND_MELEE1,
    STKIND_MELEE2,
    STKIND_VSKINGDEDEDE,
};

GOBJ *StadiumToggle_Create(int (**input_cb)(GOBJ *))
{
    // get menu assets
    HSD_Archive *custom_archive = CitySettings_GetCustomMenuArchive();
    JOBJSet **set = Archive_GetPublicAddress(custom_archive, "ScMenToggleStadium_scene_models");

    // get save data
    CitySettingsSave *cs = CitySettings_SaveGet();

    // create menu gobj
    GOBJ *g = JObj_LoadSet_SetPri(0, set[0], 0, 0, MENUPLINK_CUSTOM, MENUGX_1, 1, StadiumToggle_Update, 0);

    // alloc data
    StadiumToggleData *gp = HSD_MemAlloc(sizeof(StadiumToggleData));
    GObj_AddUserData(g, 14, StadiumToggle_Destroy, gp);

    // get jobj to attach options to
    JOBJ *attach_to = JObj_GetIndex(g->hsd_object, STADTOGGLE_BG_JOINT_OPTIONS);

    // create each option
    int opt_idx = 0;
    for (int col_idx = 0; col_idx < STADTOGGLE_OPTNUM_X; col_idx++)
    {
        for (int row_idx = 0; row_idx < STADTOGGLE_OPTNUM_Y; row_idx++)
        {
            // create jobj
            JOBJ *opt_j = JObj_LoadJoint(set[1]->jobj);                                          // create jobj
            JObj_AddSetAnim(opt_j, 0, set[1], 0, 0);                                             // add anims
            JObj_SetFrameAndRate(JObj_GetIndex(opt_j, 2), 1, 0);                                 // set on/off state frame
            JObj_AddNext(attach_to, opt_j);                                                      // add to menu gobj
            gp->option[opt_idx].j = opt_j;                                                       // store ptr to lookup
            gp->option[opt_idx].text_j = JObj_GetIndex(opt_j, STADTOGGLE_OPTION_JOINT_TEXT);     // store ptr to lookup
            gp->option[opt_idx].background_j = JObj_GetIndex(opt_j, STADTOGGLE_OPTION_JOINT_BG); // store ptr to lookup
            gp->option[opt_idx].toggle_j = JObj_GetIndex(opt_j, STADTOGGLE_OPTION_JOINT_TOGGLE); // store ptr to lookup

            // init enabled state
            gp->option[opt_idx].is_on = (cs->random_stadium_bitfield & (1 << stc_togglekind_to_stadiumkind[opt_idx])) ? 1 : 0;

            // create text
            Text *text = Text_CreateText(0, stc_scene_menu_common->text.canvas_idx);
            text->kerning = 1;
            text->align = 0;
            text->use_aspect = 1;

            text->color = (GXColor){0, 0, 0, 255};
            text->aspect = (Vec2){470, 32};
            text->viewport_scale = (Vec2){0.043 * 0.95, 0.043 * 0.95};
            Text_AddSubtext(text, 0, -15, stc_stadium_names[opt_idx].name);
            gp->option[opt_idx].text = text;

            opt_idx++;

            // position the value
            opt_j->trans.X = -42 + (col_idx * 70);
            opt_j->trans.Y = ((STADTOGGLE_OPTNUM_Y / 2) * 5) - (row_idx * 5);
        }
    }
    JObj_SetMtxDirtySub(g->hsd_object);

    Text *text;

    // create name text
    text = Text_CreateText(0, stc_scene_menu_common->text.canvas_idx);
    text->kerning = 1;
    text->align = 0;
    text->use_aspect = 1;

    text->color = (GXColor){255, 255, 255, 255};
    text->aspect = (Vec2){385, 32};
    Text_AddSubtext(text, 0, -15, "");
    gp->name.text = text;
    gp->name.text_j = JObj_GetIndex(g->hsd_object, STADTOGGLE_BG_JOINT_NAME);

    // create description text
    for (int i = 0; i < GetElementsIn(gp->description); i++)
    {
        text = Text_CreateText(0, stc_scene_menu_common->text.canvas_idx);
        text->kerning = 1;
        text->align = 0;
        text->use_aspect = 1;

        text->color = (GXColor){0, 0, 0, 255};
        text->aspect = (Vec2){600, 32};
        Text_AddSubtext(text, 0, -15, "");

        gp->description[i].text = text;
        gp->description[i].text_j = JObj_GetIndex(g->hsd_object, i + STADTOGGLE_BG_JOINT_DESCRIPTION);
    }

    // init cursor
    gp->cursor.x = 0;
    gp->cursor.y = 0;

    // request hide background panel
    CitySettings_HideDescription();

    // store input update callback
    *input_cb = StadiumToggle_Input;

    return g;
}

void StadiumToggle_Destroy(StadiumToggleData *gp)
{
    CitySettingsSave *cs = CitySettings_SaveGet();

    for (int i = 0; i < GetElementsIn(gp->option); i++)
    {
        Text_Destroy(gp->option[i].text);

        int st_kind = stc_togglekind_to_stadiumkind[i];

        // write to save data struct
        if (gp->option[i].is_on)
            cs->random_stadium_bitfield |= (1 << st_kind);
        else
            cs->random_stadium_bitfield &= ~(1 << st_kind);
    }

    // destroy text
    Text_Destroy(gp->name.text);

    for (int i = 0; i < GetElementsIn(gp->description); i++)
        Text_Destroy(gp->description[i].text);

    HSD_Free(gp);

    return;
}
int StadiumToggle_Input(GOBJ *g)
{
    StadiumToggleData *gp = g->userdata;
    int down = Pad_GetDown(20);
    int rapid_held = Pad_GetRapidHeld(20);

    if (rapid_held & (PAD_BUTTON_UP | PAD_BUTTON_DPAD_UP))
    {
        if (gp->cursor.y > 0)
            gp->cursor.y--;
        else
            gp->cursor.y = STADTOGGLE_OPTNUM_Y - 1;

        SFX_Play(FGMMENU_CS_MV);
    }
    else if (rapid_held & (PAD_BUTTON_DOWN | PAD_BUTTON_DPAD_DOWN))
    {
        if (gp->cursor.y < STADTOGGLE_OPTNUM_Y - 1)
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
            gp->cursor.x = STADTOGGLE_OPTNUM_X - 1;

        SFX_Play(FGMMENU_CS_MV);
    }
    else if (rapid_held & (PAD_BUTTON_RIGHT | PAD_BUTTON_DPAD_RIGHT))
    {
        if (gp->cursor.x < STADTOGGLE_OPTNUM_X - 1)
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
        int sel_idx = (gp->cursor.x * STADTOGGLE_OPTNUM_Y) + gp->cursor.y;
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
void StadiumToggle_Update(GOBJ *g)
{
    StadiumToggleData *gp = g->userdata;
    JOBJ *j = g->hsd_object;
    int sel_idx = (gp->cursor.x * STADTOGGLE_OPTNUM_Y) + gp->cursor.y;

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

    // update name
    char buf[256];
    Text_Sanitize(stc_stadium_names[sel_idx].name, buf, sizeof(buf));
    Text_SetText(gp->name.text, 0, buf);                    // update text contents
    Text_CopyJointPosition(gp->name.text, gp->name.text_j); // update text position

    // update description text
    for (int i = 0; i < GetElementsIn(gp->description); i++)
    {
        Text_Sanitize(stc_stadium_names[sel_idx].desc[i], buf, sizeof(buf));
        Text_SetText(gp->description[i].text, 0, buf);                              // update text contents
        Text_CopyJointPosition(gp->description[i].text, gp->description[i].text_j); // update text position
    }

    // set stadium image
    JObj_SetFrameAndRate(JObj_GetIndex(j, 2), sel_idx, 0);

    JObj_AnimAll(g->hsd_object);
    return;
}