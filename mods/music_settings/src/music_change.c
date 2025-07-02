
#include "text.h"
#include "useful.h"
#include "hsd.h"
#include "preload.h"
#include "scene.h"
#include "inline.h"
#include "audio.h"
#include "game.h"

#include "music_change.h"
#include "musicsettings.h"
#include "music_database.h"
#include "fst/fst.h"
#include "text_joint/text_joint.h"

static GOBJ *stc_music_change_gobj;

extern int stc_song_num;
extern SongData *stc_song_data;
extern PlaylistData stc_playlist_data[PLAYLIST_NUM];

GOBJ *MusicChange_Create()
{
    GOBJ *g = GOBJ_EZCreator(27, GAMEPLINK_PAUSEHUD, 0,
                             0, 0,
                             HSD_OBJKIND_JOBJ, 0,
                             MusicChange_Think, 20,
                             JObj_GX, 21, 1);

    stc_music_change_gobj = g;
    return g;
}
void MusicChange_Destroy()
{
    GObj_Destroy(stc_music_change_gobj);
    stc_music_change_gobj = 0;

    return;
}
void MusicChange_Think(GOBJ *g)
{

    if (Pad_GetDown(Gm_GetGameData()->pause_ply) & PAD_TRIGGER_Z)
    {
        // not during events (will play both songs at once)
        if (Gm_IsInCity() && Gm_GetCityMode() == CITYMODE_TRIAL)
        {
            EventCheckData *ed = (*stc_eventcheck_gobj)->userdata;
            if (ed->state > 0)
            {
                SFX_Play(FGMMENU_CS_BEEP1);
                return;
            }
        }

        int is_changed = 0;
        MajorKind mj = Scene_GetCurrentMajor();

        if (mj == MJRKIND_AIR)
        {
            SongData_PlayRandomSong();
        }
        else if (mj == MJRKIND_CITY)
        {
            MusicSettingsPlaylist playlist_kind = -1;
            GroundKind gr_kind = Gm_GetCurrentGrKind();

            switch (gr_kind)
            {
            case (GRKIND_CITY1):
            case (52): // free run
            {
                playlist_kind = PLAYLIST_CITY;
            }
            case (GRKIND_DRAG1):
            case (GRKIND_DRAG2):
            case (GRKIND_DRAG3):
            case (GRKIND_DRAG4):
            {
                playlist_kind = PLAYLIST_DRAGRACE;
            }
            case (GRKIND_AIRGLIDER):
            {
                playlist_kind = PLAYLIST_AIRGLIDER;
            }
            case (GRKIND_TARGETFLIGHT):
            {
                playlist_kind = PLAYLIST_TARGETFLIGHT;
            }
            case (GRKIND_HIGHJUMP):
            {
                playlist_kind = PLAYLIST_HIGHJUMP;
            }
            case (GRKIND_KIRBYMELEE1):
            case (GRKIND_KIRBYMELEE2):
            {
                playlist_kind = PLAYLIST_KIRBYMELEE;
            }
            case (GRKIND_DESTRUCTIONDERBY1):
            case (GRKIND_DESTRUCTIONDERBY2):
            case (GRKIND_DESTRUCTIONDERBY3):
            case (GRKIND_DESTRUCTIONDERBY4):
            case (GRKIND_DESTRUCTIONDERBY5):
            {
                playlist_kind = PLAYLIST_DESTRUCTIONDERBY;
            }
            }

            if (playlist_kind != 1 || !SongData_PlayFromPlaylist(playlist_kind))
                SongData_PlayRandomSong();
        }

        SFX_Play(FGMMENU_CS_MV);

        // raise music volume
        BGM_RaiseVolume();
    }
}