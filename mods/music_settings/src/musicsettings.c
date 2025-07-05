
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
#include "patch.h"
#include "fst/fst.h"
#include "text_joint/text_joint.h"

static MusicSettingsSave *music_save;
HSD_Archive *music_prompt_archive;

MajorSceneDesc major_desc = {
    .major_id = -1,
    .next_major_id = 0,
    .initial_minor_id = -1,
    .cb_Enter = Music_MajorEnter,
    .cb_ExitMinor = Music_MajorExit,
};
MinorSceneDesc minor_scene = {
    .idx = -1,
    .x1 = -1,
    .cb_Load = MusicSettings_Enter,
    .cb_Exit = MusicSettings_Exit,
    .cb_ThinkPreGObjProc = MusicSettings_Think,
    .preload_kind = 4, // 4 will preserve menu file preloads
};

extern int stc_song_num;
extern SongData *stc_song_data;
extern PlaylistData stc_playlist_data[PLAYLIST_NUM];

static struct PlaylistDefault playlist_defaults[] = {
    {.playlist = PLAYLIST_MENU, .entry = 0, .song_name = "Menu"},
    {.playlist = PLAYLIST_CITY, .entry = 0, .song_name = "City"},
    {.playlist = PLAYLIST_CITY, .entry = 1, .song_name = "City (Backside)"},
    {.playlist = PLAYLIST_CITYHURRY, .entry = 0, .song_name = "City: Not Much Time Left"},
    {.playlist = PLAYLIST_CITYGRAPH, .entry = 0, .song_name = "Properties Graph"},
    {.playlist = PLAYLIST_RESULTS, .entry = 0, .song_name = "Air Ride Results"},
    {.playlist = PLAYLIST_RESULTS, .entry = 1, .song_name = "Top Ride Results"},
    {.playlist = PLAYLIST_HIGHJUMP, .entry = 0, .song_name = "Air Glider"},
    {.playlist = PLAYLIST_TARGETFLIGHT, .entry = 0, .song_name = "Target Flight"},
    {.playlist = PLAYLIST_DESTRUCTIONDERBY, .entry = 0, .song_name = "Drag Race"},
    {.playlist = PLAYLIST_AIRGLIDER, .entry = 0, .song_name = "Air Glider"},
    {.playlist = PLAYLIST_DRAGRACE, .entry = 0, .song_name = "Drag Race"},
    {.playlist = PLAYLIST_KIRBYMELEE, .entry = 0, .song_name = "Kirby Melee"},
};

MajorKind MusicSettings_Init()
{
    SongData_Init();
    Patches_Apply();

    minor_scene.idx = KARPlus_InstallMinorScene(&minor_scene);
    major_desc.initial_minor_id = minor_scene.idx;

    major_desc.major_id = KARPlus_InstallMajorScene(&major_desc);

    return major_desc.major_id;
}
void MusicSettings_SaveInit(MusicSettingsSave *save, int req_init)
{
    music_save = save; // save ptr to save data

    // init playlist values on save creation
    if (req_init)
    {
        // null all entries on all playlists
        for (int i = 0; i < GetElementsIn(save->playlist); i++)
        {
            for (int j = 0; j < GetElementsIn(save->playlist->song_hash); j++)
                save->playlist[i].song_hash[j] = -1;

            stc_playlist_data[i].mode = PLAYLISTMODE_ORIGINAL;
        }

        // set defaults
        for (int i = 0; i < GetElementsIn(playlist_defaults); i++)
        {
            MusicSettingsPlaylist playlist = playlist_defaults[i].playlist;
            int entry = playlist_defaults[i].entry;
            char *song_name = playlist_defaults[i].song_name;

            save->playlist[playlist].song_hash[entry] = SongData_GetDataByName(song_name)->hash;
        }
    }

    // convert hash to local id
    MusicSettings_CopyFromSave();
}
void MainMenu_LoadMusicPrompt()
{
    music_prompt_archive = Archive_LoadFile("MnSelMusicPrompt");
}

// Save Functions
void MusicSettings_CopyFromSave()
{

    OSReport("Copying playlist data from save...\n");

    // convert hashes to local id
    for (int playlist_idx = 0; playlist_idx < PLAYLIST_NUM; playlist_idx++)
    {
        // OSReport("Playlist %d:\n", playlist_idx);

        // store mode
        stc_playlist_data[playlist_idx].mode = music_save->playlist[playlist_idx].mode;

        // OSReport("  Mode: %d\n", stc_playlist_data[playlist_idx].mode);

        int song_num = 0;
        for (int song_idx = 0; song_idx < SONGS_PER_PLAYLIST; song_idx++)
        {
            int hash = music_save->playlist[playlist_idx].song_hash[song_idx]; // get hash from save

            // OSReport("  hash: %x\n", hash);

            // skip for null hashes
            if (hash == -1)
                continue;

            int is_found = 0;

            // search for local database for hash from save
            for (int i = 0; i < stc_song_num; i++)
            {
                if (hash == stc_song_data[i].hash) // find match
                {
                    // set song id
                    stc_playlist_data[playlist_idx].songs[song_num] = i;
                    song_num++;
                    is_found = 1;
                    break;
                }
            }

            if (!is_found)
            {
                OSReport("ERROR: Playlist %d Song Entry %d with hash 0x%08x not found!\n", playlist_idx, song_idx, hash);
            }
        }

        // update total song num
        if (song_num > 0)
            stc_playlist_data[playlist_idx].song_num = song_num;
        else
        {
            OSReport("ERROR: No songs found on playlist %d, restoring defaults.\n", playlist_idx);

            // no songs on playlist, use defaults
            stc_playlist_data[playlist_idx].mode = PLAYLISTMODE_ORIGINAL;

            // set defaults
            for (int def_idx = 0; def_idx < GetElementsIn(playlist_defaults); def_idx++)
            {
                if (playlist_defaults[def_idx].playlist == playlist_idx)
                {
                    char *song_name = playlist_defaults[def_idx].song_name;
                    stc_playlist_data[playlist_idx].songs[stc_playlist_data[playlist_idx].song_num] = SongData_GetIndexByName(song_name);
                    stc_playlist_data[playlist_idx].song_num++;
                }
            }
            stc_playlist_data[playlist_idx].mode = PLAYLISTMODE_ORIGINAL;
        }

        // OSReport("\n");
    }
}
void MusicSettings_CopyToSave()
{
    OSReport("Copying playlist data to save...\n");

    // convert hashes to local id
    for (int playlist_idx = 0; playlist_idx < PLAYLIST_NUM; playlist_idx++)
    {
        // OSReport("Playlist %d:\n", playlist_idx);

        // store mode
        music_save->playlist[playlist_idx].mode = stc_playlist_data[playlist_idx].mode;

        // OSReport("  Mode: %d\n", stc_playlist_data[playlist_idx].mode);

        for (int song_idx = 0; song_idx < stc_playlist_data[playlist_idx].song_num; song_idx++)
        {
            int database_song_idx = stc_playlist_data[playlist_idx].songs[song_idx];
            int song_hash = stc_song_data[database_song_idx].hash;

            // OSReport("  hash: %x\n", song_hash);

            // store hash
            music_save->playlist[playlist_idx].song_hash[song_idx] = song_hash;
        }

        // OSReport("\n");
    }

    if (KARPlus_WriteSave() == CARD_RESULT_READY)
        OSReport("Saved music to card.\n");
}

