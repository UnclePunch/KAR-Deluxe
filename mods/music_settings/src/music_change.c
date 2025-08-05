#include "text.h"
#include "os.h"
#include "preload.h"
#include "scene.h"
#include "inline.h"
#include "audio.h"
#include "game.h"
#include "hud.h"

#include "music_change.h"
#include "music_database.h"
#include "fst/fst.h"
#include "text_joint/text_joint.h"

static GOBJ *stc_music_change_gobj;
static Text *stc_text;
static NowPlayingAssets *music_assets;
static int music_text_canvas_idx;

extern int stc_song_num;
extern SongData *stc_song_data;
extern PlaylistData stc_playlist_data[PLAYLIST_NUM];

void MusicChange_Init()
{
    Hoshi_AddPreloadGameFile("IfAllNowPlaying", PRELOADHEAPKIND_ALLM);
}

void MusicChange_On3DLoad()
{
    HSD_Archive *archive;
    Gm_LoadGameFile(&archive, "IfAllNowPlaying");
    music_assets = Archive_GetPublicAddress(archive, "NowPlaying_scene_models");

    GOBJ *g = GOBJ_EZCreator(27, GAMEPLINK_CAMHUD, 0,
                             0, 0,
                             HSD_OBJKIND_COBJ, *Gm_Get3dData()->hud_sobj->cobjdesc,
                             0, 0,
                             MusicChange_TextCObj, 0, 6);

    g->cobj_links = (1ULL << SONGNAME_GXLINK);

    MusicChangeTextParams *param = MusicChange_GetTextParam();

    COBJ *c = g->hsd_object;
    CObj_SetScissor(c, param->scissor_left, param->scissor_right, 0, 480);

    music_text_canvas_idx = Text_CreateCanvas(0, -1, 28, GAMEPLINK_CAMHUD, 0, SONGNAME_GXLINK, 0, 0);
}

GOBJ *MusicChange_Create()
{
    JOBJSet *set = MusicChange_GetJOBJSet();

    // air ride 1p needs to render in front of the hud
    int is_airride_1p = Gm_Get3dData()->plyview_num == 1 && !Gm_IsInCity();
    int gx_pri = (is_airride_1p) ? 2 : 1;

    // create hud element gobj
    GOBJ *g = GOBJ_EZCreator(27, GAMEPLINK_PAUSEHUD, 0,
                             sizeof(MusicChangeData), HSD_Free,
                             HSD_OBJKIND_JOBJ, set->jobj,
                             MusicChange_Think, 20,
                             JObj_GX, 21, gx_pri);

    // add animations
    JObj_AddSetAnim(g->hsd_object, 0, set, 0, 1);

    // init data
    MusicChangeData *gp = g->userdata;
    gp->state = MUSICCHANGE_SCROLLSTATE_NONE;
    gp->timer = 0;
    gp->offset = (Vec2){0, 0};
    gp->param = MusicChange_GetTextParam();

    stc_music_change_gobj = g;

    // create song text
    Text *t = Text_CreateText(0, music_text_canvas_idx);
    // t->viewport_color = (GXColor){0, 0, 0, 128};
    t->kerning = 1;
    t->viewport_scale = gp->param->scale;
    t->use_aspect = 1;
    t->aspect = gp->param->aspect;
    t->trans.X = gp->param->pos.X;
    t->trans.Y = gp->param->pos.Y;
    Text_AddSubtext(t, 0, 0, "");

    stc_text = t;

    // update song name
    MusicChange_UpdateSongName(gp);

    return g;
}
void MusicChange_Destroy()
{
    GObj_Destroy(stc_music_change_gobj);
    stc_music_change_gobj = 0;

    Text_Destroy(stc_text);
    stc_text = 0;

    return;
}
void MusicChange_Think(GOBJ *g)
{
    MusicChangeData *gp = g->userdata;

    JObj_AnimAll(g->hsd_object);

    // check to change song
    if (Pad_GetDown(Gm_GetGameData()->pause_ply) & PAD_TRIGGER_Z)
    {
        int is_changed = 0;
        MajorKind mj = Scene_GetCurrentMajor();

        int volume = (stc_event_global->is_song_playing) ? 0 : 255; // song comes in muted when an event is in progress

        if (mj == MJRKIND_AIR)
        {
            SongData_PlayRandomSong(volume);
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

            if (playlist_kind != 1)
            {
                int is_played = SongData_PlayFromPlaylist(playlist_kind, volume);

                if (!is_played)
                    SongData_PlayRandomSong(volume);
            }
        }

        SFX_Play(FGMMENU_CS_MV);

        // raise music volume
        if (volume > 0)
            BGM_RaiseVolume();

        // update song name
        MusicChange_UpdateSongName(gp);
    }

    // update text scroll logic
    Text *text = stc_text;
    switch (gp->state)
    {
    case MUSICCHANGE_SCROLLSTATE_STARTWAIT:
    {
        gp->timer--;

        if (gp->timer <= 0)
        {
            gp->state = MUSICCHANGE_SCROLLSTATE_MOVE;
        }
        break;
    }
    case MUSICCHANGE_SCROLLSTATE_MOVE:
    {
        gp->offset.X -= MUSICCHANGE_SCROLLSPEED;
        float scroll_amt = MusicChange_GetScrollAmount(text, gp->param->textbox_width);

        // check to stop
        if (gp->offset.X < -scroll_amt)
        {
            gp->offset.X = -scroll_amt;
            gp->state = MUSICCHANGE_SCROLLSTATE_STOPWAIT;
            gp->timer = 60 * 1.5;
        }
        break;
    }
    case MUSICCHANGE_SCROLLSTATE_STOPWAIT:
    {
        gp->timer--;

        // wait
        if (gp->timer <= 0)
        {
            // back to start
            gp->offset.X = 0;
            gp->state = MUSICCHANGE_SCROLLSTATE_STARTWAIT;
            gp->timer = 30;
        }
        break;
    }
    }

    // update text pos
    text->trans.X = gp->param->pos.X + gp->offset.X; // original pos + offset
}

