
#include "text.h"
#include "os.h"
#include "hsd.h"
#include "preload.h"
#include "scene.h"
#include "inline.h"
#include "audio.h"
#include "game.h"

#include "musicsettings.h"
#include "music_database.h"
#include "text_joint/text_joint.h"
#include "fst/fst.h"

// Music Database
int stc_cur_playing_entrynum = -1;
int stc_song_num = 0;
SongData *stc_song_data;
PlaylistData stc_playlist_data[PLAYLIST_NUM] = {0};
static char *stc_vanilla_song_names[] = {
    "Menu",
    "How To Play",
    "Fantasy Meadows",
    "Celestial Valley",
    "Sky Sands",
    "Frozen Hillside",
    "Magma Flows",
    "Beanstalk Park",
    "Machine Passage",
    "Checker Knights",
    "Nebula Belt",
    "Spring Breeze",
    "Forest Stage",
    "The Arena",
    "Rainbow Resort",
    "Revenge of Meta Knight 1",
    "The Beginner's Room",
    "Revenge of Meta Knight 2",
    "Green Greens",
    "Dream Land",
    "Air Ride Results",
    "Air Ride Ending",
    "Top Ride: Grass",
    "Top Ride: Sand",
    "Top Ride: Sky",
    "Top Ride: Fire",
    "Top Ride: Light",
    "Top Ride: Water",
    "Top Ride: Metal",
    "The Mountain",
    "Rock Star",
    "Revenge of Meta Knight (Map)",
    "Versus King Dedede",
    "Mid-boss Room",
    "Float Islands",
    "Shooting",
    "Top Ride Results",
    "City",
    "City: Not Much Time Left",
    "City (Backside)",
    "Dyna Blade Intro",
    "Tac Challenge",
    "Flying Meteor",
    "Huge Pillar",
    "Station Fire",
    "What's in the Box?",
    "The Lighthouse Light Burns",
    "Rowdy Charge Tank",
    "Item Bounce",
    "Dense Fog Today",
    "The Legendary Air Ride Machine",
    "Properties Graph",
    "Drag Race",
    "Air Glider",
    "Target Flight",
    "Kirby Melee",
    "City Trial Ending",
};
int SongData_Init()
{
    // count songs
    // FST_ForEachInFolder("audio/jp", ".hps", 1, SongData_CountSong, 0);    // count vanilla songs
    for (int i = 0; i < 62; i++)
    {
        if (stc_soundtest_desc[i].kind == 1)
            stc_song_num++;
    }

    FST_ForEachInFolder("audio/music", ".hps", 1, (void (*)(int, void *))SongData_CountSong, 0); // count additional songs

    // alloc database
    stc_song_data = HSD_MemAlloc(sizeof(*stc_song_data) * stc_song_num);
    OSReport("allocated song database for %d songs \n", stc_song_num);

    // populate database
    stc_song_num = 0;
    // FST_ForEachInFolder("audio/jp", ".hps", 1, (void (*)(int, void *))SongData_IndexSong, 0);    // count vanilla songs
    for (int i = 0; i < 62; i++)
    {
        if (stc_soundtest_desc[i].kind == 1)
        {
            char bgm_path[64];
            strcpy(bgm_path, "audio/");
            strcat(bgm_path, stc_bgm_desc[stc_soundtest_desc[i].bgm].path);
            int entrynum = DVDConvertPathToEntrynum(bgm_path);

            stc_song_data[stc_song_num].is_rand_selected = 0;
            stc_song_data[stc_song_num].entrynum = entrynum;
            stc_song_data[stc_song_num].name = stc_vanilla_song_names[stc_song_num]; // FST_GetFilenameFromEntrynum(entrynum);

            // hash song
            char *full_path = FST_GetFilePathFromEntrynum(entrynum);
            stc_song_data[stc_song_num].hash = hash_32_str(full_path);

            // OSReport("added song %d \"%s\" with hash 0x%08x\n",
            //          stc_song_num,
            //          stc_song_data[stc_song_num].name,
            //          stc_song_data[stc_song_num].hash);

            stc_song_num++;
        }
    }
    FST_ForEachInFolder("audio/music", ".hps", 1, (void (*)(int, void *))SongData_IndexSong, 0); // count additional songs

    // verify hash collisions
    for (int i = 0; i < stc_song_num; i++)
    {
        for (int j = 0; j < stc_song_num; j++)
        {
            if (i == j)
                continue;

            if (stc_song_data[i].hash == stc_song_data[j].hash)
                OSReport("HASH COLLISION: (0x%08x) \"%s\" AND \"%s\"\n", stc_song_data[i].hash, stc_song_data[i].name, stc_song_data[j].name);
        }
    }
}
void SongData_CountSong(int entrynum)
{
    stc_song_num++;
}
void SongData_IndexSong(int entrynum)
{
    stc_song_data[stc_song_num].entrynum = entrynum;
    stc_song_data[stc_song_num].name = FST_GetFilenameFromEntrynum(entrynum);

    // hash song
    char *full_path = FST_GetFilePathFromEntrynum(entrynum);
    stc_song_data[stc_song_num].hash = hash_32_str(full_path);

    // OSReport("added song %d \"%s\" with hash 0x%08x\n",
    //          stc_song_num,
    //          stc_song_data[stc_song_num].name,
    //          stc_song_data[stc_song_num].hash);

    stc_song_num++;
}
SongData *SongData_GetDataByName(char *song_name)
{
    // search for a song that corresponds to this hash
    for (int i = 0; i < stc_song_num; i++)
    {
        if (strcmp(song_name, stc_song_data[i].name) == 0) // find match
        {
            return &stc_song_data[i];
        }
    }

    OSReport("MUSIC: warning %s not found\n", song_name);

    return 0;
}
SongData *SongData_GetDataByEntrynum(int entrynum)
{
    // search for a song that corresponds to this entrynum
    for (int i = 0; i < stc_song_num; i++)
    {
        if (entrynum == stc_song_data[i].entrynum) // find match
        {
            return &stc_song_data[i];
        }
    }

    return 0;
}
int SongData_GetIndexByName(char *song_name)
{
    // search for a song that corresponds to this hash
    for (int i = 0; i < stc_song_num; i++)
    {
        if (strcmp(song_name, stc_song_data[i].name) == 0) // find match
        {
            return i;
        }
    }

    OSReport("MUSIC: warning %s not found\n", song_name);
    return 0;
}
int SongData_CheckPlaylistForCurrentPlayingSong(MusicSettingsPlaylist playlist)
{
    // check all songs against the currently playing one
    for (int i = 0; i < SONGS_PER_PLAYLIST; i++)
    {
        int song_database_idx = stc_playlist_data[playlist].songs[i];
        if (song_database_idx != 0)
        {
            if (stc_cur_playing_entrynum == stc_song_data[song_database_idx].entrynum)
                return 1;
        }
    }

    return 0;
}
MusicSettingsPlaylistMode SongData_GetPlaylistMode(MusicSettingsPlaylist playlist)
{
    return stc_playlist_data[playlist].mode;
}

