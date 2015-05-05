#ifndef PROTOCOL_H
#define PROTOCOL_H

#endif // PROTOCOL_H

#define HEARTBEAT_TYPE 0
#define NEW_CONNECTION_TYPE 1
#define APP_DATA_TYPE 2

#define MAXPAYLOAD 10000

typedef struct proxyheader{
    unsigned char type;
    unsigned char beatHeart;
}proxyHeader_t;

typedef struct proxypacket{
    proxyHeader_t header;
    char payload[MAXPAYLOAD];

}proxyPacket_t;