// Major and Minor Functions
void Music_MajorEnter()
{
}
void Music_MajorExit()
{
}
void MusicSettings_Enter()
{
    MusicSettings_Create();
}
void MusicSettings_Exit(void *data)
{
    MusicSettings_CopyToSave();

    // MainMenu_InitAllVariables();
    GameData *gd = Gm_GetGameData();
    gd->main_menu.top_menu = MAINMENU_TOPMENU_OPTIONS;
    gd->main_menu.is_in_submenu = 1;
    gd->main_menu.cursor_val[0] = 1;
    gd->main_menu.cursor_val[1] = 0;

    Scene_SetNextMajor(MJRKIND_MENU);
    Scene_ExitMajor();
}
void MusicSettings_Think(void *data)
{
}

// Scene
static HSD_Archive *settings_archive = 0;
static HSD_SObjDesc *stc_sobj;
void MusicSettings_Create()
{
    KARPlus_AddPreloadMenuFile("MnSelMusicAll");
    Preload_Invalidate();
    Preload_Update();

    Gm_LoadGameFile(&settings_archive, "MnSelMusicAll");
    stc_sobj = Archive_GetPublicAddress(settings_archive, "ScMenSelMusic_scene_data");

    // create text canvas
    // GOBJ *textcam_g = MusicSettings_TextCamCreate(stc_sobj->cobjdesc[0]);
    // CObj_SetScissor(textcam_g->hsd_object, 0, 640, 0, 480);
    stc_scene_menu_common->canvas_idx = Text_CreateCanvas(0, -1, 41, 17, 0, MENUGX_SONGNAME, 1, -1);
    TextJoint_Init(stc_scene_menu_common->canvas_idx);

    // create cam
    GOBJ *gc = MusicSettings_CamCreate(stc_sobj->cobjdesc[0]);

    // create lights
    GOBJ *gl = GOBJ_EZCreator(38, 32, 0,
                              0, 0,
                              HSD_OBJKIND_LOBJ, stc_sobj->lights,
                              0, 0,
                              LObj_GX, MENUGX_0, 0);

    // create bg jobj
    GOBJ *bg_g = MusicSettings_BGCreate();

    // create UI
    GOBJ *ui_g = MusicSettings_UICreate();
    JObj_AttachPositionRotation(ui_g->hsd_object, JObj_GetIndex(bg_g->hsd_object, 22));

    // SongData_TestInit();
}
void MusicSettings_Destroy()
{
}

