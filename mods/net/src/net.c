#include "inline.h"
#include "text.h"
#include "ip/IP.h"
#include "ip/IPSocket.h"

#include "net.h"

#include "hoshi/screen_cam.h"

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

static char *g_buffer = 0;
static int g_buffer_size = 0;

static s32 g_sock_discover = -1;
static s32 g_sock_data = -1;
static SOSockAddrIn g_client;
static NetState g_net_state = NETSTATE_NONE;
static NetPacket g_packet;
static int g_debug_client = SO_IP(10, 0, 0, 62);

s32 NetSendPos(int ply, Vec3 *pos)
{
    int enabled = OSDisableInterrupts();

    NetPacket *p = (NetPacket *)&g_buffer[g_buffer_size];

    if (g_buffer_size + sizeof(p->pos) > BUFFER_SIZE)
        return -1;

    p->kind = NETDATA_POS;
    p->pos.ply = ply;
    p->pos.pos = *pos;
    g_buffer_size += sizeof(p->pos);

    OSRestoreInterrupts(enabled);

    return 1;
}

s32 NetSend(s32 sock, void *p, u32 size)
{
    s32 sent = SOSendTo(sock, p, size, 0, (SOSockAddr*)&g_client);
    OSReport("%x SOSendTo: %d\n", g_client.addr.addr, sent);   // expect 8

    return sent;
}
s32 NetRecv(s32 sock, void *p, u32 size)
{
    SOSockAddrIn from;
    s32 rcvd = SORecvFrom(sock, p, size, 0, (SOSockAddr*)&from);
    OSReport("SORecvFrom: %d\n", rcvd);

    return rcvd == size;
}

s32 NetDiscover()
{
    SOSockAddrIn from;
    int packet_size = sizeof(g_packet.discover);

    s32 rcvd = SORecvFrom(g_sock_discover, &g_packet, packet_size, 0, (SOSockAddr*)&from);
    OSReport("SORecvFrom: %d\n", rcvd);

    if (rcvd > 0)
    {
        OSReport("discovered %x!\n", from.addr.addr);

        // save client info
        memset(&g_client, 0, sizeof(g_client));
        g_client.len        = sizeof(g_client);
        g_client.family     = SO_PF_INET;
        g_client.port       = from.port;
        g_client.addr       = from.addr;

        // tell client we're connected and share the port we send data over
        g_packet.discover.port = PORT_DATA;
        NetSend(g_sock_discover, &g_packet, sizeof(g_packet.discover));
    }

    return rcvd > 0;
}

static int g_ip = 0;
static Text *g_ip_text = 0;
static OSThread network_thread;
void *server_loop(void *arg)
{
    int seq = 0;
    while (1)
    {
        void (*GetIPAddr)(int *ip_out) = (void *)0x804828c4;
        
        if (g_ip == 0)
            GetIPAddr(&g_ip);

        switch (g_net_state)
        {
            case (NETSTATE_NONE):
                // wait for SO to finish init'ing, then advance state
                if (SOGetHostID() != 0)
                {
                    if (g_debug_client == 0)
                        g_net_state = NETSTATE_DISCOVER;
                    else
                    {
                        // set client info
                        memset(&g_client, 0, sizeof(g_client));
                        g_client.len        = sizeof(g_client);
                        g_client.family     = SO_PF_INET;
                        g_client.port       = PORT_DATA;
                        g_client.addr.addr  = g_debug_client;
                        g_net_state = NETSTATE_CONNECTED;
                    }
                }
                break;

            case (NETSTATE_DISCOVER):
                if (NetDiscover())
                    g_net_state = NETSTATE_CONNECTED;
                    
                break;

            case (NETSTATE_CONNECTED):
                if (g_buffer_size > 0)
                {
                    NetSend(g_sock_data, g_buffer, g_buffer_size);
                    g_buffer_size = 0;
                }
                break;
        }

        // sleep thread
        OSSleepMilliseconds(500);
    }

    OSCancelThread(&network_thread);
    return 0;
}

void Net_HeapInit()
{
    u32 heap_size = 64 * 1024;
    void *heap_lo = HSD_MemAlloc(heap_size);
    heap_handle = OSCreateHeap(heap_lo, (void *)((u32)heap_lo + heap_size));

    // alloc packet buffer
    g_buffer = HSD_MemAlloc(BUFFER_SIZE);
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

    if (SOStartup(&Config) < 0)
        return;

    // prep bind
    SOSockAddrIn local;
    memset(&local, 0, sizeof(local));
    local.len         = sizeof(local);
    local.family      = SO_PF_INET;
    local.addr.addr   = SO_INADDR_ANY;

    // create discover socket
    g_sock_discover = SOSocket(SO_PF_INET, SO_SOCK_DGRAM, 0);
    if (g_sock_discover < 0) goto CLEANUP;
    local.port = PORT_DISCOVER;
    if (SOBind(g_sock_discover, (SOSockAddr*)&local) < 0) goto CLEANUP;

    // create data socket
    g_sock_data = SOSocket(SO_PF_INET, SO_SOCK_DGRAM, 0);
    if (g_sock_data < 0) goto CLEANUP;
    local.port = PORT_DATA;
    if (SOBind(g_sock_data, (SOSockAddr*)&local) < 0) goto CLEANUP;

    // set as nonblocking
    SOFcntl(g_sock_discover, SO_F_SETFL, SO_O_NONBLOCK);
    SOFcntl(g_sock_data, SO_F_SETFL, SO_O_NONBLOCK);

    // init state
    g_net_state = NETSTATE_NONE;

    OSReport("server listening on port %d\n", PORT_DISCOVER);

    // create thread
    static u8 stack[0x1000] ATTRIBUTE_ALIGN(32);
    OSCreateThread(&network_thread, server_loop, NULL, stack + sizeof(stack),
                   sizeof(stack), 0, 0);

    OSResumeThread(&network_thread);

    return;

    CLEANUP:
    if (g_sock_discover != -1) SOClose(g_sock_discover);
    if (g_sock_data != -1) SOClose(g_sock_data);
    OSReport("server stopped\n");

    return;
}
void Net_OnSceneChange()
{
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

    if (Scene_GetCurrentMinor() == MNRKIND_3D)
    {
        GOBJ *r = Ply_GetRiderGObj(0);
        if (r)
        {
            RiderData *rd = r->userdata;
            NetSendPos(rd->ply, &rd->pos);
        }
    }
}