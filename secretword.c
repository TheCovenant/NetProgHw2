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
        return 1;
    }

    for (newLineChar = getc(file); newLineChar != EOF; newLineChar = getc(file)) 
        if (newLineChar == '\n') // Increment count if this character is newline 
            wordCount =  wordCount + 1; 

    
    char wordsList[wordCount][1024];
    char * line = NULL;
    size_t len = 0;
    int currentLine = 0;
    ssize_t read;

    while ((read = getline(&line, &len, file)) != -1) {
        printf("Retrieved line of length %zu:\n", read);
        printf("%s", line);
        strcpy(wordsList[currentLine], line);
        currentLine += 1;
    }

    fclose(file);
    if(line)
        free(line);
    
    
    
    return 0;



    



}