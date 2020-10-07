#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <pthread.h>
#include <sys/select.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <fcntl.h>
#include <dirent.h>
#include <ctype.h>

#define MAX_CLIENT 5


int main(int argc, char** argv)
{
  setvbuf( stdout, NULL, _IONBF, 0 );
  if (argc != 5){
    fprintf(stderr,"ERROR: Invalid argument(s)\n");
    return EXIT_FAILURE;
  }

 
  /* Create the listener socket as TCP socket (SOCK_STREAM) */
  int tcp_socket = socket( PF_INET, SOCK_STREAM, 0 );
  int clients[MAX_CLIENT];
  int client_sock, addrlen, maxfd, socket;
  int ready =0;
  char buffer[1024];

  for (int i =0; i< MAX_CLIENT; i++){
    clients[i] = 0;
  }

  if(setsockopt(tcp_socket, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, 
        sizeof(opt)) < 0 ) 
    { 
      perror("setsockopt"); 
      exit(EXIT_FAILURE); 
    }

  /* socket structures */
  fd_set rset;
  struct sockaddr_in server;
  server.sin_family = AF_INET;
  server.sin_addr.s_addr = htonl( INADDR_ANY );
  server.sin_port = htons( 0 );
  int len = sizeof( server );

  /* attempt to bind (or associate) port 8123 with the socket */
  if ( bind( tcp_socket, (struct sockaddr *)&server, len ) == -1 )
  {
    perror( "bind() failed" );
    return EXIT_FAILURE;
  }


  printf("MAIN: Started server\n");
  printf("MAIN: Listening for TCP connections on port: \n");

  /* identify this port as a TCP listener port */
  if ( listen( tcp_socket, 5 ) == -1 )
  {
    perror( "listen() failed" );
    return EXIT_FAILURE;
  }

  

  while ( 1 )
  {
    FD_ZERO(&rset);
    FD_SET(tcp_socket, &rset);
    maxfd = findMaxFd;


    ready = select(maxfd+1, &rset, NULL, NULL, NULL);

    if (FD_ISSET(tcp_socket, &rset)){
      client_sock = accept(master_socket, (struct sockaddr *)&address, (socklen_t*)&addrlen)
      if (client_sock < 0) 
        { 
          perror("Unsuccessful accept system call"); 
          exit(EXIT_FAILURE); 
        }
      printf("successful client connected\n");
    }

}
  close(tcp_socket);
  return EXIT_SUCCESS;
}
