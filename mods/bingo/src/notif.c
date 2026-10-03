#include "obj.h"
#include "hud.h"

#include "bingo.h"
#include "card.h"   // needed for SIS index
#include "notif.h"

#include "wide/wide.h"

extern JOBJSet *notif_set;
extern WideExport *g_wide_export;

extern int bingo_text_canvas_idx;

GOBJ *BingoNotif_Create(BingoGoal *goal, int progress, int ply)
{
    // find an existing notif for this goal
    GOBJ *notif_g = 0;
    for (GOBJ *g = (*stc_gobj_lookup)[GAMEPLINK_HUD]; g;)
    {
        GOBJ *next = g->next;

        if (g->entity_class == 100)
        {
            BingoNotifData *gp = g->userdata;

            // if a notif for this exact goal is already onscreen, use it
            if (gp->goal == goal)
            {
                notif_g = g;
                break;
            }

            // if this notif was just created
            else if (gp->goal->kind == goal->kind)
            {
                if (gp->timer == 0)
                {
                    if (gp->goal->kind == goal->kind)
                    {
                        // when there are similar goals, prioritize the one with a lower number requirement
                        if (goal->num < gp->goal->num)
                            GObj_Destroy(g);    // keep only the lower number goal notif
                        else
                            return 0;           // dont make a new notif if a lower num one exists
                    }
                    // refer to the priority
                    else if (gp->goal->kind > goal->kind)
                        GObj_Destroy(g);        // destroy this notif and create a new one
                    else
                        return 0;               // dont make a new notif if its a lower priority
                }
                else if (goal->num > gp->goal->num && gp->progress < gp->goal->num)
                    return 0;           // skip new notif if the new one has a lower requirement and isnt completed
                else
                    GObj_Destroy(g);    // destroy notif with a higher requirement

            }
            else
                GObj_Destroy(g);    // out with the old, in with the new
        }

        g = next;
    }

    // create new notif
    if (!notif_g)
    {
        notif_g = GOBJ_EZCreator(100, GAMEPLINK_HUD, 0,
                            sizeof(BingoNotifData), BingoNotif_Destroy,
                            HSD_OBJKIND_JOBJ, notif_set->jobj, 
                            BingoNotif_Think, 22, 
                            BingoNotif_GX, GAMEGX_HUD, 1);


        // wide adjust
        if (g_wide_export)
            g_wide_export->HUDAdjust_Element(notif_g, 0, 0, WIDEALIGN_LEFT, HEIGHTALIGN_CENTER);

        BingoNotifData *notif_data = notif_g->userdata;
        notif_data->goal = goal;
        notif_data->ply = ply;
        notif_data->timer = 0;

        JOBJ *notif_j = notif_g->hsd_object;
        JObj_AddSetAnim(notif_j, 0, notif_set, 0, 0);
        JObj_SetFrameAndRate(notif_j, 0, 0);

        // start movement anim
        JObj_SetFrameAndRate(JObj_GetIndex(notif_j, BINGO_NOTIF_JOINT_MOVE), 0, 1);
        
        // set goal icon
        Bingo_SetIconForGoal(goal, notif_j, BINGO_NOTIF_JOINT_SINGLE_ICON, 
                                            BINGO_NOTIF_JOINT_MULTI_ICON,
                                            BINGO_NOTIF_JOINT_SINGLE_DIGIT,
                                            BINGO_NOTIF_JOINT_DOUBLE_DIGIT);

        char s[64];
        Bingo_GetDescriptionForGoal(goal, s);
        // sprintf("Progress: %d \x81\x5E %d.", progress, goal->num);
        
        // add text
        Text *t = Text_CreateText(BINGO_SIS_INDEX, bingo_text_canvas_idx);
        t->gobj->gx_cb = 0; // remove gx callback and render from notif gx
        // t->viewport_color = (GXColor){255, 0, 0, 128};
        t->color = (GXColor){255, 255, 255, 255};
        t->kerning = 1;
        t->align = 0;
        t->viewport_scale = (Vec2){0.045, 0.055};
        t->use_aspect = 1;
        t->aspect = (Vec2){350, 64};
        Text_AddSubtext(t, 0, 0, s);
        notif_data->t = t;
    }
    else
    {
        // refresh timer
        BingoNotifData *notif_data = notif_g->userdata;
        notif_data->timer = 0;
    }

    BingoNotifData *notif_data = notif_g->userdata;

    // update progress
    Bingo_UpdateIconProgress(goal, progress, notif_g->hsd_object, BINGO_NOTIF_JOINT_BACKGROUND_FILL);
    notif_data->progress = progress;

    return notif_g;
}
void BingoNotif_Destroy(BingoNotifData *gp)
{
    Text_Destroy(gp->t);
    HSD_Free(gp);
}
void BingoNotif_Think(GOBJ *g)
{
    BingoNotifData *gp = g->userdata;
    JOBJ *j = g->hsd_object;

    JObj_AnimAll(j);

    if (!JObj_CheckAObjPlaying(JObj_GetIndex(j, BINGO_NOTIF_JOINT_MOVE)))
    {
        if (++gp->timer > BINGO_NOTIF_PARAM_TIMER)
        {
            GObj_Destroy(g);
            return;
        }
    }

    // update text position
    Vec3 text_pos;
    JObj_GetChildPosition(j, BINGO_NOTIF_JOINT_TEXT, &text_pos);
    gp->t->trans.X = text_pos.X;
    gp->t->trans.Y = -text_pos.Y;

}
void BingoNotif_GX(GOBJ *g, int pass)
{
    BingoNotifData *gp = g->userdata;

    // dont render when bingo card is up or game is paused
    if (*g_hud_is_hidden)
        return;

    JObj_GX(g, pass);
    Text_GX(gp->t->gobj, pass);
}
