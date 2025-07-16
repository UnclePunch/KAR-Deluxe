#include "text.h"
#include "useful.h"
#include "hsd.h"
#include "preload.h"
#include "scene.h"
#include "inline.h"
#include "audio.h"
#include "game.h"

#include "citysettings.h"
#include "code_patch/code_patch.h"

#include "menu.h"
#include "menu_define.c"
#include "patch.h"

void Major_Enter()
{
    return;
}
void Major_ExitMinor()
{
    Scene_SetNextMajor(MJRKIND_MENU);
    Scene_ExitMajor();

    return;
}
static MajorSceneDesc major_scene =
    {
        .major_id = -1,
        .next_major_id = 0,
        .initial_minor_id = MNRKIND_STADIUMSELECT,
        .cb_Enter = Major_Enter,
        .cb_ExitMinor = Major_ExitMinor,
};

void Minor_Load()
{
    return;
}
void Minor_Think()
{

    if (Pad_GetDown(20) & PAD_BUTTON_B)
    {
        SFX_Play(FGMMENU_CS_CANCEL);
        Scene_ExitMinor();
    }

    return;
}
void Minor_Exit()
{
    return;
}
static MinorSceneDesc minor_scene = {
    .idx = -1,
    .x1 = -1,
    .cb_Load = CitySettings_Load,
    .cb_Exit = CitySettings_Exit,
    .cb_ThinkPreGObjProc = CitySettings_Think,
    .preload_kind = 0,
};

void CitySettings_OnGameStart()
{
    Patches_Init(); // insert hooks in game code to read from city settings

    return;
}
CODEPATCH_HOOKCREATE(0x80014448, "", CitySettings_OnGameStart, "", 0)

static void (*DOL_CitySettings_Load)();
static void (*DOL_CitySettings_Think)(void *data);
static void (*DOL_CitySettings_Exit)(void *data);
static HSD_Archive *settings_archive = 0;
static CitySettingsData settings_data;
static CitySettingsSave *stc_city_save;
void CitySettings_Init()
{

    // replace city game settings minor scene functions
    MinorSceneDesc *desc = KARPlus_GetMinorScenes();
    while (desc->idx != MNRKIND_NUM)
    {
        if (desc->idx == MNRKIND_CITYSETTINGS)
        {
            // OSReport("city settings minor data @ %x\n", desc);

            DOL_CitySettings_Load = desc->cb_Load;
            DOL_CitySettings_Think = desc->cb_ThinkPreGObjProc;
            DOL_CitySettings_Exit = desc->cb_Exit;

            desc->cb_Load = CitySettings_Load;
            desc->cb_ThinkPreGObjProc = CitySettings_Think;
            desc->cb_Exit = CitySettings_Exit;
            break;
        }

        desc++;
    }

    // add hook to game start code that applies patches at the beginning of every game
    CODEPATCH_HOOKAPPLY(0x80014448);

    return;
}

void CitySettings_SaveInit(CitySettingsSave *save, int req_init)
{
    stc_city_save = save;

    if (req_init)
    {
        // init default save data
        CitySettings_CopyToSave(&top_menu);

        save->random_event_bitfield = -1;
        save->random_stadium_bitfield = -1;
        save->random_item_bitfield = -1;
        save->random_machine_bitfield = (1 << VCKIND_WARP | 1 << VCKIND_WINGED | 1 << VCKIND_SHADOW | 1 << VCKIND_BULK | 1 << VCKIND_SLICK | 1 << VCKIND_FORMULA | 1 << VCKIND_WAGON | 1 << VCKIND_ROCKET | 1 << VCKIND_SWERVE | 1 << VCKIND_TURBO | 1 << VCKIND_JET | 1 << VCKIND_WHEELIEBIKE | 1 << VCKIND_REXWHEELIE | 1 << VCKIND_WHEELIESCOOTER);
    }

    CitySettings_UpdateVanillaSettings(); // apply vanilla settings immediately

    // apply patches on bootup
    Patches_Init(); // insert hooks in game code to read from city settings
}
CitySettingsSave *CitySettings_SaveGet()
{
    return stc_city_save;
}

