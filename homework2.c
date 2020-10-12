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
#define MAX_NO_NAME 5


/*
  In your terminal,
  run the server by:
    ./your_program.out [seed] [port] [dictionary_file] [longest_word_length]
  to add a new client,
  open a new terminal tab and run:
    nc 127.0.0.1 [port]
*/
int clients[MAX_CLIENTS];
char* sort(char* word);
int response(char* message, char* user, char** secretWord, char* guess, char** wordsList, int wordCount);


int findMaxFd(int tcp_socket, int * clients, int * clientsNoName){
  int maxfd = tcp_socket;
  for (int i =0; i< MAX_CLIENTS; i++){
    if (maxfd < clients[i]){
      maxfd = clients[i];
    }
  }
  for (int i =0; i< MAX_NO_NAME; i++){
    if (maxfd < clientsNoName[i]){
      maxfd = clientsNoName[i];
    }
  }
  return maxfd;
}


int clientNameExists(char * name, char * client_names[]){
  int exists = 0;
  for (int i =0; i< MAX_CLIENTS; i++){
    if (strcmp(name, client_names[i]) == 0){
      exists = 1;
    }
  }
  return exists;
}


void sendAll(int * clients,char * msg){
  int i;
  for(i=0;i< MAX_CLIENTS;i++){
    if (clients[i] != 0){
       send(clients[i], msg, strlen(msg), 0);
    }
  }
}

void getWordCount(char* fileName, int* wordCount){
    FILE *file;
    char newLineChar;file = fopen(fileName, "r");

    if (file == NULL){
        printf("Could not open file %s\n", fileName);
        return;
    }
    *wordCount = 0;
    for (newLineChar = getc(file); newLineChar != EOF; newLineChar = getc(file))
        if (newLineChar == '\n') // Increment count if this character is newline
            *wordCount =  *wordCount + 1;

    fclose(file);
}

char** getDictionaryWords(int longestWordLength, int wordCount, char* fileName) {
    char** wordsList = (char**)malloc(wordCount * sizeof(char*));
    for (int x = 0; x< wordCount; x++)
        wordsList[x]= (char *)malloc( longestWordLength * sizeof(char));

    char * line = NULL;
    size_t len = 0;
    int currentLine = 0;
    ssize_t read;
    FILE *file;

    file = fopen(fileName, "r");
    while ((read = getline(&line, &len, file)) != -1) {
        // remove new line
        if ((line)[read - 1] == '\n')
        {
            (line)[read - 1] = '\0';
            --read;
        }
        strcpy(wordsList[currentLine], line);
        currentLine += 1;
        if (currentLine == wordCount)
            break;

    }

    fclose(file);



    return wordsList;
}

char* lowerCaseWord(char** word){
    char* wordToLower = *word;

    for (char *ch = wordToLower; *ch; ch++)
    {
        *ch = tolower((unsigned char) *ch);
    }

}

void freeWordsList(char ***wordsListPointer, int wordCount)
{
   char** wordsList = *wordsListPointer;
   for (int x = 0; x < wordCount; x++)
      free(wordsList[x]);
   free (wordsList);
   *wordsListPointer = NULL;
}

void getSecretWord(char** wordsList, int wordCount, char** word){
    char* secretWord = *word;
    unsigned int secretWordLocation = rand() % wordCount;
    strcpy(secretWord, wordsList[secretWordLocation]);
    lowerCaseWord(&secretWord);
}



