#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

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
    free(file);
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
        strcpy(wordsList[currentLine], line);
        currentLine += 1;
        if (currentLine == wordCount)
            break;
        
    }

    fclose(file);
    if(line)
        free(line);

    

    return wordsList;
}

void freeWordsList(char ***wordsListPointer, int wordCount)
{
   char** wordsList = *wordsListPointer;
   for (int x = 0; x < wordCount; x++)
      free(wordsList[x]);
   free (wordsList);
   *wordsListPointer = NULL;
}
 

int main(int argc, char *argv[]) {
    int seedNumber;
    int longestWordLength;
    sscanf(argv[1], "%d", &seedNumber);
    sscanf(argv[3], "%d", &longestWordLength);
    srand(seedNumber);
    char* fileName = argv[2];
    printf("filename is %s \n", fileName);

    int wordCount;
    getWordCount(fileName, &wordCount);

    printf("we have %d words\n", wordCount);
    char **wordsList = getDictionaryWords(longestWordLength, wordCount, fileName);

    
   
    printf("First word is %s\n", wordsList[0]);
    printf("Last word is %s\n", wordsList[wordCount-1]);

    unsigned int secretWordLocation = rand() % wordCount;

    printf("secret word is %s\n", wordsList[secretWordLocation]);

    freeWordsList(&wordsList, wordCount);

    
    return 0;
}