// Camera
static COBJDesc *stc_cobj_desc = 0;
GOBJ *MusicSettings_CamCreate(COBJDesc *desc)
{
    stc_cobj_desc = desc;

    GOBJ *gc = GOBJ_EZCreator(40, 17, 0,
                              0, 0,
                              HSD_OBJKIND_COBJ, desc,
                              MusicSettings_CameraThink, 0,
                              MusicSettings_CamGX, 0, 4);
    gc->cobj_links = (1ULL << MENUGX_0) | (1ULL << MENUGX_1) | (1ULL << MENUGX_2);

    return gc;
}
void MusicSettings_CamGX(GOBJ *g)
{
    COBJ *c = g->hsd_object;

    if (!CObj_SetCurrent(c))
        return;

    // set background color
    GXColor bg_color = stc_sobj->fog[0]->color;
    CObj_SetEraseColor(bg_color.r, bg_color.g, bg_color.b, 255);
    CObj_EraseScreen(c, GX_ENABLE, GX_ENABLE, GX_ENABLE);

    // render UI
    g->cobj_links = (1ULL << MENUGX_0) | (1ULL << MENUGX_1) | (1ULL << MENUGX_2);
    CObj_RenderGXLinks(g, (1 << 0) | (1 << 1) | (1 << 2));

    // render Text
    g->cobj_links = (1ULL << MENUGX_SONGNAME);
    CObj_RenderGXLinks(g, (1 << 0) | (1 << 1) | (1 << 2));

    CObj_EndCurrent();

    return;
}
void MusicSettings_CameraThink(GOBJ *gobj) // 8022BA1C
{

#define CAM_STICKDEADZONE 0.4 // minimum magnitude per axis to move camera
#define CAM_MAXROTATEDEG 30   // max amount to rotate the camera in degrees
    Vec2 rotate_amt = {0, 0}; //

    // find the first pad out of the deadzone 8022ba58
    for (int i = 0; i < 4; i++)
    {

        HSD_Pad *this_pad = &stc_engine_pads[i];

        // check if out of deadzone, this code sucks
        if (this_pad->fsubstickX >= CAM_STICKDEADZONE)
            rotate_amt.X = (this_pad->fsubstickX - CAM_STICKDEADZONE) / (1 - CAM_STICKDEADZONE) * CAM_MAXROTATEDEG;
        else if (this_pad->fsubstickX <= -CAM_STICKDEADZONE)
            rotate_amt.X = (this_pad->fsubstickX - -CAM_STICKDEADZONE) / (1 - CAM_STICKDEADZONE) * CAM_MAXROTATEDEG;
        if (this_pad->fsubstickY >= CAM_STICKDEADZONE)
            rotate_amt.Y = (this_pad->fsubstickY - CAM_STICKDEADZONE) / (1 - CAM_STICKDEADZONE) * CAM_MAXROTATEDEG;
        else if (this_pad->fsubstickY <= -CAM_STICKDEADZONE)
            rotate_amt.Y = (this_pad->fsubstickY - -CAM_STICKDEADZONE) / (1 - CAM_STICKDEADZONE) * CAM_MAXROTATEDEG;

        if (rotate_amt.X != 0 || rotate_amt.Y != 0)
            break;
    }

    Vec3 cobj_eye;             // 72
    Vec3 cobj_interest;        // 24
    Vec3 cross;                // 60
    Vec3 identity = {0, 1, 0}; // 48
    Mtx m;                     // 84
    Vec3 final_eye;            // 36

    // rotate camera 8022bbe0
    COBJ_Init(gobj->hsd_object, stc_cobj_desc);
    COBJ_GetEyeVector(gobj->hsd_object, &cobj_eye);
    COBJ_GetInterest(gobj->hsd_object, &cobj_interest);
    VECCrossProduct(&identity, &cobj_eye, &cross);
    VECNormalize(&cross, &cross);

    // Y axis
    C_MTXRotAxisRad(&m, &cross, rotate_amt.Y * M_1DEGREE);
    MTXMultVec(&m, &cobj_eye, &cobj_eye);

    // X axis
    C_MTXRotAxisRad(&m, &identity, rotate_amt.X * M_1DEGREE);
    MTXMultVec(&m, &cobj_eye, &cobj_eye);

    // scale mtx
    VECScale(&cobj_eye,
             &cobj_eye,
             COBJ_GetEyeDistance(gobj->hsd_object));

    VECSubtract(&cobj_interest, &cobj_eye, &final_eye);
    CObj_SetEyePosition(gobj->hsd_object, &final_eye);

    return;
}

// Background
static GOBJ *stc_bg_gobj = 0;
GOBJ *MusicSettings_BGCreate()
{
    // create background
    JOBJSet **set = Archive_GetPublicAddress(settings_archive, "ScMenSelMusicBg_scene_models");
    GOBJ *g = JObj_LoadSet_SetPri(0, set[0], 0, 0, 30, MENUGX_1, 1, MusicSettings_BGThink, 0);

    // init data
    MusicSettingsBackgroundData *gp = HSD_MemAlloc(sizeof(MusicSettingsBackgroundData));
    GObj_AddUserData(g, 4, HSD_Free, gp);

    // create description text
    Text *text = TextJoint_Create(g->hsd_object,
                                  24,
                                  stc_scene_menu_common->canvas_idx,
                                  "",
                                  0,
                                  930, 32,
                                  &(GXColor){204, 204, 204, 255});
    gp->description_text = text;

    stc_bg_gobj = g;

    return g;
}
void MusicSettings_BGThink(GOBJ *g)
{
    MusicSettingsBackgroundData *gp = g->userdata;
    JOBJ *j = g->hsd_object;

    JObj_AnimAll(j);

    // when animation ends, play the next one and loop it
    if (!JObj_CheckAObjPlaying(j))
    {
        JOBJSet **bg = Archive_GetPublicAddress(settings_archive, "ScMenSelMusicBg_scene_models");

        JObj_AddSetAnim(j, 1, bg[0], 0, 1);
        JObj_SetAllAOBJLoopByFlags(j, 0xFFFF);
        // settings_data.is_intro_anim = 0;
    }

    return;
}
void MusicSettings_SetDescription(char *str, ...)
{
    MusicSettingsBackgroundData *gp = stc_bg_gobj->userdata;

    Text_SetText(gp->description_text, 0, str);
}

