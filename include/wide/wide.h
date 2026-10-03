#ifndef MOD_H_WIDEEXPORT
#define MOD_H_WIDEEXPORT

#include "datatypes.h"
#include "obj.h"

#define WIDE_VERSION_MAJOR 1
#define WIDE_VERSION_MINOR 0

// this is the width of the original perspective screen camera
#define ORIG_WIDTH (60.68)
#define ORIG_HEIGHT (48.36)
#define ORIG_ASPECT (1.255375)

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
    float (*Wide_GetAspectMult)();
} WideExport;

#endif