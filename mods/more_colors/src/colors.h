#ifndef MORECOLORS_H
#define MORECOLORS_H

typedef struct UIColor
{
    GXColor main;
    GXColor dark;
    GXColor bright;
    GXColor hud;
} UIColor;

typedef struct ElementColorDesc
{
    s16 joint_idx;
    s16 dobj_idx;
    GXColor *col;
} ElementColorDesc;

void Select_SetUIColor(JOBJ *j, int joint_idx, int dobj_idx, GXColor color);
int Select_ColorToAnim(int color_idx);
void Select_UpdatePlyBoard(JOBJ *j, int p_kind, int anim_frame, ElementColorDesc *desc);
UIColor *Colors_GetUIColors();
void Colors_ApplyPatches();
#endif