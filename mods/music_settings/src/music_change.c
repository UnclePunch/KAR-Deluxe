
#include "text.h"
#include "useful.h"
#include "hsd.h"
#include "preload.h"
#include "scene.h"
#include "inline.h"
#include "audio.h"
#include "game.h"

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
    KARPlus_AddPreloadGameFile("IfAllNowPlaying");
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

            if (playlist_kind != 1)
            {
                int is_played = SongData_PlayFromPlaylist(playlist_kind);

                if (!is_played)
                    SongData_PlayRandomSong();
            }
        }

        SFX_Play(FGMMENU_CS_MV);

        // raise music volume
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

JOBJSet *MusicChange_GetJOBJSet()
{
    switch (Gm_GetPlyViewNum())
    {
    case (0):
    case (1):
        return music_assets->np_1;
    case (2):
        if (Gm_IsInCity())
            return music_assets->np_ct_2;
        else
            return music_assets->np_air_2;
    case (3):
    case (4):
        if (Gm_IsInCity())
            return music_assets->np_ct_4;
        else
            return music_assets->np_air_4;
    }
}
MusicChangeTextParams *MusicChange_GetTextParam()
{
    static MusicChangeTextParams music_change_param_air_1 = {
        .pos = {1.1, 15.9},
        .scale = {0.045, 0.055},
        .aspect = {800, 32},
        .scissor_left = 330,
        .scissor_right = 555,
        .textbox_width = 20.0,
    };
    static MusicChangeTextParams music_change_param_city_1 = {
        .pos = {1.1, 15.9},
        .scale = {0.045, 0.055},
        .aspect = {800, 32},
        .scissor_left = 331,
        .scissor_right = 555,
        .textbox_width = 20.0,
    };
    static MusicChangeTextParams music_change_param_air_2 = {
        .pos = {-9, -4.2},
        .scale = {0.045, 0.055},
        .aspect = {950, 32},
        .scissor_left = 215,
        .scissor_right = 550,
        .textbox_width = 30.0,
    };
    static MusicChangeTextParams music_change_param_city_2 = {
        .pos = {-20, -3.7},
        .scale = {0.045, 0.055},
        .aspect = {950, 32},
        .scissor_left = 100,
        .scissor_right = 420,
        .textbox_width = 29.0,
    };
    static MusicChangeTextParams music_change_param_air_4 = {
        .pos = {-23.5, -3.25},
        .scale = {0.75 * 0.045, 0.75 * 0.055},
        .aspect = {1200, 32},
        .scissor_left = 70,
        .scissor_right = 465,
        .textbox_width = 37.0,
    };
    static MusicChangeTextParams music_change_param_city_4 = {
        .pos = {-23.5, -3.25},
        .scale = {0.75 * 0.045, 0.75 * 0.055},
        .aspect = {1200, 32},
        .scissor_left = 70,
        .scissor_right = 465,
        .textbox_width = 37.0,
    };

    int ply_view_num = Gm_GetPlyViewNum();
    int is_in_city = Gm_IsInCity();

    // single player hud
    if (ply_view_num == 1)
    {
        if (is_in_city)
            return &music_change_param_city_1;
        else
            return &music_change_param_air_1;
    }
    // 2p splitscreen hud
    else if (ply_view_num == 2)
    {
        if (is_in_city)
            return &music_change_param_city_2;
        else
            return &music_change_param_air_2;
    }
    // 3p+ splitscreen hud
    else if (ply_view_num >= 3)
    {
        if (is_in_city)
            return &music_change_param_city_4;
        else
            return &music_change_param_air_4;
    }
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
    char *song_name;
    u32 vpb_index = stc_bgm_data_arr[1].vpb_index;

    if (vpb_index == 63)
        song_name = "None";
    else
    {
        int cur_playing_entrynum = ax_live->voice_data[vpb_index].x30[2];
        song_name = SongData_GetDataByEntrynum(cur_playing_entrynum)->name;
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