void CitySettings_Load()
{

    CitySettings_Create(); //

    return;
}
void CitySettings_Think(void *data)
{
    if (!settings_data.menu.gobj_cur)
        return;

    CitySettingsMenuDesc *desc = settings_data.menu.desc_cur;

    // update inputs
    if (settings_data.menu.gobj_cur->p_link == MENUPLINK_GENERIC) // generic menu
    {
        CitySettingsMenuData *md = settings_data.menu.gobj_cur->userdata;
        CitySettingsMenuAction action_kind = Menu_Input(settings_data.menu.gobj_cur);

        switch (action_kind)
        {
        case (CITYSETTING_MENUACT_NONE):
        case (CITYSETTING_MENUACT_CHANGE):
        {
            break;
        }
        case (CITYSETTING_MENUACT_ADVANCE):
        {
            settings_data.menu.dir = CITYSETTING_MENUDIR_ADVANCE; // animate forward

            // if previous menu still exists, destroy it
            if (settings_data.menu.gobj_prev)
                GObj_Destroy(settings_data.menu.gobj_prev);

            settings_data.menu.gobj_prev = settings_data.menu.gobj_cur; // update prev menu

            CitySettingsMenuDesc *next_desc = desc->generic.options[desc->generic.cursor_val].u.menu.desc;
            next_desc->prev = desc;                  // store pointer to prev menu to the next menu
            settings_data.menu.desc_cur = next_desc; // update current menu desc pointer

            // create incoming menu
            if (next_desc->kind == CITYSETTING_MENUKIND_GENERIC)
                settings_data.menu.gobj_cur = Menu_Create(next_desc); // create new menu
            else if (next_desc->kind == CITYSETTING_MENUKIND_CUSTOM)
                settings_data.menu.gobj_cur = next_desc->custom.init_menu_cb(&settings_data.menu.input_update); // create new menu

            // menu name update
            CitySettingsMenuDesc *this_desc = next_desc;
            for (int i = 0; i < GetElementsIn(settings_data.menu.name); i++)
            {
                static u8 anim_ids[] = {0, 2, 4};

                if (this_desc) // animate it
                {
                    Text_SetText(settings_data.menu.name[i].text, 0, this_desc->name);
                    JObj_AddSetAnim(settings_data.menu.name[i].j, anim_ids[i], settings_data.menu.name_jobjset[0], 0, 1);

                    this_desc = this_desc->prev;
                }
                else // hide it
                    JObj_AddSetAnim(settings_data.menu.name[i].j, 6, settings_data.menu.name_jobjset[0], 0, 1);
            }

            // new menu goes offscreen
            JOBJ *menu_cur_j = settings_data.menu.gobj_cur->hsd_object;
            menu_cur_j->trans = (Vec3){60, 0, 0};

            // prev menu is focused
            JOBJ *menu_prev_j = settings_data.menu.gobj_prev->hsd_object;
            menu_prev_j->trans = (Vec3){0, 0, 0};

            break;
        }
        case (CITYSETTING_MENUACT_REGRESS):
        {
            CitySettingsMenuDesc *prev_desc = desc->prev;

            if (prev_desc)
            {
                desc->generic.cursor_val = 0;
                settings_data.menu.dir = CITYSETTING_MENUDIR_REGRESS; // animate backwards

                // if previous menu still exists, destroy it
                if (settings_data.menu.gobj_prev)
                    GObj_Destroy(settings_data.menu.gobj_prev);

                settings_data.menu.desc_cur = prev_desc;

                settings_data.menu.gobj_prev = settings_data.menu.gobj_cur; // update prev menu
                settings_data.menu.gobj_cur = Menu_Create(prev_desc);

                // menu name update
                CitySettingsMenuDesc *this_desc = desc;
                for (int i = 0; i < GetElementsIn(settings_data.menu.name); i++)
                {
                    static u8 anim_ids[] = {1, 3, 5};

                    if (this_desc) // animate it
                    {
                        Text_SetText(settings_data.menu.name[i].text, 0, this_desc->name);
                        JObj_AddSetAnim(settings_data.menu.name[i].j, anim_ids[i], settings_data.menu.name_jobjset[0], 0, 1);

                        this_desc = this_desc->prev;
                    }
                    else // hide it
                        JObj_AddSetAnim(settings_data.menu.name[i].j, 6, settings_data.menu.name_jobjset[0], 0, 1);
                }

                // new menu goes offscreen
                JOBJ *menu_cur_j = settings_data.menu.gobj_cur->hsd_object;
                menu_cur_j->trans = (Vec3){-60, 0, 0};

                // prev menu is focused
                JOBJ *menu_prev_j = settings_data.menu.gobj_prev->hsd_object;
                menu_prev_j->trans = (Vec3){0, 0, 0};

                // remove pointer to previous menu
                desc->prev = 0;
            }
            else
            {
                Scene_SetDirection(PAD_BUTTON_B);
                Scene_ExitMinor();
            }

            break;
        }
        case (CITYSETTING_MENUACT_EXIT):
        {
            Scene_SetDirection(PAD_BUTTON_B);
            Scene_ExitMinor();

            break;
        }
        }
    }
    else if (settings_data.menu.gobj_cur->p_link == MENUPLINK_CUSTOM)
    {
        if (settings_data.menu.input_update &&
            settings_data.menu.input_update(settings_data.menu.gobj_cur))
        {
            CitySettingsMenuDesc *prev_desc = desc->prev;

            if (prev_desc)
            {
                settings_data.menu.dir = CITYSETTING_MENUDIR_REGRESS; // animate backwards

                // if previous menu still exists, destroy it
                if (settings_data.menu.gobj_prev)
                    GObj_Destroy(settings_data.menu.gobj_prev);

                settings_data.menu.desc_cur = prev_desc;

                settings_data.menu.gobj_prev = settings_data.menu.gobj_cur; // update prev menu
                settings_data.menu.gobj_cur = Menu_Create(prev_desc);

                // menu name update
                CitySettingsMenuDesc *this_desc = desc;
                for (int i = 0; i < GetElementsIn(settings_data.menu.name); i++)
                {
                    static u8 anim_ids[] = {1, 3, 5};

                    if (this_desc) // animate it
                    {
                        Text_SetText(settings_data.menu.name[i].text, 0, this_desc->name);
                        JObj_AddSetAnim(settings_data.menu.name[i].j, anim_ids[i], settings_data.menu.name_jobjset[0], 0, 1);

                        this_desc = this_desc->prev;
                    }
                    else // hide it
                        JObj_AddSetAnim(settings_data.menu.name[i].j, 6, settings_data.menu.name_jobjset[0], 0, 1);
                }

                // new menu goes offscreen
                JOBJ *menu_cur_j = settings_data.menu.gobj_cur->hsd_object;
                menu_cur_j->trans = (Vec3){-60, 0, 0};

                // prev menu is focused
                JOBJ *menu_prev_j = settings_data.menu.gobj_prev->hsd_object;
                menu_prev_j->trans = (Vec3){0, 0, 0};

                // remove pointer to previous menu
                desc->prev = 0;
            }
            else
            {
                Scene_SetDirection(PAD_BUTTON_B);
                Scene_ExitMinor();
            }
        }
    }

    // move current menu to background joint
    if (settings_data.is_intro_anim)
    {
        JObj_GetWorldPosition(JObj_GetIndex(settings_data.bg_gobj->hsd_object, 20),
                              0,
                              &((JOBJ *)settings_data.menu.gobj_cur->hsd_object)->trans);
        JObj_SetMtxDirtySub(settings_data.menu.gobj_cur->hsd_object);
    }

    float move_speed = 12;
    switch (settings_data.menu.dir)
    {
    case (CITYSETTING_MENUDIR_NONE):
    {
        break;
    }
    case (CITYSETTING_MENUDIR_ADVANCE):
    {

        JOBJ *menu_cur_j = settings_data.menu.gobj_cur->hsd_object;
        JOBJ *menu_prev_j = settings_data.menu.gobj_prev->hsd_object;

        // move menus
        menu_cur_j->trans.X -= move_speed;
        menu_prev_j->trans.X -= move_speed;

        // check if done moving
        if (menu_cur_j->trans.X <= 0)
        {
            menu_cur_j->trans.X = 0;

            // destroy old menu
            GObj_Destroy(settings_data.menu.gobj_prev);
            settings_data.menu.gobj_prev = 0;

            // stop transition
            settings_data.menu.dir = CITYSETTING_MENUDIR_NONE;
        }

        JObj_SetMtxDirtySub(menu_cur_j);
        JObj_SetMtxDirtySub(menu_prev_j);

        break;
    }
    case (CITYSETTING_MENUDIR_REGRESS):
    {
        JOBJ *menu_cur_j = settings_data.menu.gobj_cur->hsd_object;
        JOBJ *menu_prev_j = settings_data.menu.gobj_prev->hsd_object;

        // move menus
        menu_cur_j->trans.X += move_speed;
        menu_prev_j->trans.X += move_speed;

        // check if done moving
        if (menu_cur_j->trans.X >= 0)
        {
            menu_cur_j->trans.X = 0;

            // destroy old menu
            GObj_Destroy(settings_data.menu.gobj_prev);
            settings_data.menu.gobj_prev = 0;

            // stop transition
            settings_data.menu.dir = CITYSETTING_MENUDIR_NONE;
        }

        JObj_SetMtxDirtySub(menu_cur_j);
        JObj_SetMtxDirtySub(menu_prev_j);

        break;
    }
    }

    return;
}
void CitySettings_Exit(void *data)
{
    // destroy menu gobjs
    CitySettings_Destroy();

    CitySettings_CopyToSave(&top_menu);
    CitySettings_UpdateVanillaSettings();
    KARPlus_WriteSave();

    return;
}

