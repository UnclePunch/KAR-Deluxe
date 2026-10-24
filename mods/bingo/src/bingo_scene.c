#include "obj.h"
#include "scene.h"
#include "game.h"
#include "hsd.h"
#include "inline.h"

#include "bingo_scene.h"

#include "hoshi/func.h"

#include "code_patch/code_patch.h"

extern int g_is_bingo_mode;
int g_is_clear_efb;

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

static COBJDesc cobj_desc = {
    .class_name = 0,
    .flags = 0,
    .projection_type = PROJ_ORTHO,
    .viewport_left = 0,
    .viewport_right = 640,
    .viewport_top = 0,
    .viewport_bottom = 480,
    .scissor_lr = 640,
    .scissor_tb = 480,
    .eye_desc = &(WOBJDesc){.pos = {0, 0, 1}},
    .interest_desc = &(WOBJDesc){.pos = {0, 0, 0}},
    .roll = 0,
    .vector = 0,
    .near = 0.1f,
    .far = 10.0f,
    .projection_param.ortho.left = 0,
    .projection_param.ortho.right = 640,
    .projection_param.ortho.top = 0,
    .projection_param.ortho.bottom = 480,
};

HSD_Archive *g_bingo_archive;

// _HSD_ImageDesc g_image_desc = {
//     .width = 640,
//     .height = 480,
//     .format = GX_TF_RGB565,
//     .mipmap = GX_DISABLE,
//     .maxLOD = 0,
//     .img_ptr = 0,
// };
_HSD_ImageDesc *g_image_desc_ptr;

u16 *g_fullscreen_image_ptr, *g_downscale_image_ptr;
int g_downscale_num;
void BingoResults_EFBDownscale_CObjThink(GOBJ *g)
{
    if (g_downscale_num >= DOWNSCALE_NUM)
        return;

    COBJ *c = g->hsd_object;

    // set resolution
    c->viewport_right = DOWNSCALE_WIDTH;
    c->scissor_right = DOWNSCALE_WIDTH;
    c->viewport_bottom = DOWNSCALE_HEIGHT;
    c->scissor_bottom = DOWNSCALE_HEIGHT;

    if (!CObj_SetCurrent(g->hsd_object))
        return;

    OSReport("downscale #%d\n", g_downscale_num);

    // set image to the fullscreen EFB
    g_image_desc_ptr->img_ptr = g_fullscreen_image_ptr;
    g_image_desc_ptr->width = 640;
    g_image_desc_ptr->height = 480;

    // render it at the reduced resolution
    CObj_RenderGXLinks(g, (1 << 0) | (1 << 1) | (1 << 2));

    // now copy the downscaled image from EFB 
    g_image_desc_ptr->img_ptr = g_downscale_image_ptr;
    g_image_desc_ptr->width = DOWNSCALE_WIDTH;
    g_image_desc_ptr->height = DOWNSCALE_HEIGHT;
    HSD_ImageDescCopyFromEFB(g_image_desc_ptr, 0, 0, GX_TRUE, GX_TRUE);
}
void BingoResults_EFBUpscale_CObjThink(GOBJ *g)
{

}

int g_is_efb_dumped;
JOBJSet *g_efb_jobjset;
int g_efb_anim_idx;
void BingoResults_BackgroundThink(GOBJ *g)
{
    if (g_is_efb_dumped == 1)
        g_is_efb_dumped = 2;

    JObj_AnimAll(g->hsd_object);

    // // advance anim
    // if (!JObj_CheckAObjPlaying(g->hsd_object))
    //     JObj_AddSetAnim(g->hsd_object, ++g_efb_anim_idx, g_efb_jobjset, 0, 1);
}
void BingoResults_BackgroundGX(GOBJ *g, int pass)
{
    if (pass != 2 || !(g_is_efb_dumped >= 2))
        return;

    // set img_data to render the downscaled image
    g_image_desc_ptr->img_ptr = g_downscale_image_ptr;
    g_image_desc_ptr->width = DOWNSCALE_WIDTH;
    g_image_desc_ptr->height = DOWNSCALE_HEIGHT;

    // render the downscaled image
    JObj_GX(g, pass);

    if (g_downscale_num < DOWNSCALE_NUM)
    {
        // dump the downscaled image
        g_image_desc_ptr->img_ptr = g_fullscreen_image_ptr;
        g_image_desc_ptr->width = 640;
        g_image_desc_ptr->height = 480;
        HSD_ImageDescCopyFromEFB(g_image_desc_ptr, 0, 0, GX_FALSE, GX_TRUE);
        
        g_downscale_num++;
    }

    return;
}

