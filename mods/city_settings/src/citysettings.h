#ifndef CITYSETTINGS_H
#define CITYSETTINGS_H

#include "datatypes.h"

#define MENUGX_0 (0)
#define MENUGX_1 (1)
#define MENUGX_2 (2)

#define MENUPLINK_GENERIC (30)
#define MENUPLINK_CUSTOM (31)

#define VERSION_MAJOR 1
#define VERSION_MINOR 0

typedef struct CitySettingsMenuDesc CitySettingsMenuDesc;

typedef enum CitySettingsOptionKind
{
    CITYSETTING_OPTKIND_VALUE,
    CITYSETTING_OPTKIND_MENU,
    CITYSETTING_OPTKIND_NUM,
} CitySettingsOptionKind;

typedef enum CitySettingsMenuKind
{
    CITYSETTING_MENUKIND_GENERIC,
    CITYSETTING_MENUKIND_CUSTOM,
    CITYSETTING_MENUKIND_NUM,
} CitySettingsMenuKind;

typedef enum CitySettingsOptionTexture
{
    CITYSETTING_OPTTEX_ALLOW_REPEATS,
    CITYSETTING_OPTTEX_DAMAGE_RATIO,
    CITYSETTING_OPTTEX_DRAGOON,
    CITYSETTING_OPTTEX_FREQUENCY,
    CITYSETTING_OPTTEX_HYDRA,
    CITYSETTING_OPTTEX_INITIAL_DELAY,
    CITYSETTING_OPTTEX_ALLUP,
    CITYSETTING_OPTTEX_PREDICTION,
    CITYSETTING_OPTTEX_REVEAL,
    CITYSETTING_OPTTEX_SPEED_LIMIT,
    CITYSETTING_OPTTEX_STADIUM,
    CITYSETTING_OPTTEX_STARTING_MACHINE,
    CITYSETTING_OPTTEX_TEMPO,
    CITYSETTING_OPTTEX_TIME,
    CITYSETTING_OPTTEX_MACHINE_RESPAWN,
    CITYSETTING_OPTTEX_GAUNTLETMODE,
} CitySettingsOptionTexture;

typedef enum CitySettingsMenuTexture
{
    CITYSETTING_MENUTEX_EVENT_SETTINGS,
    CITYSETTING_MENUTEX_ITEM_SETTINGS,
    CITYSETTING_MENUTEX_MACHINE_SETTINGS,
    CITYSETTING_MENUTEX_RANDOM_ITEMS,
    CITYSETTING_MENUTEX_ADDITIONAL_SETTINGS,
    CITYSETTING_MENUTEX_RANDOM_EVENT,
    CITYSETTING_MENUTEX_RANDOM_MACHINE,
    CITYSETTING_MENUTEX_RANDOM_STADIUM,
    CITYSETTING_MENUTEX_STADIUM_SETTINGS,
} CitySettingsMenuTexture;

typedef enum CitySettingsNumTexture
{
    CITYSETTING_NUMTEX_MIN = 1,
} CitySettingsNumTexture;

typedef enum CitySettingsValueSmallTexture
{
    CITYSETTING_VALSMALLTEX_ALWAYS,
    CITYSETTING_VALSMALLTEX_CORRECT,
    CITYSETTING_VALSMALLTEX_DEFAULT,
    CITYSETTING_VALSMALLTEX_LAPS,
    CITYSETTING_VALSMALLTEX_NEVER,
    CITYSETTING_VALSMALLTEX_OFF,
    CITYSETTING_VALSMALLTEX_ON,
    CITYSETTING_VALSMALLTEX_ORIGINAL,
    CITYSETTING_VALSMALLTEX_QUICK,
    CITYSETTING_VALSMALLTEX_SLOW,
} CitySettingsValueSmallTexture;