void CitySettings_Create()
{
    // stc_preload_menu_files[18] = 1; // load MnSelruleAll into the preload cache
    KARPlus_AddPreloadMenuFile("MnSelruleCustom");
    Preload_Invalidate();
    Preload_Update();

    // Gm_LoadGameFile(&settings_archive, "MnSelruleAll");
    // HSD_SObjDesc *sobj = Archive_GetPublicAddress(settings_archive, "ScMenSelrule_scene_data");

    Gm_LoadGameFile(&settings_archive, "MnSelruleCustom");
    HSD_SObjDesc *sobj = Archive_GetPublicAddress(settings_archive, "ScMenSelrule_scene_data");

    // create cam
    GOBJ *gc = CitySettings_CamCreate(sobj->cobjdesc[0]);

    // create lights
    GOBJ *gl = GOBJ_EZCreator(38, 32, 0,
                              0, 0,
                              HSD_OBJKIND_LOBJ, sobj->lights,
                              0, 0,
                              LObj_GX, MENUGX_0, 0);

    // create bg jobj
    settings_data.bg_gobj = CitySettings_BGCreate();
    settings_data.is_intro_anim = 1;

    // create description text
    stc_scene_menu_common->canvas_idx = Text_CreateCanvas(0, -1, 41, 17, 0, MENUGX_2, 0, -1);
    Text *text = Text_CreateText(0, stc_scene_menu_common->canvas_idx);
    text->kerning = 1;
    text->align = 0;
    text->use_aspect = 1;
    text->color = (GXColor){204, 204, 204, 255};
    // text->trans = (Vec3){-21, -0.4 + 17.1, 0};
    text->aspect = (Vec2){890, 32};
    text->viewport_scale = (Vec2){0.043 * 1.25, 0.05 * 1.25};
    // text->hidden = 1;
    Text_AddSubtext(text, 0, 0, "\x81\x5f");
    settings_data.description_text = text;

    // create menu name text
    settings_data.menu.name_jobjset = Archive_GetPublicAddress(settings_archive, "ScMenSelrulePanel_scene_models");
    for (int i = 0; i < GetElementsIn(settings_data.menu.name); i++)
    {
        text = Text_CreateText(0, stc_scene_menu_common->canvas_idx);
        text->kerning = 1;
        text->align = 1;
        text->use_aspect = 1;
        text->color = (GXColor){179, 179, 179, 255};
        // text->trans = (Vec3){1, -0.4 - 18.3, 0};
        text->aspect = (Vec2){240, 32};
        text->viewport_scale = (Vec2){0.043 * 1.55, 0.05 * 1.55};
        Text_AddSubtext(text, -10, -16, "");
        settings_data.menu.name[i].text = text;
        settings_data.menu.name[i].j = JObj_LoadJoint(settings_data.menu.name_jobjset[0]->jobj);

        // add hidden animation
        JObj_AddSetAnim(settings_data.menu.name[0].j, 6, settings_data.menu.name_jobjset[0], 0, 1);
    }

    Menu_Init(settings_archive);
    top_menu.generic.cursor_val = 0;      // init cursor
    CitySettings_CopyFromSave(&top_menu); // copy settings from save file to menu
    settings_data.menu.desc_cur = &top_menu;

    settings_data.menu.gobj_cur = Menu_Create(&top_menu); // create top menu
    settings_data.menu.gobj_prev = 0;                     // no previous
    settings_data.menu.dir = CITYSETTING_MENUDIR_NONE;    // no menudir
    settings_data.menu.input_update = 0;                  // init func ptr

    // init first menu name
    Text_SetText(settings_data.menu.name[0].text, 0, top_menu.name);
    JObj_AddSetAnim(settings_data.menu.name[0].j, 0, settings_data.menu.name_jobjset[0], 0, 1);
}
void CitySettings_Destroy()
{
    if (settings_data.menu.gobj_cur)
    {
        GObj_Destroy(settings_data.menu.gobj_cur);
        settings_data.menu.gobj_cur = 0;
    }
    if (settings_data.menu.gobj_prev)
    {
        GObj_Destroy(settings_data.menu.gobj_prev);
        settings_data.menu.gobj_prev = 0;
    }
    if (settings_data.bg_gobj)
    {
        GObj_Destroy(settings_data.bg_gobj);
        settings_data.bg_gobj = 0;
    }

    if (settings_data.description_text)
    {
        Text_Destroy(settings_data.description_text);
        settings_data.description_text = 0;
    }

    for (int i = 0; i < GetElementsIn(settings_data.menu.name); i++)
    {
        if (settings_data.menu.name[i].text)
            Text_Destroy(settings_data.menu.name[i].text);
        if (settings_data.menu.name[i].j)
            JObj_RemoveAll(settings_data.menu.name[i].j);
    }
}

