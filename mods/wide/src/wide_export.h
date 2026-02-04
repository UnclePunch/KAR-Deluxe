#ifndef MOD_H_WIDEEXPORT
#define MOD_H_WIDEEXPORT

#include "gamehud.h"
#include "wide.h"

typedef struct WideExport
{
    void (*HUDAdjust_Element)(GOBJ *g, int joint_index, int is_ply_element, WideAlign align);
} WideExport;

#endif