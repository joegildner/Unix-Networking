#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

int verify(char guess, char* secretword);

int main( int argc, char **argv) {
	char* secretword;

	if (argc != 2)
	{
		printf("Wrong number of input arguments\n");
		exit(1);
	}

	secretword = argv[1];
	int guesses = strlen(secretword);
	char* progress = malloc(sizeof(char) * guesses);

	for(int i=0; i<strlen(secretword); i++){
		progress[i] = '_';
		progress[i+1] = '\0';
	}

	while(guesses>=0){
		printf("Board: %s (%d guesses left)\n", progress, guesses);
		printf("Enter guess: ");
		char guess = getchar();

		int index = verify(guess, secretword);
		if(index != -1){
			progress[index] = guess;
		}else{
			printf("wrong");
		}
		fflush(stdin);
		//update progress
	}
}

int verify(char guess, char* secretword){
	char* c = strchr(secretword, guess);
	if(c != NULL){
		char* first = &secretword[0];
		int index = (c - first) / sizeof(char);
		return index;
	}
	else{
		return -1;
	}
}


char guess(char* secretword){


	//error check input(more than 1 char submitted)
	//check if input is in secret word
	//update progress if yes
	//nothing if no
}