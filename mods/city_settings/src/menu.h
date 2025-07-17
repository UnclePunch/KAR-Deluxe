#ifndef CITYMENU_H
#define CITYMENU_H

#include <stdarg.h>

#include "obj.h"
#include "citysettings.h"
typedef struct CitySettingsMenuAssets
{
    JOBJSet *option_set;
    JOBJSet *menu_set;
    JOBJSet *value_set_small;
    JOBJSet *value_set_wide;
    JOBJSet *value_set_num;
    JOBJSet *cursor_set;
    JOBJSet *reset_set;
    JOBJSet *warning_set;
} CitySettingsMenuAssets;

typedef struct CitySettingsMenuData
{
    CitySettingsMenuDesc *desc;
    GOBJ *window_gobj;
} CitySettingsMenuData;

void Menu_Init(HSD_Archive *custom_archive);
GOBJ *Menu_Create(CitySettingsMenuDesc *desc);
void Menu_Update(CitySettingsMenuDesc *desc);
CitySettingsMenuAction Menu_Input(CitySettingsMenuDesc *desc);
void Menu_CursorUpdate(JOBJ *cursor_j, JOBJ *val_j, int is_wide);

#define DEFINE_VALUE(_texture_frame, _description) _texture_frame, _description, 0
#define DEFINE_SELECTION_VALUE(_num, _default_val, _save_idx, ...) \
    .value.num = (_num), .value.cur_selection = (_default_val), .value.save_idx = (_save_idx), .value.pos_j = (0), .value.data = (CitySettingsValue[]) { __VA_ARGS__ }
#define DEFINE_SELECTION_NUM(_min_val, _max_val, _default_val, _unit_texture_frame, _description, _save_idx) .number.min = _min_val, .number.max = _max_val, .number.cur_selection = _default_val, .number.texture_frame = _unit_texture_frame, .number.description = _description, .number.save_idx = _save_idx, 0
#define DEFINE_SELECTION_MENU_GENERIC(_menu_ptr, _description) .menu.desc = _menu_ptr, .menu.description = _description
#define DEFINE_SELECTION_MENU_CUSTOM(_init_func, _menu_name, _description) .menu.kind = CITYSETTING_MENUKIND_CUSTOM, .menu.custom.name = _menu_name, .menu.custom.init_menu_cb = _init_func, .menu.description = _description

#define DEFINE_OPTION(kind, texture_frame, ...) \
    kind, texture_frame, 0, { __VA_ARGS__ }

#define DEFINE_SELECTION_MENU(_desc, _description) .menu.desc = _desc, .menu.description = _description

#define DEFINE_MENU_GENERIC(_var_name, _menu_name, _opt_num, ...) \
    static CitySettingsMenuDesc _var_name = {                     \
        .name = _menu_name, .kind = CITYSETTING_MENUKIND_GENERIC, .generic.opt_num = _opt_num, (CitySettingsOption[]){__VA_ARGS__}}

#define DEFINE_MENU_CUSTOM(_var_name, _menu_name, _init_func) \
    static CitySettingsMenuDesc _var_name = {                 \
        .name = _menu_name, .kind = CITYSETTING_MENUKIND_CUSTOM, .custom.init_menu_cb = _init_func}

#define DEFINE_MENU(_var_name, _menu_name, opt_num, ...) \
    static CitySettingsMenuDesc _var_name = {            \
        0, 0, 0, _menu_name, opt_num, {__VA_ARGS__}}

#endif