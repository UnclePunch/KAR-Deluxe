#ifndef MOD_H_WIDEHUD
#define MOD_H_WIDEHUD

typedef struct WideAdjustData
{
    float *ptr;
    float orig;
} WideAdjustData;

void HUDAdjust_Init();
void Wide_CreateDebugHUDGObj();
void Wide_AdjustConstants();

#endif