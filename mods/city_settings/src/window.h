#ifndef CITYWINDOW_H
#define CITYWINDOW_H

#include <stdarg.h>

#include "obj.h"
#include "citysettings.h"

typedef struct CitySettingsWindowAssets
{
    JOBJSet *window_set;
} CitySettingsWindowAssets;

typedef struct CitySettingsWindowData
{
    Text *text;
    int cursor;
    void (*cb_confirm)();
} CitySettingsWindowData;

void Window_Init(HSD_Archive *custom_archive);
GOBJ *Window_Create(char *string, void (*cb_confirm)());
void Window_Update(GOBJ *g);
CitySettingsMenuAction Window_Input(GOBJ *g);
void Window_Destroy(CitySettingsWindowData *gp);

#endif