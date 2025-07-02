
#include "text.h"
#include "useful.h"
#include "hsd.h"
#include "preload.h"
#include "scene.h"
#include "inline.h"
#include "audio.h"
#include "game.h"

#include "musicsettings.h"
#include "music_database.h"
#include "fst/fst.h"

// Music Database
int stc_cur_playing = -1;
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
int SongData_SanitizeText(char *in, char *out, int buffer_size)
{
    // symbol lookup
    struct ASCIISymbolLookup
    {
        u8 ascii;
        u16 text_code;
    };

    static struct ASCIISymbolLookup symbol_lookup[] = {
        {
            .ascii = ' ',
            .text_code = 0x8140,
        },
        {
            .ascii = '!',
            .text_code = 0x8149,
        },
        {
            .ascii = '"',
            .text_code = 0x8168,
        },
        {
            .ascii = '#',
            .text_code = 0x8194,
        },
        {
            .ascii = '$',
            .text_code = 0x8190,
        },
        {
            .ascii = '%',
            .text_code = 0x8193,
        },
        {
            .ascii = '&',
            .text_code = 0x8195,
        },
        {
            .ascii = '(',
            .text_code = 0x8169,
        },
        {
            .ascii = ')',
            .text_code = 0x816A,
        },
        {
            .ascii = '[',
            .text_code = 0x816d,
        },
        {
            .ascii = ']',
            .text_code = 0x816e,
        },
        {
            .ascii = '*',
            .text_code = 0x8196,
        },
        {
            .ascii = '+',
            .text_code = 0x817B,
        },
        {
            .ascii = '\'',
            .text_code = 0x8166,
        },
        {
            .ascii = ',',
            .text_code = 0x8143,
        },
        {
            .ascii = '-',
            .text_code = 0x817C,
        },
        // {
        //     .ascii = '.',
        //     .text_code = 0x8144,
        // },
        {
            .ascii = '/',
            .text_code = 0x815E,
        },
        {
            .ascii = ':',
            .text_code = 0x8146,
        },
        {
            .ascii = ';',
            .text_code = 0x8147,
        },
        {
            .ascii = '=',
            .text_code = 0x8181,
        },
        {
            .ascii = '?',
            .text_code = 0x8148,
        },
        {
            .ascii = '@',
            .text_code = 0x8197,
        },
        {
            .ascii = '_',
            .text_code = 0x8151,
        },
    };

    int out_size = 0;

    while (in[0] != '\0')
    {
        char this_char = in[0];

        // normal character
        if ((this_char >= '0' && this_char <= '9') ||
            (this_char >= 'A' && this_char <= 'Z') ||
            (this_char >= 'a' && this_char <= 'z'))
        {
            if ((out_size + 1 + 1) >= buffer_size)
                return 0;

            // copy directly
            out[0] = in[0];
            in++;
            out++;
            out_size++;
        }
        else
        {
            int is_found = 0;

            // check if its a special character
            for (int i = 0; i < GetElementsIn(symbol_lookup); i++)
            {
                if (this_char == symbol_lookup[i].ascii)
                {
                    if ((out_size + 2 + 1) >= buffer_size)
                        return 0;

                    *((u16 *)&out[0]) = symbol_lookup[i].text_code;
                    in++;
                    out += 2;
                    out_size += 2;

                    is_found = 1;

                    break;
                }
            }

            // symbol not found in lookup, just copy it over
            if (!is_found)
            {
                if ((out_size + 1 + 1) >= buffer_size)
                    return 0;

                // copy directly
                out[0] = in[0];
                in++;
                out++;
                out_size++;
            }
        }
    }

    out[0] = '\0';

    return 1;
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
            if (stc_cur_playing == stc_song_data[song_database_idx].entrynum)
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
        } while (rand_song_idx == stc_cur_playing);

        SongData_PlaySong(rand_song_idx); // play it

        return 1;
    }
    }
}
int SongData_PlaySong(int song_database_idx)
{
    int entrynum = stc_song_data[song_database_idx].entrynum;

    if (stc_cur_playing == entrynum)
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
    int rand_song_idx;

    // get a random song
    do
    {
        rand_song_idx = HSD_Randi(stc_song_num);
    } while (rand_song_idx == stc_cur_playing);

    SongData_PlaySong(rand_song_idx); // play it

    return 1;
}

int SongData_GetCurPlaying()
{
    return stc_cur_playing;
}
void SongData_UpdateCurPlaying(int entrynum)
{
    stc_cur_playing = entrynum;

    // OSReport("currently playing (0x%08x) (0x%x) %s\n", &stc_cur_playing, entrynum, FST_GetFilenameFromEntrynum(entrynum));
}
void SongData_StopCurPlaying()
{
    stc_cur_playing = -1;

    // OSReport("removing currently playing song\n");
}