void BingoResults_Enter()
{
    int gx_link, image_size;

    g_bingo_archive = Archive_LoadFile("MnBingoResults");
    if (!g_bingo_archive)
        return;

    HSD_SObjDesc *sobj_desc = Archive_GetPublicAddress(g_bingo_archive, "ScResultsBingo_scene_data");
    if (!sobj_desc)
        return;

    // downscale process
    gx_link = 4;
    GOBJ_EZCreator(0, 0, 0,
                   0, 0,
                   HSD_OBJKIND_COBJ, sobj_desc->cobjdesc[1],
                   0, 0,
                   BingoResults_EFBDownscale_CObjThink, (1 << gx_link), 0);
    GOBJ_EZCreator(0, 0, 0,
                    0, 0,
                    HSD_OBJKIND_JOBJ, sobj_desc->jobjset[1]->jobj,
                    0, 0,
                    JObj_GX, gx_link, 0);

    // display image
    gx_link = 5;
    GOBJ_EZCreator(0, 0, 0,
                   0, 0,
                   HSD_OBJKIND_COBJ, sobj_desc->cobjdesc[1],
                   0, 0,
                   CObjThink_Common, (1 << gx_link), 1);
    GOBJ_EZCreator(0, 0, 0,
                   0, 0,
                   HSD_OBJKIND_JOBJ, sobj_desc->jobjset[1]->jobj,
                   BingoResults_BackgroundThink, 0,
                   BingoResults_BackgroundGX, gx_link, 0);
    // g_efb_jobjset = sobj_desc->jobjset[0];
    // g_efb_anim_idx = 0;
    // JObj_AddSetAnim(bg_gobj->hsd_object, g_efb_anim_idx, g_efb_jobjset, 0, 1);

    // get image_desc
    g_image_desc_ptr = Archive_GetPublicAddress(g_bingo_archive, "ScResultsBingoEFB_image_desc");
    g_image_desc_ptr->format = GX_TF_RGBA8;

    // alloc downscaled image
    image_size = GXGetTexBufferSize(DOWNSCALE_WIDTH,
                                    DOWNSCALE_HEIGHT,
                                    g_image_desc_ptr->format,
                                    g_image_desc_ptr->mipmap,
                                    g_image_desc_ptr->maxLOD);
    g_downscale_image_ptr = HSD_MemAlloc(image_size);

    // alloc fullscreen image buffer
    image_size = GXGetTexBufferSize(640,
                                    480,
                                    g_image_desc_ptr->format,
                                    g_image_desc_ptr->mipmap,
                                    g_image_desc_ptr->maxLOD);
    g_fullscreen_image_ptr = HSD_MemAlloc(image_size);

    g_is_efb_dumped = 0;
    g_downscale_num = 0;
    if (g_is_efb_dumped == 0)
    {
        g_image_desc_ptr->width = 640;
        g_image_desc_ptr->height = 480;
        g_image_desc_ptr->img_ptr = g_fullscreen_image_ptr;
        HSD_ImageDescCopyFromEFB(g_image_desc_ptr, 0, 0, GX_TRUE, GX_TRUE);
        g_is_efb_dumped = 1;
    }
}
void BingoResults_Exit(void *data)
{

}
void BingoResults_Think()
{
    int down = Pad_GetDown(20);

    if (down & PAD_BUTTON_B)
    {
        SFX_Play(FGMMENU_CS_CANCEL);
        Scene_SetDirection(PAD_BUTTON_B);
        Scene_ExitMinor();
    }
}

MinorSceneDesc minor_scene = {
    .idx = -1,
    .x1 = -1,
    .cb_Load = BingoResults_Enter,
    .cb_Exit = BingoResults_Exit,
    .cb_ThinkPreGObjProc = BingoResults_Think,
    .preload_kind = 4,
};

