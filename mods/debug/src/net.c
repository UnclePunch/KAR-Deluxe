
#include "text.h"
#include "useful.h"
#include "hsd.h"
#include "preload.h"
#include "scene.h"
#include "inline.h"
#include "audio.h"
#include "game.h"
#include "useful.h"

#include "debug.h"
#include "net.h"
#include "fst/fst.h"
#include "code_patch/code_patch.h"

static int net_log_num = 0;
static NetLog *net_log;

void LogBufferHex(const void *data, int len)
{
    const u8 *bytes = (const u8 *)data;

    OSReport("Buffer (hex, len=%d):\n", len);
    for (int i = 0; i < len; ++i)
    {
        OSReport("%02X ", bytes[i]);

        if ((i + 1) % 16 == 0)
            OSReport("\n"); // wrap every 16 bytes
    }
    OSReport("\n");
}

void Net_Think(GOBJ *g)
{
    if (net_log_num == 0)
        return;

    OSReport("## LOG ##\n\n");
    OSReport("%d logged\n", net_log_num);

    for (int i = 0; i < net_log_num; i++)
    {
        NetLog *nl = &net_log[i];
        static char *log_source_names[] = {"SEND", "RECEIVE"};

        OSReport("%s of size %d\n", log_source_names[nl->source], nl->len);

        LogBufferHex(nl->buf, nl->len);
    }

    net_log_num = 0;
    OSReport("\n## END ##\n\n");

    return;
}

void Hook_SOSendTo(int s, const void *buf, int len, int flags, const void *sockTo)
{
    int size = (len > NET_BUF_SIZE) ? NET_BUF_SIZE : len;

    if (net_log_num >= NET_ARR_SIZE)
        return;

    NetLog *nl = &net_log[net_log_num];

    nl->source = LOGSOURCE_SEND;
    nl->len = len;

    for (int i = 0; i < size; i++)
        nl->buf[i] = ((u8 *)buf)[i];

    net_log_num++;

    return;
}
CODEPATCH_HOOKCREATE(0x8047d08c, "", Hook_SOSendTo, "", 0)

void Hook_SORecvFrom(int s, void *buf, int len, int flags, void *sockFrom)
{
    int size = (len > NET_BUF_SIZE) ? NET_BUF_SIZE : len;

    if (net_log_num >= NET_ARR_SIZE)
        return;

    NetLog *nl = &net_log[net_log_num];

    nl->source = LOGSOURCE_RECEIVE;
    nl->len = len;

    for (int i = 0; i < size; i++)
        nl->buf[i] = ((u8 *)buf)[i];

    net_log_num++;

    return;
}
CODEPATCH_HOOKCREATE(0x8047ce2c, "", Hook_SORecvFrom, "", 0)

void Net_Init()
{
    net_log = HSD_MemAlloc(sizeof(*net_log));

    CODEPATCH_HOOKAPPLY(0x8047d08c);
    CODEPATCH_HOOKAPPLY(0x8047ce2c);
}
