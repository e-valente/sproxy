/*sproxy.c connects to the real telnet server
 * It should listen to port 6200
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

#define MAXPAYLOAD 10000
#define LISTEN_PORT 6200
#define TELNET_PORT 23

int startTelnetClient(char *serverIPAddr, int port);
void startSProxyServer();

int socketFromTelnetServer, socketFromCProxy;


char payload[MAXPAYLOAD];

void error(char *msg){
  perror(msg);
  exit(1);
}

/*It will receive incoming connections from the CProxy client*/
void startSProxyServer() {

  int sockfd, bytes_received, bytes_sent;
  socklen_t clientlen;
  struct sockaddr_in local_serv_addr, client_addr, serv_addr;
  fd_set readfds;
  struct timeval tv;
  int ret, n;

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
  socketFromCProxy = accept(sockfd, (struct sockaddr *) &client_addr, &clientlen);

  if(socketFromCProxy < 0)
    error("ERROR on accept");

  /*connects to the *real* telnet server*/
  socketFromTelnetServer = socket (AF_INET, SOCK_STREAM, 0);
  if (socketFromTelnetServer < 0)
    error ("ERROR opening socket\n");

  memset(&serv_addr, 0, sizeof(serv_addr));

  serv_addr.sin_family = AF_INET;
  /*convert and copy server's ip addr into serv_addr*/
  if(inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) < 0)
    error ("ERROR, copying ip addr\n");

  serv_addr.sin_port = htons(TELNET_PORT);
   if (connect(socketFromTelnetServer,(struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
    error ("ERROR connecting");

   /*fprintf(stderr, "TELNET:\n%s\n", payload);*/


  /*At this point we have two active sockets:
   * one is socketFromTelnetClient connected to the end user
   * and socketFromTelnetClient connected to the SProxy
   * */


  while(1) {

  /*clear the set ahead of time*/
  FD_ZERO(&readfds);
  //FD_SET(socketFromTelnetServer, &readfds);
  /*this socket is going to be "greater"*/
  FD_SET(socketFromCProxy, &readfds);
  FD_SET(socketFromTelnetServer, &readfds);


  /*setting our delay for the events*/
  tv.tv_sec = 5;
  tv.tv_usec = 500000;

  /*param for select()*/
  n = socketFromTelnetServer+ 1;

  ret = select(n, &readfds, NULL, NULL, &tv);

  if(ret == -1)
      fprintf(stderr, "Error in select()!\n");
  else if(ret == 0)
      fprintf(stderr, "Timeout occurred! No data after the specified time!\n");
  else {
      /*one of the both sockets has data to be received*/
      if(FD_ISSET(socketFromTelnetServer, &readfds)) {
          bytes_received = recv(socketFromTelnetServer, payload, sizeof(char) * MAXPAYLOAD, 0);
          send(socketFromCProxy, payload, sizeof(char) * bytes_received, 0);
          fprintf(stderr,"telnet server -> cproxy %d bytes\n", bytes_received);
      }

      if(FD_ISSET(socketFromCProxy, &readfds)) {
          bytes_received = recv(socketFromCProxy, payload, sizeof(char) * MAXPAYLOAD, 0);
          send(socketFromTelnetServer, payload, sizeof(char) * bytes_received, 0);
          fprintf(stderr,"cproxy -> telnet server %d bytes\n", bytes_received);
      }


      memset(payload, 0, sizeof(char)* MAXPAYLOAD);
      bytes_received = 0;

  }
  }


  close (socketFromCProxy);
  close(socketFromTelnetServer);
  close (sockfd);

}



int main(int argc, char * argv[]){

 startSProxyServer();

  return 0;
}
