#include "text.h"
#include "os.h"
#include "hsd.h"
#include "hud.h"
#include "inline.h"

#include "wide.h"
#include "menu.h"

#include "code_patch/code_patch.h"

void JOBJ_SetAlpha(JOBJ *j, void *arg)
{
    float alpha = *(float *)arg;
    for (DOBJ *d = j->dobj; d; d = d->next)
    {
        if (d->mobj && d->mobj->mat)
            d->mobj->mat->alpha = alpha;
    }
}

void Menu_ShiftText(Text *t)
{
    float aspect_mult = Wide_GetAspectMult();
    float shift_amt = -((640.f * aspect_mult) - 640.f) / 2;
    
    t->trans.X += shift_amt;
}

// Main Menu Cursor1 Edits
void MainMenu_Cursor1Think(JOBJ *j, MenuElementData *gp)
{
    // runs every frame per cursor1 gobj
    // here we operate on some custom variables to fade the cursor1 jobjs in and out
    // when they are moving in and out of the center

    // get our custom variables
    u8 *cd = (u8 *)gp;
    u8 *anim_direction = &cd[0x12];
    u8 *anim_timer = &cd[0x13];

    ScMenuCommon *mp = Gm_GetMenuData();
    MenuElementData *pos_data = mp->main.ScMenCursorpos_gobj[gp->cursor1.cursor_pos_id]->userdata;
    
    // only if the cursor1's are moving
    if (*anim_direction != 0)
    {
        (*anim_timer)--;

        // determine divisor for out target alpha value (fading in or out)
        float divisor;
        if (*anim_direction == 1)
            divisor = (float)(4 - *anim_timer);
        else
            divisor = (float)*anim_timer;

        // adjust transparency for all meshes in the cursor1 jobj
        float alpha = divisor / 4.0f;
        JObj_ForEachJoint(j, JOBJ_SetAlpha, &alpha);

        // null anim_direction when animation ends
        if ((*anim_timer) == 0)
            *anim_direction = 0;
    }
}
CODEPATCH_HOOKCREATE(0x80149054, "mr 3, 30\t\n" "mr 4, 31\t\n", MainMenu_Cursor1Think, "", 0)
void MainMenu_OnCursor1Anim(int cursor_pos_idx)
{
    // runs when a cursor position animation is entered
    // here we initialzie 2 custom variables in the cursor1 gobj
    //      anim_direction  at 0x12
    //      anim_timer      at 0x13

    ScMenuCommon *md = Gm_GetMenuData();    
    MenuElementData *pd = md->main.ScMenCursorpos_gobj[cursor_pos_idx]->userdata;

    for (int i = 0; i < 6; i++)
    {
        MenuElementData *cd = md->main.ScMenCursor1_gobj[cursor_pos_idx][i]->userdata;

        u8 *arr = (u8 *)cd;
        arr[0x12] = pd->cursor1_pos.anim_direction;
        arr[0x13] = pd->cursor1_pos.anim_timer;
    }
}
CODEPATCH_HOOKCREATE(0x80148be8, "mr 3, 29\t\n", MainMenu_OnCursor1Anim, "", 0)
void MainMenu_OnCursor1Create(GOBJ *g)
{
    // init all cursor1 objects to hidden when creating them
    MenuElementData *cd = g->userdata;

    cd->is_visible = 0;
}
CODEPATCH_HOOKCREATE(0x80149314, "mr 3, 29\t\n", MainMenu_OnCursor1Create, "", 0)


void SelPly_AdjustText(int ply)
{
    ScMenuCommon *md = Gm_GetMenuData();

    Menu_ShiftText(md->text.ply_machine_description[ply].x0);
    Menu_ShiftText(md->text.ply_machine_description[ply].x4);
}
CODEPATCH_HOOKCREATE(0x80153d88, "mr 3, 30\t\n", SelPly_AdjustText, "", 0)
CODEPATCH_HOOKCREATE(0x8015e79c, "mr 3, 30\t\n", SelPly_AdjustText, "", 0)

// air ride results
CODEPATCH_HOOKCREATE(0x8013f89c, "mr 3, 31\t\n", Menu_ShiftText, "", 0)
CODEPATCH_HOOKCREATE(0x801404f0, "mr 3, 31\t\n", Menu_ShiftText, "", 0)
CODEPATCH_HOOKCREATE(0x80140084, "mr 3, 31\t\n", Menu_ShiftText, "", 0)

// air ride results splitscreen
CODEPATCH_HOOKCREATE(0x801426a0, "mr 3, 31\t\n", Menu_ShiftText, "", 0)
CODEPATCH_HOOKCREATE(0x80142e94, "mr 3, 31\t\n", Menu_ShiftText, "", 0)

// city results
CODEPATCH_HOOKCREATE(0x801438f0, "mr 3, 31\t\n", Menu_ShiftText, "", 0)
CODEPATCH_HOOKCREATE(0x80144cbc, "mr 3, 30\t\n", Menu_ShiftText, "", 0)
CODEPATCH_HOOKCREATE(0x80145544, "mr 3, 31\t\n", Menu_ShiftText, "", 0)

// city results splitscreen
CODEPATCH_HOOKCREATE(0x80143f90, "mr 3, 27\t\n", Menu_ShiftText, "", 0)
CODEPATCH_HOOKCREATE(0x8014515c, "mr 3, 30\t\n", Menu_ShiftText, "", 0)
CODEPATCH_HOOKCREATE(0x801444e0, "mr 3, 31\t\n", Menu_ShiftText, "", 0) // points
CODEPATCH_HOOKCREATE(0x80145c58, "mr 3, 26\t\n", Menu_ShiftText, "", 0)

void MainMenuAdjust_Init()
{
    CODEPATCH_HOOKAPPLY(0x80153d88);
    CODEPATCH_HOOKAPPLY(0x8015e79c);

    
    CODEPATCH_HOOKAPPLY(0x8013f89c);
    CODEPATCH_HOOKAPPLY(0x801404f0);
    CODEPATCH_HOOKAPPLY(0x80140084);

    CODEPATCH_HOOKAPPLY(0x801426a0);
    CODEPATCH_HOOKAPPLY(0x80142e94);


    CODEPATCH_HOOKAPPLY(0x801438f0);
    CODEPATCH_HOOKAPPLY(0x80144cbc);
    CODEPATCH_HOOKAPPLY(0x80145544);
    
    CODEPATCH_HOOKAPPLY(0x80143f90);
    CODEPATCH_HOOKAPPLY(0x8014515c);
    CODEPATCH_HOOKAPPLY(0x801444e0);
    CODEPATCH_HOOKAPPLY(0x80145c58);
    
    CODEPATCH_HOOKAPPLY(0x80149054);
    CODEPATCH_HOOKAPPLY(0x80148be8);
    CODEPATCH_HOOKAPPLY(0x80149314);
}