static COBJDesc *stc_cobj_desc = 0;
GOBJ *CitySettings_CamCreate(COBJDesc *desc)
{
    stc_cobj_desc = desc;

    GOBJ *gc = GOBJ_EZCreator(40, 17, 0,
                              0, 0,
                              HSD_OBJKIND_COBJ, desc,
                              CitySettings_CameraThink, 0,
                              CitySettings_CamGX, 4, 4);
    gc->cobj_links = (1ULL << MENUGX_0) | (1ULL << MENUGX_1) | (1ULL << MENUGX_2);
}
void CitySettings_CamGX(GOBJ *g)
{
    if (!CObj_SetCurrent(g->hsd_object))
        return;

    CObj_SetEraseColor(160, 182, 134, 255);
    CObj_EraseScreen(g->hsd_object, GX_ENABLE, GX_ENABLE, GX_ENABLE);

    CObj_RenderGXLinks(g, (1 << 0) | (1 << 1) | (1 << 2));
    CObj_EndCurrent();

    return;
}
void CitySettings_CameraThink(GOBJ *gobj) // 8022BA1C
{

#define CAM_STICKDEADZONE 0.4 // minimum magnitude per axis to move camera
#define CAM_MAXROTATEDEG 30   // max amount to rotate the camera in degrees
    Vec2 rotate_amt = {0, 0}; //

    // find the first pad out of the deadzone 8022ba58
    for (int i = 0; i < 4; i++)
    {

        HSD_Pad *this_pad = &stc_engine_pads[i];

        // check if out of deadzone, this code sucks
        if (this_pad->fsubstickX >= CAM_STICKDEADZONE)
            rotate_amt.X = (this_pad->fsubstickX - CAM_STICKDEADZONE) / (1 - CAM_STICKDEADZONE) * CAM_MAXROTATEDEG;
        else if (this_pad->fsubstickX <= -CAM_STICKDEADZONE)
            rotate_amt.X = (this_pad->fsubstickX - -CAM_STICKDEADZONE) / (1 - CAM_STICKDEADZONE) * CAM_MAXROTATEDEG;
        if (this_pad->fsubstickY >= CAM_STICKDEADZONE)
            rotate_amt.Y = (this_pad->fsubstickY - CAM_STICKDEADZONE) / (1 - CAM_STICKDEADZONE) * CAM_MAXROTATEDEG;
        else if (this_pad->fsubstickY <= -CAM_STICKDEADZONE)
            rotate_amt.Y = (this_pad->fsubstickY - -CAM_STICKDEADZONE) / (1 - CAM_STICKDEADZONE) * CAM_MAXROTATEDEG;

        if (rotate_amt.X != 0 || rotate_amt.Y != 0)
            break;
    }

    Vec3 cobj_eye;             // 72
    Vec3 cobj_interest;        // 24
    Vec3 cross;                // 60
    Vec3 identity = {0, 1, 0}; // 48
    Mtx m;                     // 84
    Vec3 final_eye;            // 36

    // rotate camera 8022bbe0
    COBJ_Init(gobj->hsd_object, stc_cobj_desc);
    COBJ_GetEyeVector(gobj->hsd_object, &cobj_eye);
    COBJ_GetInterest(gobj->hsd_object, &cobj_interest);
    VECCrossProduct(&identity, &cobj_eye, &cross);
    VECNormalize(&cross, &cross);

    // Y axis
    C_MTXRotAxisRad(&m, &cross, rotate_amt.Y * M_1DEGREE);
    MTXMultVec(&m, &cobj_eye, &cobj_eye);

    // X axis
    C_MTXRotAxisRad(&m, &identity, rotate_amt.X * M_1DEGREE);
    MTXMultVec(&m, &cobj_eye, &cobj_eye);

    // scale mtx
    VECScale(&cobj_eye,
             &cobj_eye,
             COBJ_GetEyeDistance(gobj->hsd_object));

    VECSubtract(&cobj_interest, &cobj_eye, &final_eye);
    CObj_SetEyePosition(gobj->hsd_object, &final_eye);

    return;
}

