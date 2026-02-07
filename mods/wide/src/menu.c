#include "text.h"
#include "os.h"
#include "hsd.h"
#include "hud.h"

#include "wide.h"
#include "menu.h"

#include "code_patch/code_patch.h"

void Menu_ShiftText(Text *t)
{
    float aspect_mult = Wide_GetAspectMult();
    float shift_amt = -((640.f * aspect_mult) - 640.f) / 2;
    
    t->trans.X += shift_amt;
}

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
    
}