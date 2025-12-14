
#include "text.h"
#include "os.h"
#include "hsd.h"
#include "preload.h"
#include "game.h"
#include "scene.h"
#include "inline.h"
#include "audio.h"
#include "scene.h"

#include "hoshi/func.h"

#include <string.h>

#include "credits.h"

int end_timer = 0;
int stc_canvas_idx;
MajorSceneDesc major_desc = {
    .major_id = -1,
    .next_major_id = 0,
    .initial_minor_id = -1,
    .cb_Enter = CreditsMajor_Enter,
    .cb_ExitMinor = CreditsMajor_Exit,
};
MinorSceneDesc minor_scene = {
    .idx = -1,
    .x1 = -1,
    .cb_Load = CreditsMinor_Enter,
    .cb_Exit = CreditsMinor_Exit,
    .cb_ThinkPreGObjProc = CreditsMinor_Think,
    .preload_kind = 0, // 4 will preserve menu file preloads
};

MajorKind Credits_Init()
{
    minor_scene.idx = Hoshi_InstallMinorScene(&minor_scene);
    major_desc.initial_minor_id = minor_scene.idx;
    major_desc.major_id = Hoshi_InstallMajorScene(&major_desc);

    return major_desc.major_id;
}

void CreditsMajor_Enter()
{
    return;
}
void CreditsMajor_Exit()
{
    return;
}

GOBJ *stc_scroll_gobj;
void CreditsMinor_Enter()
{
    // init text
    stc_canvas_idx = Text_CreateCanvas(0, 0, 0, 0, 0, 0, 0, 0);
    Text_LoadSisFile(0, "SisModCredits.dat", "SIS_ModCreditsData");

    stc_scroll_gobj = Credits_ScrollCreate();

    BGM_Play(BGM_LEGENDARYAIRRIDEMACHINE);
    return;
}
void CreditsMinor_Exit(void *data)
{
    if (Scene_GetDirection() == PAD_BUTTON_B)
    {
        GameData *gd = Gm_GetGameData();
        gd->main_menu.top_menu = MAINMENU_TOPMENU_OPTIONS;
        gd->main_menu.is_in_submenu = 1;
        gd->main_menu.cursor_val[0] = 4;
        gd->main_menu.cursor_val[1] = 0;

        Scene_SetNextMajor(MJRKIND_MENU);
        Scene_ExitMajor();
    }

    return;
}
void CreditsMinor_Think(void *data)
{
    if (Pad_GetDown(20) & (PAD_BUTTON_A | PAD_BUTTON_B | PAD_BUTTON_START))
    {
        // destroy gobj
        GObj_Destroy(stc_scroll_gobj);

        BGM_Stop();
        SFX_Play(FGMMENU_CS_KETTEI);
        Scene_SetDirection(PAD_BUTTON_B);
        Scene_ExitMinor();
    }
    else if (end_timer > 0)
    {
        end_timer--;

        if (end_timer <= 0)
        {
            Scene_SetDirection(PAD_BUTTON_B);
            Scene_ExitMinor();
        }
    }

    return;
}

GOBJ *Credits_ScrollCreate()
{
    // create credit scroll gobj
    GOBJ *g = GOBJ_EZCreator(0, 0, 0,
                             sizeof(ScrollData), Credits_ScrollDestroy,
                             HSD_OBJKIND_NONE, 0,
                             Credits_ScrollThink, 0,
                             0, 0, 0);

    ScrollData *gp = g->userdata;
    gp->credit_idx = 0;
    gp->is_end = 0;

    return g;
}
void Credits_ScrollThink(GOBJ *g)
{
    ScrollData *gp = g->userdata;

    // next text entry in text array
    Text *t = gp->text[0];

    if (!t || t->trans.Y < 350)
    {
        if (gp->credit_idx < CREDIT_NUM)
        {
            // create next credit
            Text *t;
            t = Text_CreateTextManual(0, stc_canvas_idx, 0, 480, 10, 489.6, 38.4);
            t->viewport_scale = (Vec2){1.3, 1.3};
            Text_SetSisText(t, 2 + gp->credit_idx);
            Text_FinalizeSisText(t, 2 + gp->credit_idx);

            // remove last if exists
            if (gp->text[TEXT_ARRAY_SIZE])
                Text_Destroy(gp->text[TEXT_ARRAY_SIZE]);

            // shift array
            for (int i = TEXT_ARRAY_SIZE - 1; i > 0; i--)
                gp->text[i] = gp->text[i - 1];

            gp->text[0] = t;

            gp->credit_idx++;
        }
    }

    // get scroll speed
    float trigger_val = 0;
    for (int i = 0; i < 4; i++)
    {
        if (stc_engine_pads[i].ftriggerLeft > 0)
            trigger_val += stc_engine_pads[i].ftriggerLeft;
        if (stc_engine_pads[i].ftriggerRight > 0)
            trigger_val += stc_engine_pads[i].ftriggerRight;
    }
    float scroll_speed = SCROLL_SPEED + (SCROLL_SPEED * trigger_val * 2);

    // move all up
    for (int i = 0; i < TEXT_ARRAY_SIZE; i++)
    {
        Text *t = gp->text[i];

        if (!t)
            continue;

        if (gp->credit_idx == (CREDIT_NUM) && i == 0)
        {
            if (!gp->is_end)
            {
                if (t->trans.Y > 0)
                    t->trans.Y -= scroll_speed;
                else
                {
                    t->trans.Y = 0;
                    gp->is_end = 1;
                    end_timer = 60;
                }
            }
        }
        else
            t->trans.Y -= scroll_speed;
    }
}
void Credits_ScrollDestroy(ScrollData *gp)
{
    //
    for (int i = 0; i < TEXT_ARRAY_SIZE; i++)
        Text_Destroy(gp->text[i]);

    HSD_Free(gp);
}
