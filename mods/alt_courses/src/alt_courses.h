#ifndef MOTIONSETTINGS_MOTION_H
#define MOTIONSETTINGS_MOTION_H

#include "stage.h"

#include "hoshi/settings.h"

#define COURSE_NUM 22

typedef struct AltCourse
{
    StageKind kind : 16;
    u32 alt_num : 16;
    char *orig_file_path;
    char alt_file_path[64];
    u16 *alt_entrynums;
} AltCourse;

void AltCourses_Init();
OptionDesc *AltCourses_CreateMenu();
void AltCourses_ApplyAltStages();

#endif