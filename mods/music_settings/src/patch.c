
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
#include "music_change.h"
#include "code_patch/code_patch.h"
#include "fst/fst.h"

extern MajorSceneDesc major_desc;
extern HSD_Archive *music_prompt_archive;

// save previous bgm entrynum
void Hook_BGM_Play(int entrynum)
{
    SongData_UpdateCurPlayingEntrynum(entrynum);
}
CODEPATCH_HOOKCREATE(0x80445308, "mr 3,29\n\t", Hook_BGM_Play, "", 0)
void Hook_BGM_Stop()
{
    SongData_StopCurPlaying();
}
CODEPATCH_HOOKCREATE(0x8005e63c, "", Hook_BGM_Stop, "", 0)

// main in-game music hook
int Hook_Game_BGMPlay()
{
    GroundKind gr_kind = Gm_GetCurrentGrKind();
    switch (gr_kind)
    {
    case (GRKIND_CITY1):
    case (52): // city trial free run
    {
        if (SongData_PlayFromPlaylist(PLAYLIST_CITY, 255))
            return 1;
        break;
    }
    case (GRKIND_DRAG1):
    case (GRKIND_DRAG2):
    case (GRKIND_DRAG3):
    case (GRKIND_DRAG4):
    {
        if (SongData_PlayFromPlaylist(PLAYLIST_DRAGRACE, 255))
            return 1;
        break;
    }
    case (GRKIND_AIRGLIDER):
    {
        if (SongData_PlayFromPlaylist(PLAYLIST_AIRGLIDER, 255))
            return 1;
    }
    case (GRKIND_TARGETFLIGHT):
    {
        if (SongData_PlayFromPlaylist(PLAYLIST_TARGETFLIGHT, 255))
            return 1;
        break;
    }
    case (GRKIND_HIGHJUMP):
    {
        if (SongData_PlayFromPlaylist(PLAYLIST_HIGHJUMP, 255))
            return 1;
        break;
    }
    case (GRKIND_KIRBYMELEE1):
    case (GRKIND_KIRBYMELEE2):
    {
        if (SongData_PlayFromPlaylist(PLAYLIST_KIRBYMELEE, 255))
            return 1;
        break;
    }
    case (GRKIND_DESTRUCTIONDERBY1):
    case (GRKIND_DESTRUCTIONDERBY2):
    case (GRKIND_DESTRUCTIONDERBY3):
    case (GRKIND_DESTRUCTIONDERBY4):
    case (GRKIND_DESTRUCTIONDERBY5):
    {
        if (SongData_PlayFromPlaylist(PLAYLIST_DESTRUCTIONDERBY, 255))
            return 1;
        break;
    }
    }

    return 0;
}
CODEPATCH_HOOKCONDITIONALCREATE(0x8000ff34, "", Hook_Game_BGMPlay, "", 0, 0x8000ff58)

// main menu music replacement function
void Hook_MainMenu_BGMPlay()
{
    MusicSettingsPlaylistMode playlist_mode = SongData_GetPlaylistMode(PLAYLIST_MENU);

    // if menu music is random and we're already playing a song, keep playing it
    if (playlist_mode == PLAYLISTMODE_RANDOM && SongData_GetCurPlayingEntrynum() != -1)
        return;

    // original behavior
    if (playlist_mode == PLAYLISTMODE_ORIGINAL)
    {
        // original behavior, i guess ill just rewrite it (8000bbac)
        int (*Unlock_CheckUnk)() = (void *)0x8007b650;

        BGMKind soundtest_bgm_kind = Gm_GetGameData()->main_menu.soundtest_bgm_kind;

        // prioritize sound test song
        if (soundtest_bgm_kind != -1 && Unlock_CheckUnk() == 0)
        {
            BGM_Play(soundtest_bgm_kind);
        }
        else
            BGM_Play(BGM_MENU);
    }

    // play custom song
    else
    {
        if (!SongData_CheckPlaylistForCurrentPlayingSong(PLAYLIST_MENU) &&
            SongData_PlayFromPlaylist(PLAYLIST_MENU, 255))
        {
            // null sound test song just in case
            Gm_GetGameData()->main_menu.soundtest_bgm_kind = -1;
        }
    }
}

// city results music hook
int Hook_CityResults_BGMPlay()
{
    if (SongData_PlayFromPlaylist(PLAYLIST_RESULTS, 255))
        return 1;
    else
        return 0;
}
CODEPATCH_HOOKCONDITIONALCREATE(0x80045a18, "", Hook_CityResults_BGMPlay, "", 0, 0x80045a20)