static JOBJSet *stc_bg_panel_jobjset = 0;
static JOBJ *stc_bg_panel_jobj = 0;
GOBJ *CitySettings_BGCreate()
{
    // create background
    JOBJSet **bg = Archive_GetPublicAddress(settings_archive, "ScMenSelruleBgCt_scene_models");
    GOBJ *bg_gobj = JObj_LoadSet_SetPri(0, bg[0], 0, 0, 30, MENUGX_1, 1, CitySettings_BGThink, 0);

    // grab isolated animations for the bottom panel of the background jobj (moves them up and down)
    stc_bg_panel_jobjset = Archive_GetPublicAddress(settings_archive, "ScMenSelDescription_model_set");
    stc_bg_panel_jobj = JObj_GetIndex(bg_gobj->hsd_object, 22); // save ptr to the bottom panel jobj

    return bg_gobj;
}
void CitySettings_BGThink(GOBJ *g)
{

    JObj_AnimAll(g->hsd_object);

    // update text pos
    if (settings_data.description_text)
    {
        static Vec3 bg_panel_text_offset = {-21, -0.4 - 16.4, 0};
        static Vec2 bg_panel_scale_mult = {1.2, 1.2};

        Text_CopyJointPosition(settings_data.description_text, stc_bg_panel_jobj);

        settings_data.description_text->trans.X += bg_panel_text_offset.X;
        settings_data.description_text->trans.Y -= bg_panel_text_offset.Y;
        settings_data.description_text->trans.Z += bg_panel_text_offset.Z;

        settings_data.description_text->viewport_scale.X *= bg_panel_scale_mult.X;
        settings_data.description_text->viewport_scale.Y *= bg_panel_scale_mult.Y;
    }

    // update menu names
    for (int i = 0; i < GetElementsIn(settings_data.menu.name); i++)
    {
        if (settings_data.menu.name[i].text)
        {
            // animate joint
            JObj_AnimAll(settings_data.menu.name[i].j);

            // update text position
            Text_CopyJointPosition(settings_data.menu.name[i].text, JObj_GetIndex(settings_data.menu.name[i].j, 2));
        }
    }

    // when animation ends, play the next one and loop it
    if (!JObj_CheckAObjPlaying(g->hsd_object))
    {
        JOBJSet **bg = Archive_GetPublicAddress(settings_archive, "ScMenSelruleBgCt_scene_models");

        JObj_AddSetAnim(g->hsd_object, 1, bg[0], 0, 1);
        JObj_SetAllAOBJLoopByFlags(g->hsd_object, 0xFFFF);
        settings_data.is_intro_anim = 0;

        // remove animation from bottom panel, will be moving this independently
        JObj_RemoveAnimByFlags(stc_bg_panel_jobj, JObj_ANIM);
    }

    return;
}

