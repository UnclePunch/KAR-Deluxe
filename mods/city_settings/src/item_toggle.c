/*---------------------------------------------------------------------------*
    UI code for the city settings item toggle menu.
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
#include "item_toggle.h"

#include "text_joint/text_joint.h"

static ItemToggleNames stc_item_names[] = {
    {
        .name = "Boost Up",
        .desc = {"Increases acceleration",
                 "and deceleration."},
    },
    {
        .name = "Boost Down",
        .desc = {"Decreases acceleration",
                 "and deceleration."},
    },
    {
        .name = "Top Speed Up",
        .desc = {"Increases maximum",
                 "speed."},
    },
    {
        .name = "Top Speed Down",
        .desc = {"Decreases maximum",
                 "speed."},
    },
    {
        .name = "Offense Up",
        .desc = {"Increases damage dealt",
                 "to others."},
    },
    {
        .name = "Offense Down",
        .desc = {"Decreases damage dealt",
                 "to others."},
    },
    {
        .name = "Defense Up",
        .desc = {"Decreases damage",
                 "received."},
    },
    {
        .name = "Defense Down",
        .desc = {"Increases damage",
                 "received."},
    },
    {
        .name = "Turn Up",
        .desc = {"Increases turning",
                 "speed."},
    },
    {
        .name = "Turn Down",
        .desc = {"Decreases turning",
                 "speed."},
    },
    {
        .name = "Glide Up",
        .desc = {"Increases air time and",
                 "improves mid-air control."},
    },
    {
        .name = "Glide Down",
        .desc = {"Decreases air time and",
                 "reduces mid-air control."},
    },
    {
        .name = "Charge Up",
        .desc = {"Increases charge gauge",
                 "fill rate."},
    },
    {
        .name = "Charge Down",
        .desc = {"Decreases charge gauge",
                 "fill rate."},
    },
    {
        .name = "Weight Up",
        .desc = {"Increases ground speed and damage,",
                 "decreases flight altitude."},
    },
    {
        .name = "Weight Down",
        .desc = {"Decreases ground speed and damage,",
                 "increases flight altitude."},
    },
    {
        .name = "HP Up",
        .desc = {"Increases maximum health.",
                 ""},
    },
    {
        .name = "All Up",
        .desc = {"Increases every stat",
                 "by 1."},
    },
    {
        .name = "Speed Up",
        .desc = {"Temporarily increases",
                 "speed."},
    },
    {
        .name = "Speed Down",
        .desc = {"Temporarily reduces",
                 "speed."},
    },
    {
        .name = "Attack Up",
        .desc = {"Temporarily increases",
                 "damage dealt."},
    },
    {
        .name = "Defense Up",
        .desc = {"Temporarily decreases",
                 "damage received."},
    },
    {
        .name = "Max Speed",
        .desc = {"Temporarily maximizes top speed,",
                 "charge, and boost."},
    },
    {
        .name = "No Charge",
        .desc = {"Temporarily disables the boost",
                 "gauge and reduces top speed."},
    },
    {
        .name = "Candy",
        .desc = {"Grants temporary",
                 "invulnerability."},
    },
    {
        .name = "Bomb Panel",
        .desc = {"Gives Kirby the Bomb Ability.",
                 "Hold and release A to throw."},
    },
    {
        .name = "Fire Panel",
        .desc = {"Gives Kirby the Fire Ability.",
                 "Press A to shoot flames."},
    },
    {
        .name = "Freeze Panel",
        .desc = {"Gives Kirby the Freeze Ability.",
                 "Hold A to create an immobilizing flurry."},
    },
    {
        .name = "Sleep Panel",
        .desc = {"Zzzzz Zzzzz.",
                 "Do not disturb."},
    },
    {
        .name = "Wheel Panel",
        .desc = {"Transform Kirby into a fast",
                 "but unruly wheel."},
    },
    {
        .name = "Wing Panel",
        .desc = {"Transforms Kirby into a bird.",
                 "Excellent glide ability but cannot charge."},
    },
    {
        .name = "Plasma Panel",
        .desc = {"Move the control stick to build charge",
                 "then press A to shoot 1 of 5 projectiles."},
    },
    {
        .name = "Tornado Panel",
        .desc = {"Launch enemies into the sky.",
                 "Charge/Quick spin to activate, even in mid-air."},
    },
    {
        .name = "Sword Panel",
        .desc = {"Gives Kirby the Sword ability.",
                 "Kirby will automatically attack nearby targets."},
    },
    {
        .name = "Needle Panel",
        .desc = {"Gives Kirby the Needle ability.",
                 "Hold A to unleash a myriad of spikes."},
    },
    {
        .name = "Mike Panel",
        .desc = {"Gives Kirby the Mike ability.",
                 "Damages everything in a nearby radius."},
    },
    {
        .name = "Maxim Tomato",
        .desc = {"The iconic Maxim Tomato.",
                 "Fully heals Kirby."},
    },
    {
        .name = "Energy Drink",
        .desc = {"An invogorating Energy Drink.",
                 "Heals 70 health points."},
    },
    {
        .name = "Ice Cream",
        .desc = {"A sweet Ice Cream cone.",
                 "Heals 5 health points."},
    },
    {
        .name = "Riceball",
        .desc = {"Also known as an onigiri.",
                 "Heals 20 health points."},
    },
    {
        .name = "Roast Chicken",
        .desc = {"A seasoned Roast Chicken.",
                 "Heals 35 health points."},
    },
    {
        .name = "Curry",
        .desc = {"A hot plate of curry.",
                 "Heals 30 health points."},
    },
    {
        .name = "Ramen",
        .desc = {"A delicious bowl of ramen.",
                 "Heals 30 health points."},
    },
    {
        .name = "Omelet",
        .desc = {"A tasty omelet.",
                 "Heals 25 health points."},
    },
    {
        .name = "Hamburger",
        .desc = {"A juicy hamburger.",
                 "Heals 25 health points."},
    },
    {
        .name = "Sushi",
        .desc = {"A cold plate of sushi.",
                 "Heals 25 health points."},
    },
    {
        .name = "Hot Dog",
        .desc = {"A yummy Hot Dog.",
                 "Heals 25 health points."},
    },
    {
        .name = "Apple",
        .desc = {"A nutritious apple.",
                 "Heals 18 health points."},
    },
    {
        .name = "Fireworks",
        .desc = {"Attack your foes from afar.",
                 "Contains 25 shots."},
    },
    {
        .name = "Panic Spin",
        .desc = {"Spin indefinitely, dealing",
                 "damage to anything within reach."},
    },
    {
        .name = "Sensor Bomb",
        .desc = {"Press A to plant a bomb.",
                 "Motion will trigger an explosion."},
    },
    {
        .name = "Gold Spike",
        .desc = {"Gives Kirby 3 giant Gordos.",
                 "Press A to aim and throw."},
    },
};
static u8 stc_togglekind_to_itemkind[] = {
    ITKIND_ACCEL,
    ITKIND_ACCELDOWN,
    ITKIND_TOPSPEED,
    ITKIND_TOPSPEEDDOWN,
    ITKIND_OFFENSE,
    ITKIND_OFFENSEDOWN,
    ITKIND_DEFENSE,
    ITKIND_DEFENSEDOWN,
    ITKIND_TURN,
    ITKIND_TURNDOWN,
    ITKIND_GLIDE,
    ITKIND_GLIDEDOWN,
    ITKIND_CHARGE,
    ITKIND_CHARGEDOWN,
    ITKIND_WEIGHT,
    ITKIND_WEIGHTDOWN,
    ITKIND_HP,
    ITKIND_ALLUP,
    ITKIND_SPEEDMAX,
    ITKIND_SPEEDMIN,
    ITKIND_OFFENSEMAX,
    ITKIND_DEFENSEMAX,
    ITKIND_CHARGEMAX,
    ITKIND_CHARGENONE,
    ITKIND_CANDY,
    ITKIND_COPYBOMB,
    ITKIND_COPYFIRE,
    ITKIND_COPYICE,
    ITKIND_COPYSLEEP,
    ITKIND_COPYTIRE,
    ITKIND_COPYBIRD,
    ITKIND_COPYPLASMA,
    ITKIND_COPYTORNADO,
    ITKIND_COPYSWORD,
    ITKIND_COPYSPIKE,
    ITKIND_COPYMIC,
    ITKIND_FOODMAXIMTOMATO,
    ITKIND_FOODENERGYDRINK,
    ITKIND_FOODICECREAM,
    ITKIND_FOODRICEBALL,
    ITKIND_FOODCHICKEN,
    ITKIND_FOODCURRY,
    ITKIND_FOODRAMEN,
    ITKIND_FOODOMELET,
    ITKIND_FOODHAMBURGER,
    ITKIND_FOODSUSHI,
    ITKIND_FOODHOTDOG,
    ITKIND_FOODAPPLE,
    ITKIND_FIREWORKS,
    ITKIND_PANICSPIN,
    ITKIND_TIMEBOMB,
    ITKIND_GORDO,
};

GOBJ *ItemToggle_Create(int (**input_cb)(GOBJ *))
{
    // get menu assets
    HSD_Archive *custom_archive = CitySettings_GetCustomMenuArchive();
    JOBJSet **set = Archive_GetPublicAddress(custom_archive, "ScMenToggleStadium_scene_models");

    // get save data
    CitySettingsSave *cs = CitySettings_SaveGet();

    // create menu gobj
    GOBJ *g = JObj_LoadSet_SetPri(0, set[6], 0, 0, MENUPLINK_CUSTOM, MENUGX_1, 1, ItemToggle_Update, 0);

    // alloc data
    ItemToggleData *gp = HSD_MemAlloc(sizeof(ItemToggleData));
    GObj_AddUserData(g, 14, ItemToggle_Destroy, gp);

    // get jobj to attach options to
    JOBJ *attach_to = JObj_GetIndex(g->hsd_object, ITEMTOGGLE_BG_JOINT_OPTIONS);

    // create each option
    for (int opt_idx = 0; opt_idx < ITEMTOGGLE_OPTNUM; opt_idx++)
    {
        int col_idx = opt_idx % ITEMTOGGLE_OPTNUM_X;
        int row_idx = opt_idx / (ITEMTOGGLE_OPTNUM_Y + 1);

        // create jobj
        JOBJ *opt_j = JObj_LoadJoint(set[7]->jobj);                                          // create jobj
        JObj_AddSetAnim(opt_j, 0, set[7], 0, 0);                                             // add anims
        JObj_AddNext(attach_to, opt_j);                                                      // add to menu gobj
        gp->option[opt_idx].j = opt_j;                                                       // store ptr to lookup
        gp->option[opt_idx].background_j = JObj_GetIndex(opt_j, ITEMTOGGLE_OPTION_JOINT_BG); // store ptr to lookup
        gp->option[opt_idx].toggle_j = JObj_GetIndex(opt_j, ITEMTOGGLE_OPTION_JOINT_TOGGLE); // store ptr to lookup
        gp->option[opt_idx].icon_j = JObj_GetIndex(opt_j, ITEMTOGGLE_OPTION_JOINT_ICON);     // store ptr to lookup

        // init enabled state
        ItemKind it_kind = stc_togglekind_to_itemkind[opt_idx];
        gp->option[opt_idx].is_on = (cs->random_item_bitfield & (1ULL << it_kind)) ? 1 : 0;

        // set icon frame
        JObj_PauseAllTObjAnimFrame(gp->option[opt_idx].icon_j, opt_idx);

        // position the value
        opt_j->trans.X = -((((float)ITEMTOGGLE_OPTNUM_X - 1) * 15.2) / 2.0) + (col_idx * 15.2);
        opt_j->trans.Y = (((float)ITEMTOGGLE_OPTNUM_Y / 2.0) * 8) - (row_idx * 8);
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
    gp->name.text_j = JObj_GetIndex(g->hsd_object, ITEMTOGGLE_BG_JOINT_NAME);

    // create description text
    for (int i = 0; i < GetElementsIn(gp->description); i++)
    {
        text = Text_CreateText(0, stc_scene_menu_common->text.canvas_idx);
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
        gp->description[i].text_j = JObj_GetIndex(g->hsd_object, i + ITEMTOGGLE_BG_JOINT_DESCRIPTION);
    }

    // init cursor
    gp->cursor.x = 0;
    gp->cursor.y = 0;

    // request hide background panel
    CitySettings_HideDescription();

    // store input update callback
    *input_cb = ItemToggle_Input;

    return g;
}

void ItemToggle_Destroy(ItemToggleData *gp)
{
    CitySettingsSave *cs = CitySettings_SaveGet();

    for (int i = 0; i < GetElementsIn(gp->option); i++)
    {
        ItemKind it_kind = stc_togglekind_to_itemkind[i];

        // write to save data struct
        if (gp->option[i].is_on)
            cs->random_item_bitfield |= (1ULL << it_kind);
        else
            cs->random_item_bitfield &= ~(1ULL << it_kind);
    }

    // destroy text
    Text_Destroy(gp->name.text);

    for (int i = 0; i < GetElementsIn(gp->description); i++)
        Text_Destroy(gp->description[i].text);

    HSD_Free(gp);

    return;
}
int ItemToggle_Input(GOBJ *g)
{
    ItemToggleData *gp = g->userdata;
    int down = Pad_GetDown(20);
    int rapid_held = Pad_GetRapidHeld(20);

    if (rapid_held & (PAD_BUTTON_UP | PAD_BUTTON_DPAD_UP))
    {
        if (gp->cursor.y > 0)
            gp->cursor.y--;
        else
            gp->cursor.y = ITEMTOGGLE_OPTNUM_Y - 1;

        int sel_idx = (gp->cursor.y * ITEMTOGGLE_OPTNUM_X) + gp->cursor.x;
        if (sel_idx >= ITEMTOGGLE_OPTNUM)

            gp->cursor.y = (ITEMTOGGLE_OPTNUM - ITEMTOGGLE_OPTNUM_X) / (ITEMTOGGLE_OPTNUM_Y + 1);

        SFX_Play(FGMMENU_CS_MV);
    }
    else if (rapid_held & (PAD_BUTTON_DOWN | PAD_BUTTON_DPAD_DOWN))
    {
        if (gp->cursor.y < ITEMTOGGLE_OPTNUM_Y - 1)
            gp->cursor.y++;
        else
            gp->cursor.y = 0;

        int sel_idx = (gp->cursor.y * ITEMTOGGLE_OPTNUM_X) + gp->cursor.x;
        if (sel_idx >= ITEMTOGGLE_OPTNUM)
            gp->cursor.y = 0;

        SFX_Play(FGMMENU_CS_MV);
    }
    else if (rapid_held & (PAD_BUTTON_LEFT | PAD_BUTTON_DPAD_LEFT))
    {
        if (gp->cursor.x > 0)
            gp->cursor.x--;
        else
            gp->cursor.x = ITEMTOGGLE_OPTNUM_X - 1;

        int sel_idx = (gp->cursor.y * ITEMTOGGLE_OPTNUM_X) + gp->cursor.x;
        if (sel_idx >= ITEMTOGGLE_OPTNUM)
            gp->cursor.x = ITEMTOGGLE_OPTNUM % ITEMTOGGLE_OPTNUM_Y;

        SFX_Play(FGMMENU_CS_MV);
    }
    else if (rapid_held & (PAD_BUTTON_RIGHT | PAD_BUTTON_DPAD_RIGHT))
    {
        if (gp->cursor.x < ITEMTOGGLE_OPTNUM_X - 1)
            gp->cursor.x++;
        else
            gp->cursor.x = 0;

        int sel_idx = (gp->cursor.y * ITEMTOGGLE_OPTNUM_X) + gp->cursor.x;
        if (sel_idx >= ITEMTOGGLE_OPTNUM)
            gp->cursor.x = 0;

        SFX_Play(FGMMENU_CS_MV);
    }

    int sel_idx = (gp->cursor.y * ITEMTOGGLE_OPTNUM_X) + gp->cursor.x;
    if (down & PAD_TRIGGER_Z)
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
void ItemToggle_Update(GOBJ *g)
{
    ItemToggleData *gp = g->userdata;
    JOBJ *j = g->hsd_object;
    int sel_idx = (gp->cursor.y * ITEMTOGGLE_OPTNUM_X) + gp->cursor.x;

    // update options
    for (int i = 0; i < GetElementsIn(gp->option); i++)
    {
        // update selected status
        int is_selected = (i == sel_idx) ? (1) : (0);
        JObj_SetFrameAndRate(gp->option[i].background_j, is_selected, 0);

        // update toggle status
        JObj_SetAllMObjAnimFrameAndRate(gp->option[i].icon_j, gp->option[i].is_on, 0);
        JObj_SetFrameAndRate(gp->option[i].toggle_j, gp->option[i].is_on, 0);
    }

    // update name
    char buf[256];
    Text_Sanitize(stc_item_names[sel_idx].name, buf, sizeof(buf));
    Text_SetText(gp->name.text, 0, buf);                    // update text contents
    Text_CopyJointPosition(gp->name.text, gp->name.text_j); // update text position

    // update description text
    for (int i = 0; i < GetElementsIn(gp->description); i++)
    {
        Text_Sanitize(stc_item_names[sel_idx].desc[i], buf, sizeof(buf));
        Text_SetText(gp->description[i].text, 0, buf);                              // update text contents
        Text_CopyJointPosition(gp->description[i].text, gp->description[i].text_j); // update text position
    }

    // set item image
    JObj_SetFrameAndRate(JObj_GetIndex(j, ITEMTOGGLE_BG_JOINT_ITEM), sel_idx, 0);

    JObj_AnimAll(g->hsd_object);
    return;
}