void BingoMajor_OnEnter()
{
    g_is_bingo_mode = 1;

    // must spoof as city trial so the 3d minor load function inits the match correctly
    Gm_GetGameData()->major_cur = MJRKIND_CITY;

    void (*CityTrial_MajorEnter)() = (void *)0x8003fc5c;
    CityTrial_MajorEnter();
}
void BingoMajor_OnExitMinor()
{
    GameData *gd = Gm_GetGameData();
    CitySceneKind city_scene = gd->city.scene;
    int dir = Scene_GetDirection();

    switch (city_scene)
    {
    case (CITYSCENE_PLYSELECT):
        if (dir & PAD_BUTTON_START)
        {
            gd->city.scene = CITYSCENE_CITY;
            Scene_SetNextMinor(MNRKIND_3D);
        }
        else if (dir & PAD_BUTTON_B)
        {
            gd->main_menu.top_menu = MAINMENU_TOPMENU_OPTIONS;
            gd->main_menu.is_in_submenu = 1;
            gd->main_menu.cursor_val[0] = 4;
            gd->main_menu.cursor_val[1] = 0;

            g_is_bingo_mode = 0;
            Scene_SetNextMajor(MJRKIND_MENU);
            Scene_ExitMajor();
        }
        else if (dir & PAD_TRIGGER_Z)
        {
            gd->city.scene = CITYSCENE_SETTINGS;
            Scene_SetNextMinor(MNRKIND_CITYSETTINGS);
        }
        break;

    case (CITYSCENE_SETTINGS):
        if (dir & (PAD_BUTTON_A | PAD_BUTTON_B))
        {
            CitySelect_DisableLoadingAnim();
            gd->city.scene = CITYSCENE_PLYSELECT;
            Scene_SetNextMinor(MNRKIND_CITYPLYSELECT);
        }
        break;

    case (CITYSCENE_CITY):
        if (dir & PAD_BUTTON_A) // match completes?
        {
            gd->city.scene = CITYSCENE_RESULTS;
            Scene_SetNextMinor(minor_scene.idx);
        }
        else if (dir & PAD_BUTTON_B) // exit?
        {
            gd->city.scene = CITYSCENE_PLYSELECT;
            Scene_SetNextMinor(MNRKIND_CITYPLYSELECT);
        }
        else if (dir & PAD_TRIGGER_Z) // start over
        {
            gd->city.scene = CITYSCENE_CITY;
            Scene_SetNextMinor(MNRKIND_3D);
        }
        break;

    case (CITYSCENE_RESULTS):
        gd->city.scene = CITYSCENE_PLYSELECT;
        Scene_SetNextMinor(MNRKIND_CITYPLYSELECT);
        break;
    }

    gd->city.scene_prev = city_scene;

}
MajorSceneDesc major_desc = {
    .initial_minor_id = MNRKIND_CITYPLYSELECT,
    .cb_Enter = BingoMajor_OnEnter,
    .cb_ExitMinor = BingoMajor_OnExitMinor,
    .next_major_id = MJRKIND_MENU,
};

MajorKind Bingo_InstallScene()
{
    Hoshi_InstallMinorScene(&minor_scene);
    Hoshi_InstallMajorScene(&major_desc);

    return major_desc.major_id;
}

void GXCopyDispHijack(void *dest, GXBool enable)
{
    GXCopyDisp(dest, g_is_clear_efb);
    g_is_clear_efb = 1;
}
void SkipGXCopyDispOnSceneChange()
{
    g_is_clear_efb = 0;
}
CODEPATCH_HOOKCREATE(0x80006508, "", SkipGXCopyDispOnSceneChange, "", 0)

void BingoScene_SetBoot()
{
    // boot to bingo
    GameData *gd = Gm_GetGameData();
    Hoshi_SetBootScene(major_desc.major_id, 1);

    // init a bunch of city trial specific stuff
    gd->main_menu.is_in_submenu = 1;
    gd->main_menu.top_menu = MAINMENU_TOPMENU_CITY;
    gd->main_menu.submenu_kind = MAINMENU_TOPMENU_CITY;
    gd->main_menu.major_kind = MJRKIND_CITY;
    gd->city.mode = CITYMODE_TRIAL;
    gd->is_showed_tutorial_city = 1;
}

void BingoScene_OnReset()
{
    g_is_bingo_mode = 0;
    g_is_clear_efb = 1;
}

void BingoScene_Init()
{
    CODEPATCH_HOOKAPPLY(0x80006508);
    CODEPATCH_REPLACECALL(0x80411160, GXCopyDispHijack);
}