int SongData_PlayFromPlaylist(MusicSettingsPlaylist playlist)
{
    switch (stc_playlist_data[playlist].mode)
    {
    case (PLAYLISTMODE_ORIGINAL):
    {
        return 0;
        break;
    }
    case (PLAYLISTMODE_PLAYLIST):
    {
        // count songs in list
        int song_num = stc_playlist_data[playlist].song_num;

        // exit if no valid songs on list
        if (song_num == 0)
        {
            OSReport("MUSIC: No valid songs on playlist %d\n", playlist);
            return 0;
        }

        // find this song
        int rand_entry = HSD_Randi(song_num);
        int song_to_play = 0;
        song_num = 0;
        for (int i = 0; i < SONGS_PER_PLAYLIST; i++)
        {
            if (stc_playlist_data[playlist].songs[i] != 0)
            {
                if (song_num == rand_entry)
                {
                    song_to_play = stc_playlist_data[playlist].songs[song_num];
                    break;
                }
                song_num++;
            }
        }

        // play song
        SongData_PlaySong(song_to_play); // play it

        return 1;
    }
    case (PLAYLISTMODE_RANDOM):
    {
        int rand_song_idx;

        // get a random song
        do
        {
            rand_song_idx = HSD_Randi(stc_song_num);
        } while (rand_song_idx == stc_cur_playing_entrynum);

        SongData_PlaySong(rand_song_idx); // play it

        return 1;
    }
    }
}
int SongData_PlaySong(int song_database_idx)
{
    int entrynum = stc_song_data[song_database_idx].entrynum;

    if (stc_cur_playing_entrynum == entrynum)
        return 0;

    BGM_PlayFile(FST_GetFilePathFromEntrynum(entrynum),
                 255,
                 63,
                 1);

    // null the game's last played bgm id
    (*stc_bgmkind_cur_playing) = -1;

    return 1;
}
int SongData_PlayRandomSong()
{
    // create array of valid candidates
    int valid_num = 0;
    u8 *valid_arr = HSD_MemAlloc(stc_song_num); // alloc this because the number of songs can vary greatly per installation
    do
    {
        for (int i = 0; i < stc_song_num; i++)
        {
            if (stc_song_data[i].is_rand_selected == 0)
                valid_arr[valid_num++] = i;
        }

        // if no random songs remain
        if (valid_num == 0)
        {
            // reset all
            for (int i = 0; i < stc_song_num; i++)
                stc_song_data[i].is_rand_selected = 0;
        }

    } while (valid_num == 0);

    // get a random song
    int rand_song_idx;
    do
    {
        rand_song_idx = valid_arr[HSD_Randi(valid_num)];
    } while (stc_song_data[rand_song_idx].entrynum == stc_cur_playing_entrynum);

    HSD_Free(valid_arr); // immediately free before other allocs occur as to not fragment the heap

    SongData_PlaySong(rand_song_idx);                  // play it
    stc_song_data[rand_song_idx].is_rand_selected = 1; // set as selected

    return 1;
}

int SongData_GetCurPlayingEntrynum()
{
    return stc_cur_playing_entrynum;
}
void SongData_UpdateCurPlayingEntrynum(int entrynum)
{
    stc_cur_playing_entrynum = entrynum;

    // OSReport("currently playing (0x%08x) (0x%x) %s\n", &stc_cur_playing_entrynum, entrynum, FST_GetFilenameFromEntrynum(entrynum));
}
void SongData_StopCurPlaying()
{
    stc_cur_playing_entrynum = -1;

    // OSReport("removing currently playing song\n");
}