#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>

void verify(char guess, char* secretword, char* progress, int* guesses);

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
	
	char guess;

	while(guesses>0){
		printf("Board: %s (%d guesses left)\n", progress, guesses);
		printf("Enter guess: ");
		scanf(" %c", &guess);
		verify(guess, secretword, progress, &guesses);
		if(!strcmp(secretword,progress)){
			printf("You Win!\n");
			exit(0);
		}
	}

	printf("You lose! cry about it, idiot.\n");
	exit(0);
}

void verify(char guess, char* secretword, char* progress, int* guesses){
	bool right = false;
	for(int i=0; i<strlen(secretword);i++){
		if(guess==secretword[i]){
			progress[i] = guess;
			right = true;
		}
	}

	if(!right){
		printf("wrong\n");
		(*guesses)--;
	}
}
