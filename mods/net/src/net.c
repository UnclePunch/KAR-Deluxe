#include "inline.h"
#include "text.h"
#include "ip/IP.h"
#include "ip/IPSocket.h"

#include "net.h"

static int heap_handle;
void *Net_Alloc(u32 name, s32 size)
{
    return OSAllocFromHeap(heap_handle, size);
}
void *Net_Free(u32 name, void *alloc, s32 size)
{
    OSFreeToHeap(heap_handle, alloc);
}

void Net_PrintDebug()
{    
    void (*IPGetLinkState)(void* interface, int* up ) = (void *)0x8046d934;
    int (*IPGetConfigError)(void* interface) = (void *)0x8046f174;

    int link_up;
    IPGetLinkState(NULL, &link_up);
    OSReport("Link: %d\n", link_up);
    OSReport("HostID: %08X\n", SOGetHostID());
    OSReport("ConfigError: %d\n", IPGetConfigError(NULL));
}

static int g_ip = 0;
static Text *g_ip_text = 0;
static OSThread network_thread;
void *server_loop(void *arg)
{
    while (1)
    {
        Net_PrintDebug();
        
        void (*GetIPAddr)(int *ip_out) = (void *)0x804828c4;

        GetIPAddr(&g_ip);

        if (g_ip)
        {
            bp();
            int sock = SOSocket(SO_PF_INET, SO_SOCK_STREAM, 0);
            OSReport("Created socket %d\n", sock);

            if (sock < 0)
                goto CLEANUP;

            // SOSockAddrIn local = {};
            // local.len = sizeof(local);
            // local.family = SO_PF_INET;
            // local.port = 6786;
            // local.addr.addr = SO_INADDR_ANY;
            // OSReport("Bind: %d", SOBind(sock, &local));

            SOSockAddrIn addr = {};
            addr.len = sizeof(SOSockAddrIn);
            addr.family = SO_PF_INET;
            addr.port = 80;
            addr.addr.addr = 0x5DB8D822;   // example.com
            OSReport("Connect: %d\n", SOConnect(sock, &addr));

            CLEANUP:
            SOClose(sock);
        }

        OSReport("\n");

        // sleep thread
        OSSleepMilliseconds(500);
    }

    OSCancelThread(&network_thread);
    return 0;
}

void Net_Init()
{
    u32 heap_size = 64 * 1024;
    void *heap_lo = HSD_MemAlloc(heap_size);
    heap_handle = OSCreateHeap(heap_lo, (void *)((u32)heap_lo + heap_size));

}
void Net_OnSaveLoaded()
{
static SOConfig Config =
{
    SO_VENDOR_NINTENDO,     // vendor
    SO_VERSION,             // version

    (void *)Net_Alloc,               // alloc
    (void *)Net_Free,                // free

    SO_FLAG_DHCP,           // flag
    SOHtoNl(SO_INADDR_ANY), // addr
    SOHtoNl(SO_INADDR_ANY), // netmask
    SOHtoNl(SO_INADDR_ANY), // router
    SOHtoNl(SO_INADDR_ANY), // dns1
    SOHtoNl(SO_INADDR_ANY), // dns1

    4096,                   // timeWaitBuffer
    4096                    // reassemblyBuffer
};

    bp();
    Net_PrintDebug();

    SOStartup(&Config);
    
    // create thread
    static u8 stack[0x1000] ATTRIBUTE_ALIGN(32);
    OSCreateThread(&network_thread, server_loop, NULL, stack + sizeof(stack),
                   sizeof(stack), 0, 0);
    OSResumeThread(&network_thread);
}
void Net_OnSceneChange()
{
    #include "hoshi/screen_cam.h"
    g_ip_text = Hoshi_CreateScreenText();
    g_ip_text->kerning = 1;
    g_ip_text->use_aspect = 1;
    g_ip_text->trans = (Vec3){10, 30, 0};
    g_ip_text->viewport_scale = (Vec2){0.3, 0.3};
    g_ip_text->aspect = (Vec2){300, 0};
    g_ip_text->viewport_color = (GXColor){0, 0, 0, 128};
    Text_AddSubtext(g_ip_text, 0, 0, "");
}
void Net_OnFrameEnd()
{
    if (g_ip)
        Text_SetText(g_ip_text, 0, "ip: %d.%d.%d.%d", (g_ip & 0xFF000000) >> 24, (g_ip & 0x00FF0000) >> 16, (g_ip & 0x0000FF00) >> 8, g_ip & 0xFF);
}