void MusicChange_TextCObj(GOBJ *g)
{
    if (!CObj_SetCurrent(g->hsd_object))
        return;

    CObj_RenderGXLinks(g, (1 << 0) | (1 << 1) | (1 << 2));

    CObj_EndCurrent();
}

HUDLayoutKind HUD_GetLayout()
{
    int is_city = (Scene_GetCurrentMajor() == MJRKIND_CITY);

    switch (Gm_GetPlyViewNum())
    {
    case (0):
    case (1):
        if (is_city)
            return HUDLAYOUT_1P_CT;
        else
            return HUDLAYOUT_1P_AR;
    case (2):
        if (is_city)
            return HUDLAYOUT_2P_CT;
        else
            return HUDLAYOUT_2P_AR;
    case (3):
    case (4):
        if (is_city)
            return HUDLAYOUT_4P_CT;
        else
            return HUDLAYOUT_4P_AR;
    }
}
JOBJSet *MusicChange_GetJOBJSet()
{
    switch (HUD_GetLayout())
    {
    case (HUDLAYOUT_1P_AR):
    case (HUDLAYOUT_1P_CT):
        return music_assets->np_1;
    case (HUDLAYOUT_2P_AR):
        return music_assets->np_air_2;
    case (HUDLAYOUT_2P_CT):
        return music_assets->np_ct_2;
    case (HUDLAYOUT_4P_AR):
        return music_assets->np_air_4;
    case (HUDLAYOUT_4P_CT):
        return music_assets->np_ct_4;
    }
}
MusicChangeTextParams *MusicChange_GetTextParam()
{
    static MusicChangeTextParams music_change_param[] = {
        // 1p (air ride)
        {
            .pos = {1.1, 15.9},
            .scale = {0.045, 0.055},
            .aspect = {800, 32},
            .scissor_left = 330,
            .scissor_right = 555,
            .textbox_width = 20.0,
        },
        // 1p (city)
        {
            .pos = {1.1, 15.9},
            .scale = {0.045, 0.055},
            .aspect = {800, 32},
            .scissor_left = 330,
            .scissor_right = 555,
            .textbox_width = 20.0,
        },
        // 2p (air ride)
        {
            .pos = {-9, -4.2},
            .scale = {0.045, 0.055},
            .aspect = {950, 32},
            .scissor_left = 215,
            .scissor_right = 550,
            .textbox_width = 30.0,
        },
        // 2p (city)
        {
            .pos = {-20, -3.7},
            .scale = {0.045, 0.055},
            .aspect = {950, 32},
            .scissor_left = 100,
            .scissor_right = 420,
            .textbox_width = 29.0,
        },
        // 4p (air ride)
        {
            .pos = {-23.5, -3.25},
            .scale = {0.75 * 0.045, 0.75 * 0.055},
            .aspect = {1200, 32},
            .scissor_left = 70,
            .scissor_right = 465,
            .textbox_width = 37.0,
        },
        // 4p (city)
        {
            .pos = {-23.5, -3.25},
            .scale = {0.75 * 0.045, 0.75 * 0.055},
            .aspect = {1200, 32},
            .scissor_left = 70,
            .scissor_right = 465,
            .textbox_width = 37.0,
        },
    };

    return &music_change_param[HUD_GetLayout()];
}
float MusicChange_GetScrollAmount(Text *t, float textbox_width)
{
    // get text width
    float width = Text_GetWidth(t);

    if (width > textbox_width)
        return width - textbox_width;
    else
        return 0;
}
void MusicChange_UpdateSongName(MusicChangeData *gp)
{
    char *song_name = "None";
    u32 vpb_index = stc_bgm_data_arr[1].vpb_index;

    // check if no song is playing
    if (vpb_index != 63)
    {
        int cur_playing_entrynum = ax_live->voice_data[vpb_index].x30[2];
        SongData *sd = SongData_GetDataByEntrynum(cur_playing_entrynum);
        if (sd)
            song_name = sd->name;
        // else
        // {
        //     OSReport("vpb index %d, entrynum %d\n", vpb_index, cur_playing_entrynum);
        //     assert("music change");
        // }
    }

    // sanitize text
    char buf[200];
    Text_Sanitize(song_name, buf, sizeof(buf));

    // remove .hps
    char *extension_ptr = strstr(buf, ".hps");
    if (extension_ptr)
        extension_ptr[0] = '\0';

    // update song name
    Text_SetText(stc_text, 0, buf);

    float width = MusicChange_GetScrollAmount(stc_text, gp->param->textbox_width);
    if (width > 0)
        gp->state = MUSICCHANGE_SCROLLSTATE_STARTWAIT;
    else
        gp->state = MUSICCHANGE_SCROLLSTATE_NONE;

    gp->timer = 30;
    gp->offset.X = 0;

    // OSReport("text width %f\n", width);
}
void MusicChange_ScaleStats(int ply, float scale, Vec2 offsets)
{
    Game3dData *g3d = Gm_Get3dData();

    JOBJ *chart_jobj = g3d->cityui_statchart_gobj[ply]->hsd_object;
    chart_jobj->scale = (Vec3){scale,
                               scale,
                               scale};

    chart_jobj->trans.X += offsets.X;
    chart_jobj->trans.Y += offsets.Y;
    JObj_SetMtxDirtySub(chart_jobj);

    for (int i = 0; i < 9; i++)
    {
        GOBJ *bar_gobj = g3d->cityui_statbar_gobj[ply][i];
        JOBJ *bar_jobj = bar_gobj->hsd_object;
        HUDCityStatBarData *bar_data = bar_gobj->userdata;

        bar_jobj->scale = (Vec3){scale,
                                 scale,
                                 scale};

        JObj_GetChildPosition(chart_jobj, 1 + i, &bar_jobj->trans);
        JObj_SetMtxDirtySub(bar_jobj);
    }
}

float Text_GetWidth(Text *t)
{
    // get width as reported by text lib
    float width, height;
    Text_GetWidthAndHeight(t, &width, &height);

    // if using aspect, clamp width
    if (t->use_aspect && width > t->aspect.X)
        width = t->aspect.X;

    // return text width
    return (width * t->viewport_scale.X);
}