// UI
GOBJ *MusicSettings_UICreate()
{
    JOBJSet **pos_jobjset = Archive_GetPublicAddress(settings_archive, "ScMenSelMusicPos_scene_models");

    // create gobj
    GOBJ *g = GOBJ_EZCreator(38, 32, 0,
                             sizeof(MusicSettingsData), MusicSettings_UIDestroy,
                             HSD_OBJKIND_JOBJ, pos_jobjset[0]->jobj,
                             MusicSettings_UIThink, 0,
                             MusicSettings_UIGX, MENUGX_1, 0);

    MusicSettingsData *gp = g->userdata;

    JOBJ *j;
    JOBJSet **jset;

    JOBJ *attach_to_gamemode = JObj_GetIndex(g->hsd_object, 6);
    JOBJ *attach_to_playlistmode = JObj_GetIndex(g->hsd_object, 11);
    JOBJ *attach_to_playlistentry = JObj_GetIndex(g->hsd_object, 12);

    gp->game_mode.arrowup_j = JObj_GetIndex(g->hsd_object, 4);
    gp->game_mode.arrowdown_j = JObj_GetIndex(g->hsd_object, 5);
    gp->playlist.arrowup_j = JObj_GetIndex(g->hsd_object, 9);
    gp->playlist.arrowdown_j = JObj_GetIndex(g->hsd_object, 10);

    // create game mode buttons
    jset = Archive_GetPublicAddress(settings_archive, "ScMenSelMusicPlaylistNameButton_scene_models");
    for (int i = 0; i < GetElementsIn(gp->game_mode.button); i++)
    {
        // create root jobj for both
        JOBJ *root = JObj_Alloc();
        JObj_AddNext(attach_to_gamemode, root); // add to menu gobj
        gp->game_mode.button[i].j = j;          //

        // create button background
        j = JObj_LoadJoint(jset[0]->jobj);        // create jobj
        JObj_AddSetAnim(j, 0, jset[0], 0, 0);     // add anims
        JObj_AddNext(root, j);                    // add to menu gobj
        gp->game_mode.button[i].background_j = j; //

        // create button name
        j = JObj_LoadJoint(jset[1]->jobj);                    // create jobj
        JObj_AddSetAnim(j, 0, jset[1], 0, 0);                 // add anims
        JObj_AddNext(root, j);                                // add to menu gobj
        gp->game_mode.button[i].name_j = JObj_GetIndex(j, 1); //

        // position
        float spacing = 5.3;
        float offset = (GetElementsIn(gp->game_mode.button) - 1) * spacing / 2.0f;
        root->trans.Y = offset - (float)i * spacing;

        gp->game_mode.button_background_set = jset;
    }

    // create playlist mode button
    jset = Archive_GetPublicAddress(settings_archive, "ScMenSelMusicPlaylistModeButton_scene_models");
    // create frame jobj
    j = JObj_LoadJoint(jset[0]->jobj);                     // create jobj
    JObj_AddSetAnim(j, 0, jset[0], 0, 0);                  // add anims
    JObj_AddNext(attach_to_playlistmode, j);               // add to menu gobj
    gp->playlist.mode_button.j = j;                        //
    gp->playlist.mode_button.fill_j = JObj_GetIndex(j, 4); //
    gp->playlist.mode_button.name_j = JObj_GetIndex(j, 7); //
    // create cursor jobj
    jset = Archive_GetPublicAddress(settings_archive, "ScMenSelMusicPlaylistModeCursor_scene_models");
    j = JObj_LoadJoint(jset[0]->jobj);       // create jobj
    JObj_AddSetAnim(j, 0, jset[0], 0, 1.0);  // add anims
    JObj_AddNext(attach_to_playlistmode, j); // add to menu gobj
    gp->playlist.mode_button.cursor_j = j;   //

    // create playlist entry buttons
    jset = Archive_GetPublicAddress(settings_archive, "ScMenSelMusicSongEntry_scene_models");
    for (int i = 0; i < GetElementsIn(gp->playlist.entry_button); i++)
    {
        // create jobj
        j = JObj_LoadJoint(jset[0]->jobj);                              // create jobj
        JObj_AddSetAnim(j, 0, jset[0], 0, 0);                           // add anims
        JObj_AddNext(attach_to_playlistentry, j);                       // add to menu gobj
        gp->playlist.entry_button[i].j = j;                             //
        gp->playlist.entry_button[i].songname_j = JObj_GetIndex(j, 3);  //
        gp->playlist.entry_button[i].fill_j = JObj_GetIndex(j, 4);      //
        gp->playlist.entry_button[i].songnum_j = JObj_GetIndex(j, 5);   //
        gp->playlist.entry_button[i].cursorpos_j = JObj_GetIndex(j, 6); //
        gp->playlist.entry_button[i].deletepos_j = JObj_GetIndex(j, 7); //

        // position
        float spacing = 6.2;
        float offset = (GetElementsIn(gp->playlist.entry_button) - 1) * spacing / 2.0f;
        j->trans.Y = offset - (float)i * spacing;

        Text *text;

        // create song text
        text = TextJoint_Create(gp->playlist.entry_button[i].songname_j,
                                0,
                                stc_scene_menu_common->canvas_idx,
                                "Hello World",
                                0,
                                SONG_ASPECT_X, 32,
                                &(GXColor){0, 0, 0, 255});
        text->is_depth_compare = 1; // required to be occluded by the plane
        gp->playlist.entry_button[i].songname_text = text;

        // create song number text
        text = TextJoint_Create(gp->playlist.entry_button[i].songnum_j,
                                0,
                                stc_scene_menu_common->canvas_idx,
                                "10",
                                1,
                                32, 32,
                                &(GXColor){0, 0, 0, 255});
        gp->playlist.entry_button[i].songnum_text = text;
    }

    // create playlist entry add icon
    jset = Archive_GetPublicAddress(settings_archive, "ScMenSelMusicSongEntryAdd_scene_models");
    j = JObj_LoadJoint(jset[0]->jobj);        // create jobj
    JObj_AddSetAnim(j, 0, jset[0], 0, 0);     // add anims
    JObj_SetFrameAndRate(j, 0, 0);            //
    JObj_AddNext(attach_to_playlistentry, j); // add to menu gobj
    gp->playlist.add_button.j = j;

    // create playlist entry cursor
    jset = Archive_GetPublicAddress(settings_archive, "ScMenSelMusicSongEntryCursor_scene_models");
    j = JObj_LoadJoint(jset[0]->jobj);    // create jobj
    JObj_AddSetAnim(j, 0, jset[0], 0, 0); // add anims
    JObj_SetFrameAndRate(j, 0, 1);        //
    JObj_AddNext(g->hsd_object, j);       // add to menu gobj
    gp->playlist.cursor_j = j;

    // create playlist entry delete icon
    jset = Archive_GetPublicAddress(settings_archive, "ScMenSelMusicSongEntryDeleteIcon_scene_models");
    j = JObj_LoadJoint(jset[0]->jobj);    // create jobj
    JObj_AddSetAnim(j, 0, jset[0], 0, 0); // add anims
    JObj_SetFrameAndRate(j, 0, 0);        //
    JObj_AddNext(g->hsd_object, j);       // add to menu gobj
    gp->playlist.delete_button.j = j;
    JObj_SetFlagsAll(j, JOBJ_HIDDEN);

    // create occlusion plane
    JOBJSet **occlude_set = Archive_GetPublicAddress(settings_archive, "ScMenSelMusicOcclude_scene_models");
    GOBJ *occlude_g = GOBJ_EZCreator(38, 32, 0,
                                     0, 0,
                                     HSD_OBJKIND_JOBJ, occlude_set[0]->jobj,
                                     0, 0,
                                     JObj_GX, MENUGX_SONGNAME, 0);

    gp->focus = MENUFOCUS_GAMEMODE;
    gp->game_mode.cursor = 0;
    gp->game_mode.scroll = 0;
    gp->playlist.cursor = 0;
    gp->playlist.scroll = 0;

    MusicSettings_UIUpdate(g);

    return g;
}
void MusicSettings_UIDestroy(MusicSettingsData *gp)
{
    HSD_Free(gp);
    return;
}
void MusicSettings_UIGX(GOBJ *g, int pass)
{
    JObj_GX(g, pass);
}
void MusicSettings_UIThink(GOBJ *g)
{
    JOBJ *j = g->hsd_object;
    MusicSettingsData *gp = g->userdata;

    // inputs
    MusicSettings_UIInput(g);

    // scroll text for highlighted song
    if (gp->focus == MENUFOCUS_PLAYLISTENTRIES)
    {
        Text *text = gp->playlist.entry_button[gp->playlist.cursor].songname_text;

        switch (gp->playlist.text_scroll.state)
        {
        case SONGSCROLL_STARTWAIT:
        {
            gp->playlist.text_scroll.timer--;

            if (gp->playlist.text_scroll.timer <= 0)
            {
                gp->playlist.text_scroll.state = SONGSCROLL_MOVE;
            }
            break;
        }
        case SONGSCROLL_MOVE:
        {
            text->trans.X -= SONG_SCROLL_SPEED;
            float scroll_amt = SongScroll_GetAmount(text);

            // check to stop
            if (text->trans.X < -scroll_amt)
            {
                text->trans.X = -scroll_amt;
                gp->playlist.text_scroll.state = SONGSCROLL_STOPWAIT;
                gp->playlist.text_scroll.timer = 60 * 1.5;
            }
            break;
        }
        case SONGSCROLL_STOPWAIT:
        {
            gp->playlist.text_scroll.timer--;

            // wait
            if (gp->playlist.text_scroll.timer <= 0)
            {
                // back to start
                text->trans.X = 0;
                gp->playlist.text_scroll.state = SONGSCROLL_STARTWAIT;
                gp->playlist.text_scroll.timer = 30;
            }
            break;
        }
        }
    }

    JObj_AnimAll(j);
    JObj_SetMtxDirtySub(j);

    return;
}
void MusicSettings_UIInput(GOBJ *g)
{

    MusicSettingsData *gp = g->userdata;
    int is_update = UPDATEKIND_NONE;

    // inputs
    int down = Pad_GetDown(20);
    int rapid = Pad_GetRapidHeld(20);

    // check inputs
    switch (gp->focus)
    {
    case (MENUFOCUS_GAMEMODE):
    {
        int scroll_max = (PLAYLIST_NUM - GAMEMODEBUTTON_NUM >= 0) ? (PLAYLIST_NUM - GAMEMODEBUTTON_NUM) : 0;
        int cursor_max = GAMEMODEBUTTON_NUM - 1;

        if (rapid & PAD_BUTTON_UP)
        {
            if (gp->game_mode.cursor == 0)
            {
                if (gp->game_mode.scroll > 0) // try to scroll up
                {
                    gp->game_mode.scroll--;
                    is_update = UPDATEKIND_CHANGE;
                }
                else // loop?
                {
                    gp->game_mode.scroll = scroll_max;
                    gp->game_mode.cursor = cursor_max;
                    is_update = UPDATEKIND_CHANGE;
                }
            }
            else
            {
                gp->game_mode.cursor--;
                is_update = UPDATEKIND_CHANGE;
            }
        }
        else if (rapid & PAD_BUTTON_DOWN)
        {
            if (gp->game_mode.cursor == cursor_max)
            {
                if (gp->game_mode.scroll < scroll_max) // try to scroll down
                {
                    gp->game_mode.scroll++;
                    is_update = UPDATEKIND_CHANGE;
                }
                else // loop?
                {
                    gp->game_mode.scroll = 0;
                    gp->game_mode.cursor = 0;
                    is_update = UPDATEKIND_CHANGE;
                }
            }
            else
            {
                gp->game_mode.cursor++;
                is_update = UPDATEKIND_CHANGE;
            }
        }
        else if (down & PAD_BUTTON_A)
        {
            is_update = UPDATEKIND_ADVANCE;
            gp->focus = MENUFOCUS_PLAYLISTMODE;
        }
        else if (down & PAD_BUTTON_B)
        {
            is_update = UPDATEKIND_REGRESS;
            Scene_ExitMinor();
        }

        break;
    }
    case (MENUFOCUS_PLAYLISTMODE):
    {
        int playlist_idx = gp->game_mode.cursor + gp->game_mode.scroll;
        int cursor_max = PLAYLISTMODE_NUM - 1;

        if (rapid & PAD_BUTTON_LEFT)
        {
            if (stc_playlist_data[playlist_idx].mode == 0)
            {
                stc_playlist_data[playlist_idx].mode = cursor_max;
                is_update = UPDATEKIND_CHANGE;
            }
            else
            {
                stc_playlist_data[playlist_idx].mode--;
                is_update = UPDATEKIND_CHANGE;
            }
        }
        else if (rapid & PAD_BUTTON_RIGHT)
        {
            if (stc_playlist_data[playlist_idx].mode == cursor_max)
            {
                stc_playlist_data[playlist_idx].mode = 0;
                is_update = UPDATEKIND_CHANGE;
            }
            else
            {
                stc_playlist_data[playlist_idx].mode++;
                is_update = UPDATEKIND_CHANGE;
            }
        }
        else if (rapid & (PAD_BUTTON_DOWN | PAD_BUTTON_A))
        {
            if (stc_playlist_data[playlist_idx].mode == PLAYLISTMODE_PLAYLIST)
            {
                is_update = UPDATEKIND_CHANGE;
                gp->focus = MENUFOCUS_PLAYLISTENTRIES;
            }
        }
        else if (down & PAD_BUTTON_B)
        {
            is_update = UPDATEKIND_REGRESS;
            gp->focus = MENUFOCUS_GAMEMODE;
        }

        break;
    }
    case (MENUFOCUS_PLAYLISTENTRIES):
    {
        int playlist_song_entry = gp->playlist.scroll + gp->playlist.cursor;
        MusicSettingsPlaylist playlist_kind = gp->game_mode.scroll + gp->game_mode.cursor;
        int song_num = stc_playlist_data[playlist_kind].song_num;
        int scroll_max = ((song_num + 1) - PLAYLISTENTRY_NUM >= 0) ? ((song_num + 1) - PLAYLISTENTRY_NUM) : 0;
        int cursor_max = (song_num < PLAYLISTENTRY_NUM) ? (song_num) : (PLAYLISTENTRY_NUM - 1);
        int is_selected_add_btn = (gp->playlist.cursor == cursor_max && gp->playlist.scroll == scroll_max);

        // OSReport("song_num %d, scroll_max %d cursor_max %d\n", song_num, scroll_max, cursor_max);

        if (rapid & PAD_BUTTON_UP)
        {
            if (gp->playlist.cursor == 0)
            {
                if (gp->playlist.scroll > 0) // try to scroll up
                {
                    gp->playlist.scroll--;
                    is_update = UPDATEKIND_CHANGE;
                }
                else // loop?
                {
                    gp->focus = MENUFOCUS_PLAYLISTMODE;
                    is_update = UPDATEKIND_CHANGE;
                }
            }
            else
            {
                gp->playlist.cursor--;
                is_update = UPDATEKIND_CHANGE;
            }
        }
        else if (rapid & PAD_BUTTON_DOWN)
        {
            if (gp->playlist.cursor == cursor_max)
            {
                if (gp->playlist.scroll < scroll_max) // try to scroll down
                {
                    gp->playlist.scroll++;
                    is_update = UPDATEKIND_CHANGE;
                }
            }
            else
            {
                gp->playlist.cursor++;
                is_update = UPDATEKIND_CHANGE;
            }
        }
        else if (rapid & PAD_BUTTON_RIGHT)
        {

            // change song
            if (!is_selected_add_btn &&
                SongData_ChangeSong(playlist_kind, playlist_song_entry, 1))
                is_update = UPDATEKIND_CHANGE;
        }
        else if (rapid & PAD_BUTTON_LEFT)
        {
            if (!is_selected_add_btn &&
                SongData_ChangeSong(playlist_kind, playlist_song_entry, -1))
                is_update = UPDATEKIND_CHANGE;
        }
        else if (down & PAD_BUTTON_A)
        {
            // add song to playlist
            if (is_selected_add_btn &&
                SongData_ChangeSong(playlist_kind, playlist_song_entry, 0))
            {
                stc_playlist_data[playlist_kind].song_num++;
                is_update = UPDATEKIND_ADVANCE;
            }
            else
            {
                // play song
                SongData_PlaySong(stc_playlist_data[playlist_kind].songs[playlist_song_entry]);
                SFX_Play(FGMMENU_CS_KETTEI);
            }
        }
        else if (down & PAD_BUTTON_X)
        {
            if (!is_selected_add_btn && song_num > 1)
            {
                // shift up
                for (int i = playlist_song_entry; i < song_num - 1; i++)
                    stc_playlist_data[playlist_kind].songs[i] = stc_playlist_data[playlist_kind].songs[i + 1];

                stc_playlist_data[playlist_kind].songs[song_num - 1] = 0;
                stc_playlist_data[playlist_kind].song_num--;

                // scroll up if necessary
                int song_num = stc_playlist_data[playlist_kind].song_num;
                int scroll_max = ((song_num + 1) - PLAYLISTENTRY_NUM >= 0) ? ((song_num + 1) - PLAYLISTENTRY_NUM) : 0;
                // int cursor_max = (song_num < PLAYLISTENTRY_NUM) ? (song_num) : (PLAYLISTENTRY_NUM - 1);
                if (gp->playlist.scroll > scroll_max)
                    gp->playlist.scroll = scroll_max;

                is_update = UPDATEKIND_REGRESS;
            }
            else
                SFX_Play(FGMMENU_CS_BEEP1);
        }
        else if (down & PAD_BUTTON_B)
        {
            is_update = UPDATEKIND_REGRESS;
            gp->focus = MENUFOCUS_GAMEMODE;
            gp->playlist.cursor = 0;
            gp->playlist.scroll = 0;
        }
    }
    }

    // start to exit always
    if (is_update == UPDATEKIND_NONE && down & PAD_BUTTON_START)
    {
        is_update = UPDATEKIND_ADVANCE;
        Scene_ExitMinor();
    }

    // act on input
    switch (is_update)
    {
    case (UPDATEKIND_ADVANCE):
    {
        SFX_Play(FGMMENU_CS_KETTEI);
        MusicSettings_UIUpdate(g);
        break;
    }
    case (UPDATEKIND_REGRESS):
    {
        SFX_Play(FGMMENU_CS_CANCEL);
        MusicSettings_UIUpdate(g);

        break;
    }
    case (UPDATEKIND_CHANGE):
    {
        SFX_Play(FGMMENU_CS_MV);
        MusicSettings_UIUpdate(g);
        break;
    }
    }

    return;
}
void MusicSettings_UIUpdate(GOBJ *g)
{
    MusicSettingsData *gp = g->userdata;
    JOBJ *j = g->hsd_object;

    MusicSettingsPlaylist playlist_kind = gp->game_mode.scroll + gp->game_mode.cursor;
    int song_num = stc_playlist_data[playlist_kind].song_num;
    int gamemode_scroll_max = (PLAYLIST_NUM - GAMEMODEBUTTON_NUM >= 0) ? (PLAYLIST_NUM - GAMEMODEBUTTON_NUM) : 0;
    int playlist_cursor_max = (song_num >= PLAYLISTENTRY_NUM) ? PLAYLISTENTRY_NUM - 1 : song_num;
    int playlist_scroll_max = (song_num - (PLAYLISTENTRY_NUM - 1) >= 0) ? (song_num - (PLAYLISTENTRY_NUM - 1)) : 0;

    // update game mode buttons
    for (int i = 0; i < GetElementsIn(gp->game_mode.button); i++)
    {
        int anim_id;
        if (gp->game_mode.cursor == i) // if hovered over the option, use anim 1 when in control of the cursor, else its already selected
            anim_id = (gp->focus == MENUFOCUS_GAMEMODE) ? 1 : 2;
        else
            anim_id = 0;
        JObj_AddSetAnim(gp->game_mode.button[i].background_j, anim_id, gp->game_mode.button_background_set[0], 0, 1); // add anims

        JObj_SetFrameAndRate(gp->game_mode.button[i].name_j, gp->game_mode.scroll + i, 0);
    }

    // update game mode arrows
    if (gp->game_mode.scroll == gamemode_scroll_max)
        JObj_SetFlagsAll(gp->game_mode.arrowdown_j, JOBJ_HIDDEN);
    else
        JObj_ClearFlagsAll(gp->game_mode.arrowdown_j, JOBJ_HIDDEN);
    if (gp->game_mode.scroll == 0)
        JObj_SetFlagsAll(gp->game_mode.arrowup_j, JOBJ_HIDDEN);
    else
        JObj_ClearFlagsAll(gp->game_mode.arrowup_j, JOBJ_HIDDEN);

    // update playlist mode
    if (gp->focus == MENUFOCUS_PLAYLISTMODE)
    {
        JObj_SetFrameAndRate(gp->playlist.mode_button.fill_j, 1, 0);
        JObj_ClearFlagsAll(gp->playlist.mode_button.cursor_j, JOBJ_HIDDEN);
    }
    else
    {
        JObj_SetFrameAndRate(gp->playlist.mode_button.fill_j, 0, 0);
        JObj_SetFlagsAll(gp->playlist.mode_button.cursor_j, JOBJ_HIDDEN);
    }
    JObj_SetFrameAndRate(gp->playlist.mode_button.name_j, stc_playlist_data[gp->game_mode.cursor + gp->game_mode.scroll].mode, 0);

    // update playlist entry buttons
    MusicSettingsPlaylistMode playlist_mode = stc_playlist_data[gp->game_mode.scroll + gp->game_mode.cursor].mode;
    for (int i = 0; i < GetElementsIn(gp->playlist.entry_button); i++)
    {
        int playlist_song_num = stc_playlist_data[gp->game_mode.scroll + gp->game_mode.cursor].song_num;
        int song_database_idx = stc_playlist_data[gp->game_mode.scroll + gp->game_mode.cursor].songs[gp->playlist.scroll + i];

        if (playlist_mode != PLAYLISTMODE_PLAYLIST || gp->playlist.scroll + i >= playlist_song_num)
            JObj_SetFlagsAll(gp->playlist.entry_button[i].j, JOBJ_HIDDEN);
        else
        {
            JObj_ClearFlagsAll(gp->playlist.entry_button[i].j, JOBJ_HIDDEN);

            int frame;
            if (gp->focus == MENUFOCUS_PLAYLISTENTRIES)
                frame = (gp->playlist.cursor == i) ? 1 : 0;
            else
                frame = 0;

            JObj_SetFrameAndRate(gp->playlist.entry_button[i].fill_j, frame, 0);

            // get song name
            int song_database_idx = stc_playlist_data[gp->game_mode.scroll + gp->game_mode.cursor].songs[gp->playlist.scroll + i];
            char sanitized_text[256];

            // sanitize text
            Text_Sanitize(stc_song_data[song_database_idx].name, sanitized_text, sizeof(sanitized_text));

            // remove .hps
            char *extension_ptr = strstr(sanitized_text, ".hps");
            if (extension_ptr)
                extension_ptr[0] = '\0';

            char *song_name = sanitized_text;

            // update text
            Text *text = gp->playlist.entry_button[i].songname_text;
            Text_SetText(text, 0, song_name); // song name
            text->trans.X = 0;                // reset text scroll

            // store width of this text for scrolling
            if (gp->focus == MENUFOCUS_PLAYLISTENTRIES && i == gp->playlist.cursor)
            {
                // get text width
                float scroll_amt = SongScroll_GetAmount(text);

                // check if text should scroll
                if (scroll_amt > 0)
                {
                    gp->playlist.text_scroll.state = SONGSCROLL_STARTWAIT;
                    gp->playlist.text_scroll.timer = 30;
                }
                else
                {
                    gp->playlist.text_scroll.state = SONGSCROLL_NONE;
                    gp->playlist.text_scroll.timer = 0;
                }
            }

            // set song index
            if (gp->playlist.entry_button[i].songnum_text)
                Text_SetText(gp->playlist.entry_button[i].songnum_text, 0, "%d", gp->playlist.scroll + i + 1);
        }
    }

    // update playlist add button
    if (playlist_mode == PLAYLISTMODE_PLAYLIST &&   // playlist mode is selected
        gp->playlist.scroll == playlist_scroll_max) // scrolled as far as it can go
    {
        // move it
        gp->playlist.add_button.j->trans = gp->playlist.entry_button[playlist_cursor_max].j->trans;
        JObj_ClearFlagsAll(gp->playlist.add_button.j, JOBJ_HIDDEN);

        // update highlight
        int frame = (gp->playlist.cursor == (song_num - playlist_scroll_max)) ? 1 : 0;
        JObj_SetFrameAndRate(gp->playlist.add_button.j, frame, 0);
    }
    else
        JObj_SetFlagsAll(gp->playlist.add_button.j, JOBJ_HIDDEN);

    // update playlist entry arrows
    if (playlist_mode == PLAYLISTMODE_PLAYLIST && gp->playlist.scroll < playlist_scroll_max)
        JObj_ClearFlagsAll(gp->playlist.arrowdown_j, JOBJ_HIDDEN);
    else
        JObj_SetFlagsAll(gp->playlist.arrowdown_j, JOBJ_HIDDEN);
    if (playlist_mode == PLAYLISTMODE_PLAYLIST && gp->playlist.scroll > 0)
        JObj_ClearFlagsAll(gp->playlist.arrowup_j, JOBJ_HIDDEN);
    else
        JObj_SetFlagsAll(gp->playlist.arrowup_j, JOBJ_HIDDEN);

    // update playlist entry cursor
    if (gp->focus == MENUFOCUS_PLAYLISTENTRIES &&                                                    // focused on playlist entries
        !(gp->playlist.cursor == playlist_cursor_max && gp->playlist.scroll == playlist_scroll_max)) // not hovered over add button
    {
        JObj_ClearFlagsAll(gp->playlist.cursor_j, JOBJ_HIDDEN);                                                              // show cursor
        JObj_GetChildPosition(gp->playlist.entry_button[gp->playlist.cursor].cursorpos_j, 0, &gp->playlist.cursor_j->trans); // place it over the current selection
    }
    else
        JObj_SetFlagsAll(gp->playlist.cursor_j, JOBJ_HIDDEN);

    // update playlist entry delete button
    if (gp->focus == MENUFOCUS_PLAYLISTENTRIES &&                                                      // focused on playlist entries
        !(gp->playlist.cursor == playlist_cursor_max && gp->playlist.scroll == playlist_scroll_max) && // not hovered over add button
        song_num > 1)                                                                                  // over 1 song present on the list
    {
        JObj_ClearFlagsAll(gp->playlist.delete_button.j, JOBJ_HIDDEN);                                                              // show button
        JObj_GetChildPosition(gp->playlist.entry_button[gp->playlist.cursor].deletepos_j, 0, &gp->playlist.delete_button.j->trans); // place it over the current selection
    }
    else
        JObj_SetFlagsAll(gp->playlist.delete_button.j, JOBJ_HIDDEN);

    // update description
    switch (gp->focus)
    {
    case (MENUFOCUS_GAMEMODE):
    {
        MusicSettings_SetDescription("Select a mode to adjust its music.");
        break;
    }
    case (MENUFOCUS_PLAYLISTMODE):
    {
        switch (stc_playlist_data[gp->game_mode.cursor + gp->game_mode.scroll].mode)
        {
        case (PLAYLISTMODE_ORIGINAL):
        {
            MusicSettings_SetDescription("This mode's default music will play.");
            break;
        }
        case (PLAYLISTMODE_PLAYLIST):
        {
            MusicSettings_SetDescription("Create a list of songs to play in this mode.");
            break;
        }
        case (PLAYLISTMODE_RANDOM):
        {
            MusicSettings_SetDescription("All songs will play in this mode.");
            break;
        }
        }

        break;
    }
    case (MENUFOCUS_PLAYLISTENTRIES):
    {
        MusicSettings_SetDescription("Adjust the song using left and right.");
        break;
    }
    }

    JObj_AnimAll(j);
    JObj_SetMtxDirtySub(j);

    return;
}