// 30 seconds left in city music hook
int Hook_CityHurry_BGMPlay()
{
    int volume = (stc_event_global->is_song_playing) ? 0 : 255; // song comes in muted when an event is in progress

    if (SongData_PlayFromPlaylist(PLAYLIST_CITYHURRY, volume))
        return 1;
    else
        return 0;
}
CODEPATCH_HOOKCONDITIONALCREATE(0x80061c2c, "", Hook_CityHurry_BGMPlay, "", 0, 0x8000ff58)

// city stats ending music hook
void Hook_CityGraph_BGMPlay()
{
    if (SongData_PlayFromPlaylist(PLAYLIST_CITYGRAPH, 255))
        return;

    // original functionality
    else
    {
        BGM_Play(58);
        return;
    }
}
CODEPATCH_HOOKCREATE(0x800460f0, "", Hook_CityGraph_BGMPlay, "b 8\n\t", 0)

// enter scene hook
int Hook_CheckEnterScene()
{
    if (Pad_GetDown(20) & PAD_TRIGGER_Z)
    {
        SFX_Play(FGMMENU_CS_KETTEI);

        // change scene
        Scene_SetNextMajor(major_desc.major_id);
        Scene_ExitMinor();
        Scene_ExitMajor();

        return 1;
    }

    return 0;
}
CODEPATCH_HOOKCONDITIONALCREATE(0x80017960, "", Hook_CheckEnterScene, "", 0, 0x80018198)

// enter scene prompt
void Hook_MusicPrompt_Create(GOBJ *menu_gobj)
{
    // create jobj
    JOBJSet **set = Archive_GetPublicAddress(music_prompt_archive, "ScMenOpen_scene_models");
    JOBJ *j = JObj_LoadJoint(set[0]->jobj);
    JObj_AddNext(JObj_GetIndex(menu_gobj->hsd_object, 1), j);
    return;
}
CODEPATCH_HOOKCREATE(0x8014ba08, "mr 3,29\n\t", Hook_MusicPrompt_Create, "", 0)

// adjust stat size
void Hook_MusicChange_AdjustStatSize(int ply)
{
    if (Ply_IsViewOn(ply))
    {
        int ply_view_num = Gm_GetPlyViewNum();
        if (ply_view_num > 1)
        {

            switch (ply_view_num)
            {
                // 2p
            case 2:
            {
                static float scale_mult = 0.85;
                static Vec2 offsets[] = {
                    {
                        .X = -2.5,
                        .Y = 4.5,
                    },
                    {
                        .X = -2.5,
                        .Y = 3,
                    },
                };

                int ply_view_index = Ply_GetViewIndex(ply);

                MusicChange_ScaleStats(ply, scale_mult, offsets[ply_view_index]);

                break;
            }
                // 3p
            case 3:
            case 4:
            {
                static float scale_mult = 0.95;
                static Vec2 offsets[] = {
                    // top left
                    {
                        .X = -1.5,
                        .Y = 2.3,
                    },
                    // bottom left
                    {
                        .X = -1.5,
                        .Y = 0,
                    },
                    // top right
                    {
                        .X = -0.5,
                        .Y = 2.3,
                    },
                    // bottom right
                    {
                        .X = -0.5,
                        .Y = 0,
                    },
                };

                int ply_view_index = Ply_GetViewIndex(ply);

                MusicChange_ScaleStats(ply, scale_mult, offsets[ply_view_index]);
                break;
            }
            default:
                break;
            }
        }
    }

    return;
}
CODEPATCH_HOOKCREATE(0x801139f8, "mr 3,30\n\t", Hook_MusicChange_AdjustStatSize, "", 0)

void Patches_Apply()
{
    CODEPATCH_REPLACEFUNC(0x8000bba0, Hook_MainMenu_BGMPlay);
    CODEPATCH_HOOKAPPLY(0x80445308);
    CODEPATCH_HOOKAPPLY(0x8005e63c);
    CODEPATCH_HOOKAPPLY(0x8000ff34);
    CODEPATCH_HOOKAPPLY(0x80045a18);
    CODEPATCH_HOOKAPPLY(0x80061c2c);
    CODEPATCH_HOOKAPPLY(0x800460f0);
    CODEPATCH_HOOKAPPLY(0x80017960);
    CODEPATCH_HOOKAPPLY(0x8014ba08);
    CODEPATCH_HOOKAPPLY(0x801139f8);
}