typedef enum CitySettingsValueLargeTexture
{
    CITYSETTING_VALLARGETEX_SHUFFLE,
    CITYSETTING_VALLARGETEX_ALL,
    CITYSETTING_VALLARGETEX_DRAG_RACE,
    CITYSETTING_VALLARGETEX_AIR_GLIDER,
    CITYSETTING_VALLARGETEX_TARGET_FLIGHT,
    CITYSETTING_VALLARGETEX_HIGH_JUMP,
    CITYSETTING_VALLARGETEX_KIRBY_MELEE,
    CITYSETTING_VALLARGETEX_DESTRUCTION_DERBY,
    CITYSETTING_VALLARGETEX_SINGLE_RACE,
    CITYSETTING_VALLARGETEX_VS_KING_DEDEDE,
    CITYSETTING_VALLARGETEX_BLANK,
    CITYSETTING_VALLARGETEX_05,
    CITYSETTING_VALLARGETEX_075,
    CITYSETTING_VALLARGETEX_100,
    CITYSETTING_VALLARGETEX_125,
    CITYSETTING_VALLARGETEX_150,
    CITYSETTING_VALLARGETEX_175,
    CITYSETTING_VALLARGETEX_200,
    CITYSETTING_VALLARGETEX_250,
    CITYSETTING_VALLARGETEX_300,
    CITYSETTING_VALLARGETEX_500,
    CITYSETTING_VALLARGETEX_BULK_STAR,
    CITYSETTING_VALLARGETEX_COMPACT_STAR,
    CITYSETTING_VALLARGETEX_DRAGOON,
    CITYSETTING_VALLARGETEX_FLIGHT_WARP_STAR,
    CITYSETTING_VALLARGETEX_FORMULA,
    CITYSETTING_VALLARGETEX_HIGH,
    CITYSETTING_VALLARGETEX_HYDRA,
    CITYSETTING_VALLARGETEX_LOW,
    CITYSETTING_VALLARGETEX_NONE,
    CITYSETTING_VALLARGETEX_ORIGINAL,
    CITYSETTING_VALLARGETEX_RANDOM_ALL,
    CITYSETTING_VALLARGETEX_RANDOM_SINGLE,
    CITYSETTING_VALLARGETEX_REX_WHEELIE,
    CITYSETTING_VALLARGETEX_ROCKET_STAR,
    CITYSETTING_VALLARGETEX_SHADOW_STAR,
    CITYSETTING_VALLARGETEX_SLICK_STAR,
    CITYSETTING_VALLARGETEX_SWERVE_STAR,
    CITYSETTING_VALLARGETEX_TURBO_STAR,
    CITYSETTING_VALLARGETEX_VERY_HIGH,
    CITYSETTING_VALLARGETEX_WAGON_STAR,
    CITYSETTING_VALLARGETEX_WARP_STAR,
    CITYSETTING_VALLARGETEX_WHEELIE_BIKE,
    CITYSETTING_VALLARGETEX_WHEELIE_SCOOTER,
    CITYSETTING_VALLARGETEX_WINGED_STAR,
    CITYSETTING_VALLARGETEX_JET_STAR,
} CitySettingsValueLargeTexture;

typedef enum CitySettingsSaveID
{
    CITYSETTING_SAVE_TIME,
    CITYSETTING_SAVE_ITEMFREQ,
    CITYSETTING_SAVE_DRAGOONSPAWN,
    CITYSETTING_SAVE_HYDRASPAWN,
    CITYSETTING_SAVE_ALLUP,
    CITYSETTING_SAVE_STADIUM,
    CITYSETTING_SAVE_STADIUMREVEAL,
    CITYSETTING_SAVE_EVENTFREQ,
    CITYSETTING_SAVE_EVENTDELAY,
    CITYSETTING_SAVE_EVENTREPEAT,
    CITYSETTING_SAVE_TEMPO,
    CITYSETTING_SAVE_DAMAGE,
    CITYSETTING_SAVE_MACHINESTART,
    CITYSETTING_SAVE_MACHINEPACK,
    CITYSETTING_SAVE_MACHINESPEED,
    CITYSETTING_SAVE_MACHINERESPAWN,
    CITYSETTING_SAVE_GAUNTLETMODE,
} CitySettingsSaveID;

typedef enum CitySettingsMenuDirection
{
    CITYSETTING_MENUDIR_NONE,
    CITYSETTING_MENUDIR_ADVANCE,
    CITYSETTING_MENUDIR_REGRESS,
} CitySettingsMenuDirection;

typedef enum CitySettingsMenuAction
{
    CITYSETTING_MENUACT_NONE,
    CITYSETTING_MENUACT_CHANGE,
    CITYSETTING_MENUACT_ADVANCE,
    CITYSETTING_MENUACT_REGRESS,
    CITYSETTING_MENUACT_EXIT,
} CitySettingsMenuAction;

