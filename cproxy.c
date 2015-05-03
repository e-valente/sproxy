/*cproxy.c is used on client machine
 * It should listen to port 5200
 *Date: Thu Apr 23 23:38:20 MST 2015
 *Contact: emanuelvalente@gmail.com
 *
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <unistd.h>
#include <time.h>

#include "protocol.h"

#define MAXPAYLOAD 10000
#define LISTEN_PORT 5200
#define SPROXY_PORT 6200
//#define SPROXY_PORT 23b

int startSProxyClient(char *serverIPAddr, int port);
void startCProxyServer();

int socketFromTelnetClient, socketFromSProxy;


char payload[MAXPAYLOAD];

void error(char *msg){
  perror(msg);
  exit(1);
}

/*It will receive incoming connections from the CProxy client*/
void startCProxyServer(char *sproxyIPAddr) {

  int sockfd, bytes_received, bytes_sent;
  socklen_t clientlen;
  struct sockaddr_in local_serv_addr, client_addr, serv_addr;
  fd_set readfds;
  struct timeval tv;
  int n, ret;
  appData_t dataPacket;
  proxyPacket_t proxyPacket;
  int heartBeatCount;

  /*create a stream socket (TCP)*/
  sockfd = socket (AF_INET, SOCK_STREAM, 0);
  if(sockfd < 0)
    error ("ERROR opening socket");

  memset (&serv_addr, 0, sizeof(serv_addr));
  local_serv_addr.sin_family = AF_INET;
  local_serv_addr.sin_addr.s_addr = INADDR_ANY;
  local_serv_addr.sin_port = htons(LISTEN_PORT);

  if(bind(sockfd, (struct sockaddr *)&local_serv_addr, sizeof(struct sockaddr_in)) < 0)
    error("ERROR on binding");

  /*Listen!*/
  listen(sockfd, 5);
  fprintf(stderr, "Listen on port %d\n", LISTEN_PORT);
  clientlen = sizeof(client_addr);

  /*accept first incoming connection on the pending queue,
  returns a new file descriptor*/
  socketFromTelnetClient = accept(sockfd, (struct sockaddr *) &client_addr, &clientlen);

  if(socketFromTelnetClient < 0)
    error("ERROR on accept");

  /*connects to the SPROXY server*/
  socketFromSProxy = socket (AF_INET, SOCK_STREAM, 0);
  if (socketFromSProxy < 0)
    error ("ERROR opening socket\n");

  memset(&serv_addr, 0, sizeof(serv_addr));

  serv_addr.sin_family = AF_INET;
  /*convert and copy server's ip addr into serv_addr*/
  if(inet_pton(AF_INET, sproxyIPAddr, &serv_addr.sin_addr) <= 0) {
      fprintf(stderr, "%s is a bad address!\n", sproxyIPAddr);
      error ("ERROR, copying server ip address");
    }
  serv_addr.sin_port = htons(SPROXY_PORT);
  /* connect to server */
  if (connect(socketFromSProxy,(struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
    error ("ERROR connecting");

  /*At this point we have two active sockets:
   * one is socketFromTelnetClient connected to the end user
   * and socketFromTelnetClient connected to the SProxy
   * */

  /*testing... */
  /*
  strcpy(payload, "ma oi!");
  bytes_sent = send(socketFromTelnetClient, payload, (sizeof(char) * strlen(payload) + 1), 0);
  if (bytes_sent < 0)
    error ("ERROR writing to socket");

  memset(payload, 0, sizeof(char)* MAXPAYLOAD);
  bytes_received = recv(socketFromTelnetClient, payload, sizeof(char) * MAXPAYLOAD, 0);

  fprintf(stderr, "TELNET:\n%s\n", payload);
  */
  /*end of test*/

  /*pooulating packet*/
  dataPacket.type = APP_DATA_TYPE;
  heartBeatCount = 0;


  while(1) {

  /*clear the set ahead of time*/
  FD_ZERO(&readfds);
  FD_SET(socketFromTelnetClient, &readfds);
  /*this socket is going to be "greater"*/
  FD_SET(socketFromSProxy, &readfds);


  /*setting our delay for the events*/
  tv.tv_sec = 5;
  tv.tv_usec = 500000;

  /*param for select()*/
  n = socketFromSProxy + 1;

  ret = select(n, &readfds, NULL, NULL, &tv);

  if(ret == -1)
      fprintf(stderr, "Error in select()!\n");
  else if(ret == 0)
      fprintf(stderr, "Timeout occurred! No data after the specified time!\n");
  else {
      /*one of the both sockets has data to be received*/
      if(FD_ISSET(socketFromTelnetClient, &readfds)) {

          bytes_received = recv(socketFromTelnetClient, proxyPacket.payload, sizeof(char) * MAXPAYLOAD, 0);
          proxyPacket.type = APP_DATA_TYPE;
          send(socketFromSProxy, &proxyPacket, sizeof(int) + sizeof(char) * bytes_received, 0);
      }

      if(FD_ISSET(socketFromSProxy, &readfds)) {
          bytes_received = recv(socketFromSProxy, &proxyPacket, sizeof(proxyPacket_t), 0);
          if(proxyPacket.type == APP_DATA_TYPE)
            send(socketFromTelnetClient, proxyPacket.payload, sizeof(char) * bytes_received - sizeof(int), 0);
          if(proxyPacket.type == HEARTBEAT_TYPE) {
              heartBeatCount = proxyPacket.payload[0]++;
              send(socketFromSProxy, &proxyPacket, bytes_received, 0);
              fprintf(stderr,"cproxy received hearBeat.. sending %d\n", heartBeatCount);
          }

          //fprintf(stderr,"telnet -> user %d bytes\n", bytes_received);
      }


      memset(&proxyPacket, 0, sizeof(proxyPacket_t));
      bytes_received = 0;

  }
  }


  close (socketFromSProxy);
  close(socketFromTelnetClient);
  close (sockfd);

}



int main(int argc, char * argv[]){

    if(argc != 2) {
        fprintf(stderr, "Usage: %s <server_ip_addr>\n", argv[0]);
        exit(1);
    }

    startCProxyServer(argv[1]);

  return 0;
}
