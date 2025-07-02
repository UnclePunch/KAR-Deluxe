#ifndef NET_H
#define NET_H

#define NET_ARR_SIZE 6
#define NET_BUF_SIZE 512

typedef enum LogSource
{
    LOGSOURCE_SEND,
    LOGSOURCE_RECEIVE,
} LogSource;

typedef struct NetLog
{
    LogSource source;
    int len;
    u8 buf[NET_BUF_SIZE];
} NetLog;

void Net_Init();
void Net_Think(GOBJ *g);

#endif