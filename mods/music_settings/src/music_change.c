#include "text.h"
#include "os.h"
#include "preload.h"
#include "scene.h"
#include "inline.h"
#include "audio.h"
#include "game.h"
#include "hud.h"

#include "hoshi/func.h"

#include "music_change.h"
#include "music_database.h"
#include "fst/fst.h"
#include "text_joint/text_joint.h"

#include "../../wide/src/wide_export.h"

static GOBJ *stc_music_change_gobj;
static NowPlayingAssets *music_assets;
static int music_text_canvas_idx;

extern int stc_song_num;
extern SongData *stc_song_data;
extern PlaylistData stc_playlist_data[PLAYLIST_NUM];
extern WideExport *wide_export;

void MusicChange_Init()
{
    Hoshi_AddPreloadGameFile("IfAllNowPlaying", PRELOADHEAPKIND_ALLM);
}

void MusicChange_On3DLoad()
{
    HSD_Archive *archive;
    Gm_LoadGameFile(&archive, "IfAllNowPlaying");
    music_assets = Archive_GetPublicAddress(archive, "NowPlaying_scene_models");

    music_text_canvas_idx = Text_CreateCanvas(0, -1, 28, GAMEPLINK_CAMHUD, 0, GAMEGX_HUD, 3, 0);
}

void MusicChange_OnTextDraw(GOBJ *g)
{
    GXSetZMode(GX_ENABLE, GX_GEQUAL, GX_DISABLE);
}
GOBJ *MusicChange_Create()
{
    JOBJSet *set = MusicChange_GetJOBJSet();

    // air ride 1p needs to render in front of the hud
    int is_airride_1p = Gm_Get3dData()->plyview_num == 1 && !Gm_IsInCity();
    int gx_pri = (is_airride_1p) ? 2 : 1;

    // create hud element gobj
    GOBJ *g = GOBJ_EZCreator(27, GAMEPLINK_PAUSEHUD, 0,
                             sizeof(MusicChangeData), MusicChange_OnDestroy,
                             HSD_OBJKIND_JOBJ, set->jobj,
                             MusicChange_Think, 20,
                             JObj_GX, GAMEGX_HUD, gx_pri);

    // add animations
    JOBJ *j = g->hsd_object;
    JObj_AddSetAnim(j, 0, set, 0, 1);

    // widescreen shift
    if (Gm_Get3dData()->plyview_num == 1 && wide_export)
        wide_export->HUDAdjust_Element(g, 0, false, WIDEALIGN_RIGHT);

    Vec3 wide_offset = {0, 0, 0};
    JObj_GetChildPosition(j, 0, &wide_offset);

    // init data
    MusicChangeData *gp = g->userdata;
    gp->state = MUSICCHANGE_SCROLLSTATE_NONE;
    gp->timer = 0;
    gp->scroll_offset = (Vec2){0, 0};
    gp->param = MusicChange_GetTextParam();

    Vec3 text_pos = {0, 0, 0};
    JObj_GetChildPosition(j, gp->param->joint_text_pos_idx, &text_pos);
    gp->pos = (Vec2){text_pos.X, -text_pos.Y};
    OSReport("text pos: %0.2f, %0.2f\n", text_pos.X, -text_pos.Y);

    stc_music_change_gobj = g;

    // create song text
    Text *t = Text_CreateText(0, music_text_canvas_idx);
    // t->viewport_color = (GXColor){0, 0, 0, 128};
    t->render_callback = MusicChange_OnTextDraw;
    t->kerning = 1;
    t->viewport_scale = gp->param->scale;
    t->use_aspect = 1;
    t->aspect = gp->param->aspect;
    t->trans.X = gp->pos.X;
    t->trans.Y = gp->pos.Y;
    t->trans.Z = -0.1;
    Text_AddSubtext(t, 0, 0, "");

    gp->text = t;

    // update song name
    MusicChange_UpdateSongName(gp);

    return g;
}
void MusicChange_OnDestroy(MusicChangeData *gp)
{
    Text_Destroy(gp->text);

    HSD_Free(gp);

    return;
}
void MusicChange_Think(GOBJ *g)
{
    MusicChangeData *gp = g->userdata;

    JObj_AnimAll(g->hsd_object);

    // check to change song
    if (Pad_GetDown(Gm_GetGameData()->pause_ply) & PAD_TRIGGER_Z)
    {   
        if (SongData_PlayRandomStageSong())
        {
            SFX_Play(FGMMENU_CS_MV);

            // update song name
            MusicChange_UpdateSongName(gp);
        }
    }

    // update text scroll logic
    Text *text = gp->text;
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
        gp->scroll_offset.X -= MUSICCHANGE_SCROLLSPEED;
        float scroll_amt = MusicChange_GetScrollAmount(text, gp->param->textbox_width);

        // check to stop
        if (gp->scroll_offset.X < -scroll_amt)
        {
            gp->scroll_offset.X = -scroll_amt;
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
            gp->scroll_offset.X = 0;
            gp->state = MUSICCHANGE_SCROLLSTATE_STARTWAIT;
            gp->timer = 30;
        }
        break;
    }
    }


    // update text pos
    text->trans.X = gp->pos.X + gp->scroll_offset.X; // original pos + offset
}
void MusicChange_Destroy()
{
    GObj_Destroy(stc_music_change_gobj);
    stc_music_change_gobj = 0;
    return;
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
            .scale = {0.045, 0.055},
            .aspect = {800, 32},
            .joint_text_pos_idx = 6,
            .textbox_width = 20.0,
        },
        // 1p (city)
        {
            .scale = {0.045, 0.055},
            .aspect = {800, 32},
            .joint_text_pos_idx = 6,
            .textbox_width = 20.0,
        },
        // 2p (air ride)
        {
            .scale = {0.045, 0.055},
            .aspect = {950, 32},
            .joint_text_pos_idx = 6,
            .textbox_width = 30.0,
        },
        // 2p (city)
        {
            .scale = {0.045, 0.055},
            .aspect = {950, 32},
            .joint_text_pos_idx = 6,
            .textbox_width = 29.0,
        },
        // 4p (air ride)
        {
            .scale = {0.75 * 0.045, 0.75 * 0.055},
            .aspect = {1200, 32},
            .joint_text_pos_idx = 5,
            .textbox_width = 37.0,
        },
        // 4p (city)
        {
            .scale = {0.75 * 0.045, 0.75 * 0.055},
            .aspect = {1200, 32},
            .joint_text_pos_idx = 5,
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
    u32 vpb_index = stc_bgm_pid[1] & AXDRIVER_PIDMASK;

    // check if no song is playing
    if (vpb_index != AXDRIVER_PIDMASK)
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
    Text_SetText(gp->text, 0, buf);

    float width = MusicChange_GetScrollAmount(gp->text, gp->param->textbox_width);
    if (width > 0)
        gp->state = MUSICCHANGE_SCROLLSTATE_STARTWAIT;
    else
        gp->state = MUSICCHANGE_SCROLLSTATE_NONE;

    gp->timer = 30;
    gp->scroll_offset.X = 0;

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
    Text_GetWidthAndHeight(t, 0, &width, &height);

    // if using aspect, clamp width
    if (t->use_aspect && width > t->aspect.X)
        width = t->aspect.X;

    // return text width
    return (width * t->viewport_scale.X);
}
