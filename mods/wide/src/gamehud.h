#ifndef MOD_H_WIDEHUD
#define MOD_H_WIDEHUD

#include "obj.h"
typedef struct WideAdjustData
{
    float *ptr;
    float orig;
} WideAdjustData;

typedef enum WideHUDKind
{
    WIDEHUD_NONE,
    WIDEHUD_GAME,
    WIDEHUD_MNRADAR,
} WideHUDKind;

typedef enum WideAlign
{
    WIDEALIGN_LEFT,
    WIDEALIGN_RIGHT,
    WIDEALIGN_CENTER,
} WideAlign;

typedef enum HeightAlign
{
    HEIGHTALIGN_BOTTOM,
    HEIGHTALIGN_TOP,
    HEIGHTALIGN_CENTER,
} HeightAlign;

void HUDAdjust_Init();
void Wide_CreateDebugHUDGObj();
void Wide_On3DLoadStart();
void HUDAdjust_Element(GOBJ *g, int joint_index, WideHUDKind hud_kind, WideAlign x_align, HeightAlign y_align);
void HUDAdjust_Camera(COBJ *c);
#endif