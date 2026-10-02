#ifndef MOD_H_WIDEEXPORT
#define MOD_H_WIDEEXPORT

#include "datatypes.h"
#include "obj.h"

#define WIDE_VERSION_MAJOR 1
#define WIDE_VERSION_MINOR 0

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

typedef struct WideExport
{
    void (*HUDAdjust_Element)(GOBJ *g, int joint_index, int is_ply_element, WideAlign x_align, HeightAlign y_align);
    void (*HUDAdjust_Camera)(COBJ *c);
} WideExport;

#endif