float SongScroll_GetAmount(Text *t)
{
    Vec3 width_start, width_end;
    Vec3 bound_start, bound_end;
    Vec3 scale, trans;

    JOBJ *j = TextJoint_FindJointFromText(t);

    // get text width
    float width = TextJoint_GetWidth(t);

    JObj_SetupMtxSub(j);
    HSD_MtxGetTranslate(&j->rotMtx, &trans);
    HSD_MtxGetScale(&j->rotMtx, &scale);

    float max_visible = (SONG_VISIBLE_WIDTH * scale.X);

    if (width > max_visible)
        return width - max_visible;
    else
        return 0;
}

int SongData_ChangeSong(MusicSettingsPlaylist playlist, int entry, int dir)
{
    int additive = dir;
    int cur_value = stc_playlist_data[playlist].songs[entry];

    // if 0 direction, check first song, then increment
    if (dir == 0)
        additive = 1;
    else
        cur_value += additive;

    // OSReport("Changing song on playlist %d entry %d\n", playlist, entry);

    for (int song_database_idx = 0; song_database_idx < stc_song_num; song_database_idx++)
    {
        // limit
        if (cur_value < 0)
            cur_value = (stc_song_num - 1);
        else if (cur_value > (stc_song_num - 1))
            cur_value = 0;

        // check if song is already in playlist
        int is_in_playlist = 0;
        for (int i = 0; i < SONGS_PER_PLAYLIST; i++)
        {

            if (cur_value == stc_playlist_data[playlist].songs[i])
            {
                is_in_playlist = 1;
                break;
            }
        }

        if (!is_in_playlist)
        {
            stc_playlist_data[playlist].songs[entry] = cur_value;
            return 1;
        }

        // get next song
        cur_value += additive;
    }

    return 0;
}
