#ifndef PROTOCOL_H
#define PROTOCOL_H

#endif // PROTOCOL_H

#define HEARTBEAT_TYPE 0
#define NEW_CONNECTION_INI 1
#define APP_DATA 2

#define MAXPAYLOAD 10000


typedef struct heartbeat{
    unsigned char type;
    unsigned payload;
}heartBeat_t;

typedef struct appdata{
    unsigned char type;
    char payload[MAXPAYLOAD];
}appData_t;

typedef struct proxypacket{
    unsigned char type;
}proxyPacket_t;


