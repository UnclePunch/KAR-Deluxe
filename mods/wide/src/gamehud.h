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
} WideAlign;

void HUDAdjust_Init();
void Wide_CreateDebugHUDGObj();
void Wide_AdjustConstants();
void HUDAdjust_Element(GOBJ *g, int joint_index, int is_ply_element, WideAlign align);
#endif