int main(int argc, char** argv)
{
  //instant write and read, no buffer
  setvbuf( stdout, NULL, _IONBF, 0 );

  // judge the parameters
  if (argc != 5){
    fprintf(stderr,"ERROR: Invalid argument(s)\n");
    return EXIT_FAILURE;
  }

  // get the port number
  int port = atoi(argv[2]);

  /* creates the listener socket as TCP socket (SOCK_STREAM) */
  int tcp_socket = socket( PF_INET, SOCK_STREAM, 0 );


  // stores socket descriptors of current clients

  for (int i =0; i< MAX_CLIENTS; i++){
    clients[i] = 0;
  }

  int clientsNoName[MAX_NO_NAME];
  for (int i =0; i< MAX_NO_NAME; i++){
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

  //bind and listen to the socket
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
  int clientNum = 0; // current # of clients

  int seedNumber;
  int longestWordLength;
  sscanf(argv[1], "%d", &seedNumber);
  sscanf(argv[4], "%d", &longestWordLength);
  srand(seedNumber);
  char* fileName = argv[3];
  printf("filename is %s \n", fileName);

  int wordCount;
  getWordCount(fileName, &wordCount);

  printf("we have %d words\n", wordCount);
  char **wordsList = getDictionaryWords(longestWordLength, wordCount, fileName);

  char* secretWord =  (char*)malloc(1024);
  getSecretWord(wordsList, wordCount, &secretWord);

  printf("For the purpose of testing I will say the secret word is %s\n", secretWord);


  while ( 1 ){

    FD_ZERO(&rset);
    FD_SET(tcp_socket, &rset);


    /*
       adds socket descriptors from
       both lists of clients and clientsNoname
       to rset
     */
    for (int i =0; i< MAX_CLIENTS; i++){
      if (clients[i] != 0){
        FD_SET(clients[i], &rset);
      }
    }
    for (int i =0; i< MAX_NO_NAME; i++){
      if (clientsNoName[i] != 0){
        FD_SET(clientsNoName[i], &rset);
      }
    }


    maxfd = findMaxFd(tcp_socket, clients, clientsNoName);
    ready = select(maxfd+1, &rset, NULL, NULL, NULL);

    /*
       if a new client comes in
       add it to the list of clients without username
    */
    if (FD_ISSET(tcp_socket, &rset) && clientNum < 5){
      clientNum++;

      client_sock = accept(tcp_socket, (struct sockaddr *)&client, (socklen_t*)&fromlen);

      if (client_sock < 0) {
        perror("Unsuccessful accept system call");
        exit(EXIT_FAILURE);
      }

      // sends the message to ask client for the username
      char askForName[] = "Welcome to Guess the Word, please enter your username.\n";
      k = send(client_sock, askForName, strlen(askForName), 0);

      // adds the client to clientsNoName
      for (int i = 0; i < MAX_NO_NAME; i++){
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
  for (int i =0; i<MAX_NO_NAME; i++) {

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
          simply remove it from clientsNoName
        */
        if (valread == 0){
          close( socket );
          clientsNoName[i] = 0;
          clientNum--;
        }

        else{

          // check if client name already exists
          if (clientNameExists(name, client_names)){
            char reAskForName[1024];
            sprintf(reAskForName, "Username %s is already taken, please enter a different username\n", name);
            k = send(socket, reAskForName, strlen(reAskForName), 0);
           }

          else{
	    // get # clients with usernames
	    int numUsers = 0;
	    int oneAvailableSpot = 0;
	    for (int i = 0; i < MAX_CLIENTS; i++) {
	      if (clients[i] != 0) {
		numUsers += 1;
	      } else {
		oneAvailableSpot = 1; // is 1 if user is able to login. 0 otherwise
	      }
	    }
	    numUsers += oneAvailableSpot;
            char playMessage[1024 + 100]; // max size is actully 1024 + 85 or something...
            sprintf(playMessage, "Let's start playing, %s\nThere are %d player(s) playing. The secret word is %d letter(s).\n", name,  numUsers, (int)strlen(secretWord));
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
        // and remove the name as well
        if (valread == 0){
          close(socket);
          clientNum--;
          clients[i] = 0;
          strcpy(client_names[i], "");
          printf("client terminating...\n");
        }

        // ** CJ's code goes here **
        // if received word guess from client
        else if (valread > 0){
    // This is cause the newline character is read in as well...
    // I just replaced it with a end of string character
    buffer[valread-1] = '\0';
    char message[1024];
    int sendToAll = response(message, client_names[i], &secretWord, (char*)buffer, wordsList, wordCount);
    if (sendToAll == 0) {
      send(socket, message, strlen(message), 0);
     }
     else if (sendToAll ==1) {
      sendAll(clients, message);
    }
        }
      }
    }
  }
}

freeWordsList(&wordsList, wordCount);
free(secretWord);

  close(tcp_socket);

  for (int i =0; i< MAX_CLIENTS; i++){
    free(client_names[i]);
  }
  free(client_names);

  return EXIT_SUCCESS;
}



// ~~~~~~~~~~~~~~~~~  Guess Handling ~~~~~~~~~~~~~~~~~

/*
  word: word to be sorted
  Since words are assumed to be short, will be using an O(n^2) time sort
*/
char* sort(char* word) {
  char* sorted = malloc (sizeof (char) * strlen(word));
  strcpy(sorted, word);
  for (int i = 0; i < strlen(word); i++) {
    int min = i;
    for (int j = i; j < strlen(word); j++) {
      if (sorted[min] > sorted[j]) {
  min = j;
      }
    }
    char temp = sorted[i];
    sorted[i] = sorted[min];
    sorted[min] = temp;
  }
  return sorted;
}


// This returns a char* (and doens't directly make word lowercase) cause
// in my testing functions i'm an idiot and forgot char* = "word" can't be modified
// you can just replace with commented code if you have malloc'ed strings
char* lowercase(char* word) {
  char* lower = malloc (sizeof (char) * strlen(word));
  for (int i = 0; i < strlen(word); i++) {
    // word[i] = tolower(word[i]);
    lower[i] = tolower(word[i]);
  }
  return lower;
}


/*
  ARGUMENTS:
  word: word to be guessed
  sortedWord: word to be guessed (sorted)
  guess: client's guess
  ASSUMES:
    1. words/guesses are lowercase (so that you don't call lowercase each function call)
  RETURNS:
    0 if message should be sent to one person
    1 if message should be sent to everyone
 */
int response(char* message, char* user, char** secretWord, char* guess, char** wordsList, int wordCount) {
  char* word = *secretWord;
  printf("Comparing guess to word %s\n", word);
  if (strlen(word) != strlen(guess)) {
    sprintf(message, "Invalid guess length. The secret word is %d letter(s).\n", (int)strlen(word));
    return 0;
  }
  char* sortedWord = sort(word);
  char* sortedGuess = sort(guess);
  int correctPositions = 0;

  // correctPositions
  for (int i = 0; i < strlen(word); i++) {
    if (word[i] == guess[i]) {
      correctPositions += 1;
    }
  }

  // Check for correct guess
  if (correctPositions == strlen(word)) {
    sprintf(message, "%s has correctly guessed the word %s\n", user, word);
    // disconnect the clients
    sendAll(clients, message);
    int k;
    for(k=0;k<5;k++){
      if (clients[k] !=0){
        close(clients[k]);
      }

    }
    printf("Now choosing new word\n");
    getSecretWord(wordsList, wordCount, &word);
    printf("For the purpose of debugging, the new word is %s\n", word);
    // return 2 if disconnect
    return 2;
  }

  // correctLetters
  int wordIndex = 0;
  int guessIndex = 0;
  int correctLetters = 0;
  while ((wordIndex < strlen(sortedWord)) && (guessIndex < strlen(sortedGuess))) {
    if (sortedWord[wordIndex] == sortedGuess[guessIndex]) {
      correctLetters += 1;
      wordIndex += 1;
      guessIndex += 1;
    } else if (sortedWord[wordIndex] < sortedGuess[guessIndex]) {
      wordIndex += 1;
    } else {
      guessIndex += 1;
    }
  }
  sprintf(message, "%s guessed %s: %d letter(s) were correct and %d letter(s) were correctly placed.\n", user, guess, correctLetters, correctPositions);
  return 1;
}
