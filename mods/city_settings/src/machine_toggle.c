/*---------------------------------------------------------------------------*
    UI code for the city settings machine toggle menu.
 *---------------------------------------------------------------------------*/

#include "text.h"
#include "os.h"
#include "hsd.h"
#include "preload.h"
#include "scene.h"
#include "inline.h"
#include "game.h"
#include "audio.h"

#include "citysettings.h"   // needed for the custom archive ptr and p_link enum
#include "machine_toggle.h" //

static char *stc_machine_names[] = {
    "Compact Star",
    "Warp Star",
    "Turbo Star",
    "Formula Star",
    "Slick Star",
    "Swerve Star",
    "Wagon Star",
    "Bulk Star",
    "Shadow Star",
    "Winged Star",
    "Jet Star",
    "Rocket Star",
    "Wheelie Scooter",
    "Wheelie Bike",
    "Rex Wheelie",
    "Dragoon",
    "Hydra",
    "Flight Star",
    // "Free Star",
    // "Steer Star",
};
static u8 stc_togglekind_to_vckind[] = {
    VCKIND_COMPACT,
    VCKIND_WARP,
    VCKIND_TURBO,
    VCKIND_FORMULA,
    VCKIND_SLICK,
    VCKIND_SWERVE,
    VCKIND_WAGON,
    VCKIND_BULK,
    VCKIND_SHADOW,
    VCKIND_WINGED,
    VCKIND_JET,
    VCKIND_ROCKET,
    VCKIND_WHEELIESCOOTER,
    VCKIND_WHEELIEBIKE,
    VCKIND_REXWHEELIE,
    VCKIND_DRAGOON,
    VCKIND_HYDRA,
    VCKIND_FLIGHT,
    // VCKIND_FREE,
    // VCKIND_STEER,
};

GOBJ *MachineToggle_Create(int (**input_cb)(GOBJ *))
{
    // get menu assets
    HSD_Archive *custom_archive = CitySettings_GetCustomMenuArchive();
    JOBJSet **set = Archive_GetPublicAddress(custom_archive, "ScMenToggleStadium_scene_models");

    // get save data
    CitySettingsSave *cs = CitySettings_SaveGet();

    // create menu gobj
    GOBJ *g = JObj_LoadSet_SetPri(0, set[2], 0, 0, MENUPLINK_CUSTOM, MENUGX_1, 1, MachineToggle_Update, 0);

    // alloc data
    MachineToggleData *gp = HSD_MemAlloc(sizeof(MachineToggleData));
    GObj_AddUserData(g, 14, MachineToggle_Destroy, gp);

    // get jobj to attach options to
    JOBJ *attach_to = JObj_GetIndex(g->hsd_object, MCHNTOGGLE_BG_JOINT_OPTIONS);

    // create each option
    int opt_idx = 0;
    for (int col_idx = 0; col_idx < MCHNTOGGLE_OPTNUM_X; col_idx++)
    {
        for (int row_idx = 0; row_idx < MCHNTOGGLE_OPTNUM_Y; row_idx++)
        {
            // create jobj
            JOBJ *opt_j = JObj_LoadJoint(set[3]->jobj);                                          // create jobj
            JObj_AddSetAnim(opt_j, 0, set[3], 0, 0);                                             // add anims
            JObj_AddNext(attach_to, opt_j);                                                      // add to menu gobj
            gp->option[opt_idx].j = opt_j;                                                       // store ptr to lookup
            gp->option[opt_idx].text_j = JObj_GetIndex(opt_j, MCHNTOGGLE_OPTION_JOINT_TEXT);     // store ptr to lookup
            gp->option[opt_idx].background_j = JObj_GetIndex(opt_j, MCHNTOGGLE_OPTION_JOINT_BG); // store ptr to lookup
            gp->option[opt_idx].toggle_j = JObj_GetIndex(opt_j, MCHNTOGGLE_OPTION_JOINT_TOGGLE); // store ptr to lookup

            // init enabled state
            gp->option[opt_idx].is_on = (cs->random_machine_bitfield & (1 << stc_togglekind_to_vckind[opt_idx])) ? 1 : 0;

            // set icon frame
            JObj_SetFrameAndRate(JObj_GetIndex(opt_j, MCHNTOGGLE_OPTION_JOINT_ICON), opt_idx, 0); // set on/off state frame

            // create text
            Text *text = Text_CreateText(0, stc_scene_menu_common->text.canvas_idx);
            text->kerning = 1;
            text->align = 0;
            text->use_aspect = 1;

            text->color = (GXColor){0, 0, 0, 255};
            text->aspect = (Vec2){280, 32};
            text->viewport_scale = (Vec2){0.043 * 0.95, 0.043 * 0.95};
            Text_AddSubtext(text, 0, -17, stc_machine_names[opt_idx]);
            gp->option[opt_idx].text = text;

            opt_idx++;

            // position the value
            opt_j->trans.X = -(60 / 2) + (col_idx * 60);
            opt_j->trans.Y = 3 + ((MCHNTOGGLE_OPTNUM_Y - 1) * 6.8 / 2.0) - (row_idx * 6.8);
        }
    }

    JObj_SetMtxDirtySub(g->hsd_object);

    // init cursor
    gp->cursor.x = 0;
    gp->cursor.y = 0;

    // store input update callback
    *input_cb = MachineToggle_Input;

    return g;
}