void CitySettings_UpdateVanillaSettings()
{
    // apply settings from vanilla game
    GameData *gd = Gm_GetGameData();
    gd->city.time_seconds = stc_city_save->settings[CITYSETTING_SAVE_TIME] * 60;
    gd->city.game_tempo = stc_city_save->settings[CITYSETTING_SAVE_TEMPO] + 1;
    gd->city.menu_stadium_selection = stc_city_save->settings[CITYSETTING_SAVE_STADIUM];
    gd->city.events_enable = (stc_city_save->settings[CITYSETTING_SAVE_EVENTFREQ] == 0) ? 0 : 1;

    return;
}
void CitySettings_CopyToSave(CitySettingsMenuDesc *desc)
{
    // recursively iterate through all menus and copy their option values to the save file

    if (!desc)
        return;

    for (int opt_idx = 0; opt_idx < desc->generic.opt_num; opt_idx++)
    {
        // process each option
        switch (desc->generic.options[opt_idx].kind)
        {
        case (CITYSETTING_OPTKIND_NUM):
        {
            stc_city_save->settings[desc->generic.options[opt_idx].u.number.save_idx] = desc->generic.options[opt_idx].u.number.cur_selection;
            break;
        }
        case (CITYSETTING_OPTKIND_VALUE):
        {
            stc_city_save->settings[desc->generic.options[opt_idx].u.value.save_idx] = desc->generic.options[opt_idx].u.value.cur_selection;
            break;
        }
        case (CITYSETTING_OPTKIND_MENU):
        {
            if (desc->generic.options[opt_idx].u.menu.desc && desc->generic.options[opt_idx].u.menu.desc->kind == CITYSETTING_MENUKIND_GENERIC)
                CitySettings_CopyToSave(desc->generic.options[opt_idx].u.menu.desc);
            break;
        }
        }
    }

    return;
}
void CitySettings_CopyFromSave(CitySettingsMenuDesc *desc)
{
    // recursively iterate through all menus and init their option values from the save file
    if (!desc)
        return;

    for (int opt_idx = 0; opt_idx < desc->generic.opt_num; opt_idx++)
    {
        // process each option
        switch (desc->generic.options[opt_idx].kind)
        {
        case (CITYSETTING_OPTKIND_NUM):
        {
            // ensure value is within bounds
            if (stc_city_save->settings[desc->generic.options[opt_idx].u.number.save_idx] >= desc->generic.options[opt_idx].u.number.min &&
                stc_city_save->settings[desc->generic.options[opt_idx].u.number.save_idx] < desc->generic.options[opt_idx].u.number.max)
                desc->generic.options[opt_idx].u.number.cur_selection = stc_city_save->settings[desc->generic.options[opt_idx].u.number.save_idx];

            break;
        }
        case (CITYSETTING_OPTKIND_VALUE):
        {
            // ensure value is within bounds
            if (stc_city_save->settings[desc->generic.options[opt_idx].u.value.save_idx] >= 0 &&
                stc_city_save->settings[desc->generic.options[opt_idx].u.value.save_idx] < desc->generic.options[opt_idx].u.value.num)
                desc->generic.options[opt_idx].u.value.cur_selection = stc_city_save->settings[desc->generic.options[opt_idx].u.value.save_idx];

            break;
        }
        case (CITYSETTING_OPTKIND_MENU):
        {
            if (desc->generic.options[opt_idx].u.menu.desc && desc->generic.options[opt_idx].u.menu.desc->kind == CITYSETTING_MENUKIND_GENERIC)
                CitySettings_CopyFromSave(desc->generic.options[opt_idx].u.menu.desc);

            break;
        }
        }
    }

    return;
}

