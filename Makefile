word_guess.out: hw4.c
	gcc -o word_guess.out hw4.c
 
clean: 
	rm *.o word_guess.o *.out