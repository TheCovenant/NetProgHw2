//add name
// check name used or not
// not manage the number of user
//request name ?
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAXSTRING 100
char userName[5][MAXSTRING];

int checkName(char *name){

  int i ;
  for(i=0;i<5;i++){
    // if the name used
    if (strlen(userName[i])!=0){

    if (strcmp(userName[i],name)==0){
      return 0;
    }
  }
  }

  return 1;
}
void addName(char *name){

  int i;
  for(i=0;i<5;i++){
    if (strlen(userName[i])==0){

      strncpy(userName[i],name,strlen(name));
    }
  }


}
void removeName(char *name){

  int i;
  for(i=0;i<5;i++){
    if (strcmp(userName[i],name)==0){

      bzero(userName[i],strlen(userName[i]));
    }
  }

}

int main(int argc, char const *argv[]) {
  /* code */
  char  name[100];

  printf("print your username \n");

  scanf("%s",name);

  while(checkName(name)==0){
    printf("print your username \n");
    scanf("%s",name);

  }

  addName(name);
  removeName(name);

}