void CitySettings_UpdateDescription(char *s)
{
#pragma pack(push, 1) // Align to 1-byte boundaries
    struct TextHeader
    {
        u8 opcode;
        int x1;
        u8 x5;
        u8 color_r;
        u8 color_g;
        u8 color_b;
        u8 x9;
        u8 xa;
        u8 xb;
        u8 xc;
        u8 xd;
        u8 text_data[];
    };
#pragma pack(pop)

    Text *description_text = settings_data.description_text;

    // build text header data
    struct TextHeader header;
    header.opcode = 7;
    header.x1 = 0;
    header.x5 = 12;
    header.color_r = description_text->color.r;
    header.color_g = description_text->color.g;
    header.color_b = description_text->color.b;
    header.x9 = 14;
    header.xa = 1;
    header.xb = 0;
    header.xc = 1;
    header.xd = 0;

    // build text data
    char out[400];
    int new_size = Text_ConvertASCIIRewrite(out, s) + sizeof(header) + 0x2;

    // check to resize
    if (new_size > description_text->allocInfo->size)
    {
        // free old
        Text_Free(description_text->text_start);

        // alloc new
        int alloc_size = OSRoundUp32B(new_size + sizeof(header) + 0x3);
        description_text->text_start = Text_Alloc(alloc_size);
        description_text->allocInfo->size = alloc_size;
        description_text->allocInfo->x4 = (void *)description_text->text_start;
    }

    // copy data
    memcpy(description_text->text_start, &header, sizeof(header));
    memcpy(description_text->text_start + sizeof(header), &out, new_size);
    description_text->text_start[sizeof(header) + new_size + 0x0] = 15;
    description_text->text_start[sizeof(header) + new_size + 0x1] = 13;
    description_text->text_start[sizeof(header) + new_size + 0x2] = 0;
}
void CitySettings_HideDescription()
{
    JObj_AddSetAnim(stc_bg_panel_jobj, 0, stc_bg_panel_jobjset, 0, 1);
}
void CitySettings_ShowDescription()
{
    JObj_AddSetAnim(stc_bg_panel_jobj, 1, stc_bg_panel_jobjset, 0, 1);
}
HSD_Archive *CitySettings_GetCustomMenuArchive()
{
    return settings_archive;
}

