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


/*
  In your terminal, 
  run the server by:
    ./your_program.out [seed] [port] [dictionary_file] [longest_word_length]

  to add a new client,
  open a new terminal tab and run:
    nc 127.0.0.1 [port]
*/

int findMaxFd(int tcp_socket, int * clients){
  int maxfd = tcp_socket;
  for (int i =0; i< 5; i++){
    if (maxfd < clients[i]){
      maxfd = clients[i];
    }
  }
  return maxfd;
}

int ClientNameExists(char * name, char * client_names[]){
  int exists = 0;
  for (int i =0; i<5; i++){
    if (strcmp(name, client_names[i]) == 0){
      exists = 1;
    }
  }
  return exists;
}


int main(int argc, char** argv)
{
  
  setvbuf( stdout, NULL, _IONBF, 0 );
  
  if (argc != 5){
    fprintf(stderr,"ERROR: Invalid argument(s)\n");
    return EXIT_FAILURE;
  }

  int port = atoi(argv[2]);
 
  /* creates the listener socket as TCP socket (SOCK_STREAM) */
  int tcp_socket = socket( PF_INET, SOCK_STREAM, 0 );

  // stores socket descriptors of current clients
  int clients[5];
  for (int i =0; i< 5; i++){
    clients[i] = 0;
  }

  // stores names of all current clients
  char ** client_names = malloc(5 * sizeof(char*));
  for (int i =0; i<5; i++){
    client_names[i] = (char*)malloc(1024);
  }

  char buffer[1024];




  /* sets up socket structures */
  fd_set rset;
  struct sockaddr_in server;
  server.sin_family = AF_INET;
  server.sin_addr.s_addr = htonl( INADDR_ANY );
  server.sin_port = htons( port );
  int len = sizeof( server );

  if ( bind( tcp_socket, (struct sockaddr *)&server, len ) == -1 ){
    perror( "bind() failed" );
    return EXIT_FAILURE;
  }

  printf("MAIN: Started server\n");

  if ( listen( tcp_socket, 5 ) == -1 ){
    perror( "listen() failed" );
    return EXIT_FAILURE;
  }
  
  struct sockaddr_in client;
  int fromlen = sizeof( client );
  int client_sock, maxfd;
  int ready = 0;


  while ( 1 ){
    FD_ZERO(&rset);
    FD_SET(tcp_socket, &rset);

    maxfd = findMaxFd(tcp_socket, clients);

    // adds current active client descriptors to the set
    for (int i =0; i<5; i++){
      if (clients[i] != 0){
        FD_SET(clients[i], &rset);
      }
    }

    ready = select(maxfd+1, &rset, NULL, NULL, NULL);

    // if a new client comes in
    if (FD_ISSET(tcp_socket, &rset)){

      client_sock = accept(tcp_socket, (struct sockaddr *)&client, (socklen_t*)&fromlen);

      if (client_sock < 0) { 
        perror("Unsuccessful accept system call"); 
        exit(EXIT_FAILURE); 
      }

      int n = 0;
      int k = 0;
      char name[1024];

      char askForName[] = "Welcome to Guess the Word, please enter your username.\n";
      k = send(client_sock, askForName, strlen(askForName), 0);

      // server hangs until this new client has entered its name
      while (1){

        n = recv(client_sock, name, 1024, 0);
        name[n-1]='\0'; // terminate the string for ease of comparison

        if (ClientNameExists(name, client_names)){
          char reAskForName[1024];
          sprintf(reAskForName, "Username %s is already taken, please enter a different username\n", name);
          k = send(client_sock, reAskForName, strlen(reAskForName), 0);
          continue;
        }
        
        else{
          char playMessage[1024];
          sprintf(playMessage, "Let's start playing, %s\n", name);
          k = send(client_sock, playMessage, strlen(playMessage), 0);
          break;
        }
      }

      // store the new client and its name to the current lists
      for (int i = 0; i < 5; i++){
        if( clients[i] == 0 ){
          clients[i] = client_sock;
          strcpy(client_names[i], name);
          break;
        }
      }
    }
}

  close(tcp_socket);

  for (int i =0; i<5; i++){
    free(client_names[i]);
  }
  free(client_names);

  return EXIT_SUCCESS;
}
