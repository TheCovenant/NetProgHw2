#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
int main(int argc, char *argv[]) {
    int seedNumber;
    int longestWordLength;
    sscanf(argv[1], "%d", &seedNumber);
    sscanf(argv[3], "%d", &longestWordLength);
    srand(seedNumber);

    char* fileName = argv[2];
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

    fclose(file);
    free(file);

    printf("we have %d words\n", wordCount);
    char **wordsList = (char**)malloc(wordCount * sizeof(char*));
    for (int x = 0; x< wordCount; x++)
        wordsList[x]= (char *)malloc( longestWordLength * sizeof(char));
    char * line = NULL;
    size_t len = 0;
    int currentLine = 0;
    ssize_t read;

    file = fopen(fileName, "r");
    while ((read = getline(&line, &len, file)) != -1) {
        strcpy(wordsList[currentLine], line);
        currentLine += 1;
        if (currentLine == wordCount)
            break;
        
    }

    fclose(file);
    if(line)
        free(line);
    printf("First word is %s\n", wordsList[0]);
    printf("Last word is %s\n", wordsList[wordCount-1]);

    unsigned int secretWordLocation = rand() % wordCount;

    printf("secret word is %s\n", wordsList[secretWordLocation]);



    

    
    return 0;



    



}