#ifndef PROTOCOL_H
#define PROTOCOL_H

#endif // PROTOCOL_H

#define HEARTBEAT_TYPE 0
#define NEW_CONNECTION_TYPE 1
#define APP_DATA_TYPE 2

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
    char payload[MAXPAYLOAD];

}proxyPacket_t;


