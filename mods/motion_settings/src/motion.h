#ifndef MOTIONSETTINGS_MOTION_H
#define MOTIONSETTINGS_MOTION_H

void Motion_Init();
GOBJ *Motion_BorderCreate();
void Motion_BorderGX(GOBJ *g, int pass);


void Motion_DrawBoxWithLines(Vec3 *tl, Vec3 *br, int width, GXColor *color);
void Motion_DrawLine(float x1, float y1, float x2, float y2, int width, GXColor *color);

#endif