#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
int main(int argc, char *argv[]) {
    char* fileName = argv[1];
    printf("filename is %s \n", fileName);

    FILE *file;
    char newLineChar;
    int wordCount = 0;

    file = fopen(fileName, "r");

    if (file == NULL){
        printf("Could not open file %s\n", fileName);
        return 0;
    }

    for (newLineChar = getc(file); newLineChar != EOF; newLineChar = getc(file)) 
        if (newLineChar == '\n') // Increment count if this character is newline 
            wordCount =  wordCount + 1; 
    



}