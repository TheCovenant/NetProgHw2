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


#define MAX_CLIENTS 5
#define MAX_CLIENTS_NO_NAME 20
/*
  In your terminal, 
  run the server by:
    ./your_program.out [seed] [port] [dictionary_file] [longest_word_length]

  to add a new client,
  open a new terminal tab and run:
    nc 127.0.0.1 [port]
*/

int findMaxFd(int tcp_socket, int * clients, int * clientsNoName){
  int maxfd = tcp_socket;
  for (int i =0; i< MAX_CLIENTS; i++){
    if (maxfd < clients[i]){
      maxfd = clients[i];
    }
  }
  for (int i =0; i< MAX_CLIENTS_NO_NAME; i++){
    if (maxfd < clientsNoName[i]){
      maxfd = clientsNoName[i];
    }
  }
  return maxfd;
}


int ClientNameExists(char * name, char * client_names[]){
  int exists = 0;
  for (int i =0; i< MAX_CLIENTS; i++){
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
  int clients[MAX_CLIENTS];
  for (int i =0; i< MAX_CLIENTS; i++){
    clients[i] = 0;
  }

  int clientsNoName[MAX_CLIENTS_NO_NAME];

  for (int i =0; i< MAX_CLIENTS_NO_NAME; i++){
    clientsNoName[i] = 0;
  }

  // stores names of all current clients
  char ** client_names = malloc(MAX_CLIENTS * sizeof(char*));
  for (int i =0; i<MAX_CLIENTS; i++){
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
  int client_sock, maxfd, socket;
  int ready = 0;

  int valread = 0;
  char name[1024];
  int k = 0;

 
  while ( 1 ){

    FD_ZERO(&rset);
    FD_SET(tcp_socket, &rset);


    // adds socket descriptors from
    // both lists of clients and clientsNoname
    // to rset
    for (int i =0; i< MAX_CLIENTS; i++){
      if (clients[i] != 0){
        FD_SET(clients[i], &rset);
      }
    }
    for (int i =0; i< MAX_CLIENTS_NO_NAME; i++){
      if (clientsNoName[i] != 0){
        FD_SET(clientsNoName[i], &rset);
      }
    }

    maxfd = findMaxFd(tcp_socket, clients, clientsNoName);
    ready = select(maxfd+1, &rset, NULL, NULL, NULL);

    /* if a new client comes in
       add it to the list of clients without username
    */
    if (FD_ISSET(tcp_socket, &rset)){

      client_sock = accept(tcp_socket, (struct sockaddr *)&client, (socklen_t*)&fromlen);

      if (client_sock < 0) { 
        perror("Unsuccessful accept system call"); 
        exit(EXIT_FAILURE);
      }

      // sends the message to ask client for the username
      char askForName[] = "Welcome to Guess the Word, please enter your username.\n";
      k = send(client_sock, askForName, strlen(askForName), 0);

      // adds the client to clientsNoName
      for (int i = 0; i < MAX_CLIENTS_NO_NAME; i++){
        if( clientsNoName[i] == 0 ){
          clientsNoName[i] = client_sock;
          break;
        }
      }
    }


  /* 
     loops through each socket in clientsNoName
     and check if the client has entered the username.
  */
  for (int i =0; i<MAX_CLIENTS_NO_NAME; i++) {

    socket = clientsNoName[i];
    if (socket != 0){
     if (FD_ISSET( socket , &rset)){

        /* 
         sets the socket to NON BLOCKING so it doesn't
         block on empty pipe
        */
        fcntl(socket, F_SETFL, O_NONBLOCK);
        valread = read(socket, name, 1024);
        name[valread-1]='\0';

        /* 
          if client closes socket before entering username
          simply remove it from list of client without names
        */ 
        if (valread == 0){
          close( socket );
          clientsNoName[i] = 0;
        }

        else{

          // check if client name already exists
          if (ClientNameExists(name, client_names)){
            char reAskForName[1024];
            sprintf(reAskForName, "Username %s is already taken, please enter a different username\n", name);
            k = send(socket, reAskForName, strlen(reAskForName), 0);
           }

          else{
            char playMessage[1024];
            sprintf(playMessage, "Let's start playing, %s\n", name);
            k = send(socket, playMessage, strlen(playMessage), 0);
            clientsNoName[i] = 0; // remove client from clientsNoName 

            // adds the client to the list of active clients
            // if there are less than 5 active clients
            for (int i = 0; i < MAX_CLIENTS; i++){
              if ( clients[i] == 0 ){
              clients[i] = socket;
              strcpy(client_names[i], name);
              break;
              }
            }
          }
        }
      }
    }
  }


  /*
    Loop through each socket in list of
    active clients and check if there are
    any incoming word guesses
  */
  for (int i =0; i< MAX_CLIENTS; i++) {

    socket = clients[i];

    if (clients[i] != 0){

      if (FD_ISSET(socket, &rset)){

        // sets the socket to NON BLOCKING so it doesn't
        // block on empty pipe
        fcntl(socket, F_SETFL, O_NONBLOCK);
        valread = read(socket, buffer, 1024);

        // if client terminates, free the client from the list
        // and remove the name
        if (valread == 0){
          close(socket);
          clients[i] = 0;
          strcpy(client_names[i], "");
          printf("client terminating...\n");
        }

        // if received word guess from clients
        else if (valread > 0){
          printf("received message from socket %d\n", socket);
        }   
      }
    }
  }
}

  close(tcp_socket);

  for (int i =0; i< MAX_CLIENTS; i++){
    free(client_names[i]);
  }
  free(client_names);

  return EXIT_SUCCESS;
}