void Text_CopyJointPosition(Text *t, JOBJ *j)
{
    Vec3 trans, scale;

    // update matrix
    JObj_SetupMtxSub(j);

    // get values
    HSD_MtxGetTranslate(&j->rotMtx, &trans); // get transform
    HSD_MtxGetScale(&j->rotMtx, &scale);     // get scale

    t->trans = (Vec3){trans.X, -trans.Y, trans.Z};
    t->viewport_scale = (Vec2){0.043 * scale.X, 0.05 * scale.Y};
    (j->flags & JOBJ_HIDDEN) ? (t->hidden = 1) : (t->hidden = 0);

    // copy alpha
    if (j->dobj)
    {
        // t->color.r = j->dobj->mobj->mat->diffuse.r;
        // t->color.g = j->dobj->mobj->mat->diffuse.g;
        // t->color.b = j->dobj->mobj->mat->diffuse.b;
        t->color.a = j->dobj->mobj->mat->alpha * 255.0;
    }
}
int Text_ConvertASCIIRewrite(u8 *out, char *s)
{

    // orig function goes,
    // ascii -> shift_jis by addition
    // find index of shift_jis char in ascii_to_shiftjis_mapping

    u8 *cur = out;

    while (s[0])
    {
        // numbers
        if (s[0] >= '0' && s[0] <= '9')
        {
            cur[0] = 11;
            *(u16 *)&cur[1] = 0x2000 + (s[0] - '0');
            cur += 3;
        }
        // capitals
        else if (s[0] >= 'A' && s[0] <= 'Z')
        {
            cur[0] = 11;
            *(u16 *)&cur[1] = 0x200a + (s[0] - 'A');

            cur += 3;
        }
        // lower case
        else if (s[0] >= 'a' && s[0] <= 'z')
        {

            cur[0] = 11;
            *(u16 *)&cur[1] = 0x2024 + (s[0] - 'a');

            cur += 3;
        }
        else
        {
            // symbol lookup
            struct ASCIISymbolLookup
            {
                u8 ascii;
                u16 text_code;
            };

            static struct ASCIISymbolLookup symbol_lookup[] = {
                {
                    .ascii = ' ',
                    .text_code = 0x20e3,
                },
                {
                    .ascii = '!',
                    .text_code = 0x20ec,
                },
                {
                    .ascii = '"',
                    .text_code = 0x20f4,
                },
                {
                    .ascii = '#',
                    .text_code = 0x2106,
                },
                {
                    .ascii = '$',
                    .text_code = 0x2104,
                },
                {
                    .ascii = '%',
                    .text_code = 0x2105,
                },
                {
                    .ascii = '&',
                    .text_code = 0x2107,
                },
                {
                    .ascii = '(',
                    .text_code = 0x20f5,
                },
                {
                    .ascii = ')',
                    .text_code = 0x20f6,
                },
                {
                    .ascii = '*',
                    .text_code = 0x2108,
                },
                {
                    .ascii = '+',
                    .text_code = 0x20fd,
                },
                {
                    .ascii = ',',
                    .text_code = 0x20e6,
                },
                {
                    .ascii = '-',
                    .text_code = 0x20fe,
                },
                {
                    .ascii = '.',
                    .text_code = 0x20e7,
                },
                {
                    .ascii = '/',
                    .text_code = 0x20f0,
                },
                {
                    .ascii = ':',
                    .text_code = 0x20e9,
                },
                {
                    .ascii = ';',
                    .text_code = 0x20ea,
                },
                {
                    .ascii = '=',
                    .text_code = 0x2100,
                },
                {
                    .ascii = '?',
                    .text_code = 0x20eb,
                },
                {
                    .ascii = '@',
                    .text_code = 0x2109,
                },
                {
                    .ascii = '_',
                    .text_code = 0x20ee,
                },
            };

            // find symbol
            u16 text_code = 0xFFFF;
            for (int i = 0; i < GetElementsIn(symbol_lookup); i++)
            {
                if (symbol_lookup[i].ascii == s[0])
                {
                    text_code = symbol_lookup[i].text_code;
                    break;
                }
            }

            // write out symbol
            if (text_code != 0xFFFF)
            {
                cur[0] = 11;
                *(u16 *)&cur[1] = text_code;

                cur += 3;
            }
        }

        s++;
    }

    cur[0] = 0;

    return cur - out;
}
