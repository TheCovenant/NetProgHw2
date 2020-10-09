#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

char* sort(char* word);
void response(char* user, char* word, char* sortedWord, char* guess);

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
    2. <sortedWord> is <word> sorted. (This is so that you don't sort the word each function call
 */
// Things I want to add:
//   Ensuring there aren't any non-letter characters
//   Lowercase...? I mean i was planning for the main function to lowercase stuff
//     so that you don't have to each time response is called
//   You can also return the error message instead of returning void (and printing the error message)
void response(char* user, char* word, char* sortedWord, char* guess) {
  if (strlen(word) != strlen(guess)) {
    printf("Invalid guess length. The secret word is %d letter(s).\n", (int)strlen(word));
    return;
  }
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
    printf("%s has correctly guessed the word %s\n", user, word);
    return;
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
  printf("%s guessed %s: %d letter(s) were correct and %d letter(s) were correctly placed.\n", user, guess, correctLetters, correctPositions);
}



int main(int argc, int** argv) {
 
  response("bob", "guess", sort("guess"), "snipe");
  response("bob", "guess", sort("guess"), "cross");
  response("bob", "gruel", sort("gruel"), "spill");
  response("bob", "gruel", sort("gruel"), "gruel");
  response("bob", "1", "", "abc");
}
