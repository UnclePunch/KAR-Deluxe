#ifndef MUSICDATABASE_H
#define MUSICDATABASE_H

#define SONGS_PER_PLAYLIST 15

typedef enum MusicSettingsPlaylist
{
    PLAYLIST_MENU,
    PLAYLIST_CITY,
    PLAYLIST_CITYHURRY,
    PLAYLIST_CITYGRAPH,
    PLAYLIST_RESULTS,
    PLAYLIST_DRAGRACE,
    PLAYLIST_AIRGLIDER,
    PLAYLIST_TARGETFLIGHT,
    PLAYLIST_HIGHJUMP,
    PLAYLIST_KIRBYMELEE,
    PLAYLIST_DESTRUCTIONDERBY,
    PLAYLIST_NUM,
} MusicSettingsPlaylist;

typedef enum MusicSettingsPlaylistMode
{
    PLAYLISTMODE_ORIGINAL,
    PLAYLISTMODE_PLAYLIST,
    PLAYLISTMODE_RANDOM,
    PLAYLISTMODE_NUM,
} MusicSettingsPlaylistMode;

typedef struct SongData
{
    int hash;
    u16 entrynum;
    u8 is_rand_selected;
    char *name;
} SongData;

typedef struct PlaylistData
{
    MusicSettingsPlaylistMode mode;
    int song_num;
    int songs[SONGS_PER_PLAYLIST];
} PlaylistData;

int SongData_Init();
void SongData_CountSong(int entrynum);
void SongData_IndexSong(int entrynum);
SongData *SongData_GetDataByName(char *song_name);
SongData *SongData_GetDataByEntrynum(int entrynum);
int SongData_GetIndexByName(char *song_name);
MusicSettingsPlaylistMode SongData_GetPlaylistMode(MusicSettingsPlaylist playlist);
int SongData_CheckPlaylistForCurrentPlayingSong(MusicSettingsPlaylist playlist);
int SongData_PlaySong(int song_database_idx, int volume);
int SongData_PlayRandomSong(int volume);
int SongData_PlayFromPlaylist(MusicSettingsPlaylist playlist, int volume);
void SongData_UpdateCurPlayingEntrynum(int entrynum);
int SongData_GetCurPlayingEntrynum();
void SongData_StopCurPlaying();

#endif