void MachineToggle_Destroy(MachineToggleData *gp)
{
    CitySettingsSave *cs = CitySettings_SaveGet();

    for (int i = 0; i < GetElementsIn(gp->option); i++)
    {
        Text_Destroy(gp->option[i].text);

        MachineKind vc_kind = stc_togglekind_to_vckind[i];

        // write to save data struct
        if (gp->option[i].is_on)
            cs->random_machine_bitfield |= (1 << vc_kind);
        else
            cs->random_machine_bitfield &= ~(1 << vc_kind);
    }

    HSD_Free(gp);

    return;
}
int MachineToggle_Input(GOBJ *g)
{
    MachineToggleData *gp = g->userdata;
    int down = Pad_GetDown(20);
    int rapid_held = Pad_GetRapidHeld(20);

    if (rapid_held & (PAD_BUTTON_UP | PAD_BUTTON_DPAD_UP))
    {
        if (gp->cursor.y > 0)
            gp->cursor.y--;
        else
            gp->cursor.y = MCHNTOGGLE_OPTNUM_Y - 1;

        SFX_Play(FGMMENU_CS_MV);
    }
    else if (rapid_held & (PAD_BUTTON_DOWN | PAD_BUTTON_DPAD_DOWN))
    {
        if (gp->cursor.y < MCHNTOGGLE_OPTNUM_Y - 1)
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
            gp->cursor.x = MCHNTOGGLE_OPTNUM_X - 1;

        SFX_Play(FGMMENU_CS_MV);
    }
    else if (rapid_held & (PAD_BUTTON_RIGHT | PAD_BUTTON_DPAD_RIGHT))
    {
        if (gp->cursor.x < MCHNTOGGLE_OPTNUM_X - 1)
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
        int sel_idx = (gp->cursor.x * MCHNTOGGLE_OPTNUM_Y) + gp->cursor.y;
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
            SFX_Play(FGMMENU_CS_CANCEL);
            return 1;
        }
        else
            SFX_Play(FGMMENU_CS_BEEP1);
    }

    return 0;
}
void MachineToggle_Update(GOBJ *g)
{
    MachineToggleData *gp = g->userdata;
    JOBJ *j = g->hsd_object;
    int sel_idx = (gp->cursor.x * MCHNTOGGLE_OPTNUM_Y) + gp->cursor.y;

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

    JObj_AnimAll(g->hsd_object);
    return;
}
void MachineToggle_Verify()
{
    CitySettingsSave *cs = CitySettings_SaveGet();

    int all_machines = 0;
    for (int i = 0; i < GetElementsIn(stc_togglekind_to_vckind); i++)
        all_machines |= (1 << stc_togglekind_to_vckind[i]);

    if ((cs->random_machine_bitfield & all_machines) == 0)
        cs->random_machine_bitfield = MachineToggle_GetDefaults();

    return;
}
int MachineToggle_GetDefaults()
{
    return (1 << VCKIND_WARP | 1 << VCKIND_WINGED | 1 << VCKIND_SHADOW | 1 << VCKIND_BULK | 1 << VCKIND_SLICK | 1 << VCKIND_FORMULA | 1 << VCKIND_WAGON | 1 << VCKIND_ROCKET | 1 << VCKIND_SWERVE | 1 << VCKIND_TURBO | 1 << VCKIND_JET | 1 << VCKIND_WHEELIEBIKE | 1 << VCKIND_REXWHEELIE | 1 << VCKIND_WHEELIESCOOTER);
}