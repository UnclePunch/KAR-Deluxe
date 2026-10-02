#ifndef MOD_H_WIDEHUD
#define MOD_H_WIDEHUD

#include "obj.h"
typedef struct WideAdjustData
{
    float *ptr;
    float orig;
} WideAdjustData;

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
void Wide_AdjustConstants();
void HUDAdjust_Element(GOBJ *g, int joint_index, int is_ply_element, WideAlign x_align, HeightAlign y_align);
void HUDAdjust_Camera(COBJ *c);
#endif