typedef struct CitySettingsSave
{
    u8 settings[100];
    u64 random_item_bitfield;
    int random_event_bitfield;
    int random_stadium_bitfield;
    int random_machine_bitfield;
} CitySettingsSave;

typedef struct CitySettingsData
{
    int is_intro_anim;
    GOBJ *bg_gobj;
    Text *description_text;
    struct
    {
        GOBJ *gobj_cur;
        GOBJ *gobj_prev;
        JOBJSet **name_jobjset;
        struct
        {
            Text *text;
            JOBJ *j;
        } name[3];
        CitySettingsMenuDirection dir;
        CitySettingsMenuDesc *desc_cur;
        int (*input_update)(GOBJ *); // returns is_exit
    } menu;
} CitySettingsData;

typedef struct CitySettingsValue
{
    int texture_frame; // texture frame to represent this value
    char *description; //
    JOBJ *j;           // ptr to this value's jobj
} CitySettingsValue;

typedef struct CitySettingsOption
{
    CitySettingsOptionKind kind;
    int texture_frame;
    JOBJ *j;
    union
    {
        // value data
        struct
        {
            int num;                     // number of values this option has
            int cur_selection;           // current value
            CitySettingsSaveID save_idx; //
            JOBJ *pos_j;                 //
            CitySettingsValue *data;     //
        } value;

        // menu data
        struct
        {
            CitySettingsMenuDesc *desc;
            char *description;
        } menu;

        // number data
        struct
        {
            int min;                     // number of values this option has
            int max;                     // number of values this option has
            int cur_selection;           // current value
            int texture_frame;           // texture frame to represent this value type
            char *description;           //
            CitySettingsSaveID save_idx; //
            JOBJ *j;                     // ptr to this value's jobj
        } number;
    } u;
} CitySettingsOption;
struct CitySettingsMenuDesc
{
    char *name;
    CitySettingsMenuKind kind;
    CitySettingsMenuDesc *prev; // runtime variable
    union
    {
        struct
        {
            JOBJ *cursor_j; // runtime variable
            int cursor_val; // runtime variable
            int opt_num;
            CitySettingsOption *options;
        } generic;
        struct
        {
            GOBJ *(*init_menu_cb)(int (**input_cb)(GOBJ *));
        } custom;
    };
};

typedef struct CitySettingsMenuData
{
    CitySettingsMenuDesc *desc;
} CitySettingsMenuData;

typedef struct CitySettingsMenuAssets
{
    JOBJSet *option_set;
    JOBJSet *menu_set;
    JOBJSet *value_set_small;
    JOBJSet *value_set_wide;
    JOBJSet *value_set_num;
    JOBJSet *cursor_set;
} CitySettingsMenuAssets;

void Major_Enter();
void Major_ExitMinor();
void Minor_Load();
void Minor_Think();
void Minor_Exit();
void CitySettings_OnSaveCreateOrLoad();
void CitySettings_OnGameStart();
void CitySettings_Init();
void CitySettings_SaveInit(CitySettingsSave *save, int req_init);
CitySettingsSave *CitySettings_SaveGet();
void CitySettings_Load();
void CitySettings_Think(void *data);
void CitySettings_Exit(void *data);
void CitySettings_Create();
void CitySettings_Destroy();
GOBJ *CitySettings_CamCreate(COBJDesc *desc);
void CitySettings_CamGX(GOBJ *g);
void CitySettings_CameraThink(GOBJ *gobj); // 8022BA1C
GOBJ *CitySettings_BGCreate();
void CitySettings_BGThink(GOBJ *g);
void CitySettings_UpdateVanillaSettings();
void CitySettings_CopyToSave(CitySettingsMenuDesc *desc);
void CitySettings_CopyFromSave(CitySettingsMenuDesc *desc);
void CitySettings_UpdateDescription(char *s);
void CitySettings_HideDescription();
void CitySettings_ShowDescription();
HSD_Archive *CitySettings_GetCustomMenuArchive();
void Text_CopyJointPosition(Text *t, JOBJ *j);
int Text_ConvertASCIIRewrite(u8 *out, char *s);

#endif