#ifndef MUSICSETTINGS_H
#define MUSICSETTINGS_H

#define VERSION_MAJOR 1
#define VERSION_MINOR 0

#include "music_database.h"

#define MENUGX_0 (0)
#define MENUGX_1 (1)
#define MENUGX_2 (2)
#define MENUGX_SONGNAME (3)

#define GAMEMODEBUTTON_NUM 6
#define PLAYLISTENTRY_NUM 6

#define SONG_SCROLL_SPEED (0.08)  // speed at which the text scrolls
#define SONG_VISIBLE_WIDTH (32.5) // max text width before requiring scrolling
#define SONG_ASPECT_X (1000)      // maximum width a song can be

typedef enum MenuFocus
{
    MENUFOCUS_GAMEMODE,
    MENUFOCUS_PLAYLISTMODE,
    MENUFOCUS_PLAYLISTENTRIES,
} MenuFocus;

typedef enum MenuUpdateKind
{
    UPDATEKIND_NONE,
    UPDATEKIND_ADVANCE,
    UPDATEKIND_REGRESS,
    UPDATEKIND_CHANGE,
} MenuUpdateKind;

typedef enum SongScrollState
{
    SONGSCROLL_NONE,
    SONGSCROLL_STARTWAIT,
    SONGSCROLL_MOVE,
    SONGSCROLL_STOPWAIT,
} SongScrollState;

typedef struct MusicSettingsSave
{
    struct
    {
        MusicSettingsPlaylistMode mode;
        int song_hash[SONGS_PER_PLAYLIST];
    } playlist[PLAYLIST_NUM];
} MusicSettingsSave;

typedef struct PlaylistDefault
{
    u8 playlist;
    u8 entry;
    char *song_name;
} PlaylistDefault;

typedef struct MusicSettingsBackgroundData
{
    Text *description_text;
} MusicSettingsBackgroundData;

typedef struct MusicSettingsData
{

    MenuFocus focus;

    struct
    {
        s16 cursor;
        s16 scroll;
        JOBJ *arrowup_j;
        JOBJ *arrowdown_j;
        JOBJSet **button_background_set;
        struct
        {
            JOBJ *j;
            JOBJ *name_j;
            JOBJ *background_j;
            Text *text;
        } button[GAMEMODEBUTTON_NUM];
    } game_mode;

    struct
    {
        struct
        {
            JOBJ *j;
            JOBJ *name_j;
            JOBJ *fill_j;
            JOBJ *cursor_j;
            Text *text;
        } mode_button;

        s16 cursor;
        s16 scroll;
        JOBJ *cursor_j;
        JOBJ *arrowup_j;
        JOBJ *arrowdown_j;

        struct
        {
            int state;
            int timer;
        } text_scroll;

        struct
        {
            JOBJ *j;
            JOBJ *fill_j;
            JOBJ *cursorpos_j;
            JOBJ *songname_j;
            Text *songname_text;
            JOBJ *songnum_j;
            Text *songnum_text;
            JOBJ *deletepos_j;
        } entry_button[PLAYLISTENTRY_NUM];
        struct
        {
            JOBJ *j;
        } add_button;
        struct
        {
            JOBJ *j;
        } delete_button;
    } playlist;

    struct
    {
        Text *text;
        JOBJ *text_j;
    } description[2]; // one for each line
} MusicSettingsData;

void MusicSettings_CopyFromSave();
void MusicSettings_SaveInit(MusicSettingsSave *save, int req_init);
void MainMenu_LoadMusicPrompt();

MajorKind MusicSettings_Init();
void Music_MajorEnter();
void Music_MajorExit();
void MusicSettings_Enter();
void MusicSettings_Exit(void *data);
void MusicSettings_Think(void *data);

void MusicSettings_Create();
void MusicSettings_Destroy();
GOBJ *MusicSettings_CamCreate(COBJDesc *desc);
void MusicSettings_CamGX(GOBJ *g);
void MusicSettings_CameraThink(GOBJ *gobj);

GOBJ *MusicSettings_BGCreate();
void MusicSettings_BGThink(GOBJ *g);
void MusicSettings_SetDescription(char *str, ...);

GOBJ *MusicSettings_UICreate();
void MusicSettings_UIDestroy(MusicSettingsData *gp);
void MusicSettings_UIInput(GOBJ *g);
void MusicSettings_UIUpdate(GOBJ *g);
void MusicSettings_UIThink(GOBJ *g);
void MusicSettings_UIGX(GOBJ *g, int pass);

int SongData_Init();
void SongData_TestUpdate();
void SongData_CopyFromSave();
void SongData_CopyToSave();
int SongData_ChangeSong(MusicSettingsPlaylist playlist, int entry, int dir);

float SongScroll_GetAmount(Text *t);

#endif