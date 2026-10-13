#include "ip/IPSocket.h"

#define PORT_DISCOVER 5000
#define PORT_DATA 5001

#define SO_IP(a, b, c, d) \
    ((((u32)(a) & 0xFF) << 24) | \
     (((u32)(b) & 0xFF) << 16) | \
     (((u32)(c) & 0xFF) <<  8) | \
     (((u32)(d) & 0xFF)))

typedef enum NetState
{
    NETSTATE_NONE,
    NETSTATE_DISCOVER,
    NETSTATE_CONNECTED,
} NetState;

typedef struct NetPacket
{
    union
    {
        struct 
        {
            u16 port;
        } discover;
    };
    
} NetPacket;

void Net_HeapInit();
void Net_OnSaveLoaded();
void Net_OnSceneChange();
void Net_OnFrameEnd();