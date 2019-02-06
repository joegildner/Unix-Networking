#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>

void verifyAndUpdate(char guess, char* secretword, char* board, int* guesses);
void startGame(int sd, char* secretword); 

#define QLEN 6 

int main(int argc, char **argv) {

	struct protoent *ptrp; 
	struct sockaddr_in sad; 
	struct sockaddr_in cad; 
	int sd, sd2; 
	int port; 
	int alen; 
	int optval = 1; 
	char buf[256]; 

	char* secretword = argv[2];

	if( argc != 3 ) {
		fprintf(stderr,"Error: Wrong number of arguments\n");
		fprintf(stderr,"usage:\n");
		fprintf(stderr,"./server server_port secretword\n");
		exit(EXIT_FAILURE);
	}

	memset((char *)&sad,0,sizeof(sad));
	sad.sin_family = AF_INET;	
	sad.sin_addr.s_addr = INADDR_ANY;   
     
	port = atoi(argv[1]); 
	if (port > 0) { 
		sad.sin_port = htons(port);
	} else { 
		fprintf(stderr,"Error: Bad port number %s\n",argv[1]);
		exit(EXIT_FAILURE);
	}
	
	if ( ((long int)(ptrp = getprotobyname("tcp"))) == 0) {
		fprintf(stderr, "Error: Cannot map \"tcp\" to protocol number");
		exit(EXIT_FAILURE);
	}

	sd = socket(AF_INET, SOCK_STREAM, ptrp->p_proto);
	if (sd < 0) {
		fprintf(stderr, "Error: Socket creation failed\n");
		exit(EXIT_FAILURE);
	}
	
	if( setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) < 0 ) {
		fprintf(stderr, "Error Setting socket option failed\n");
		exit(EXIT_FAILURE);
	}

	if (bind(sd, (struct sockaddr *)&sad, sizeof(sad)) < 0) {
		fprintf(stderr,"Error: Bind failed\n");
		exit(EXIT_FAILURE);
	}

	if (listen(sd, QLEN) < 0) {
		fprintf(stderr,"Error: Listen failed\n");
		exit(EXIT_FAILURE);
	}

/*================== END CONNECTION-SETUP ========================*/


	//main server loop
	while (1) {

		alen = sizeof(cad);
		if ( (sd2=accept(sd, (struct sockaddr *)&cad, &alen)) < 0) {
			fprintf(stderr, "Error: Accept failed\n");
			exit(EXIT_FAILURE);
		}
		
		const pid_t cpid = fork();
		switch(cpid) {
		
			//error
			case -1: {
		  		perror("fork");
		  		break;
			}
		
			//child
			case 0: {
				close(sd);
				startGame(sd2, secretword);
		  		exit(0);
		 		break;
			}
		
			//parent
			default: {
				close(sd2);
		  		break;
			}
		}
	}


}//end main



/*	startGame
 * -server-side logic of the game
 * -server can run multiple instances of this game for multiple clients
 *
 *	secretWord: user-input string that the client tries to guess
*/
void startGame(int sd, char* secretword) {
	
	//TODO: change so it's sending uint8_t's (so much easier with 16's)

	uint16_t outputBuf;
	char inputBuf[256];//idk if this will even be used
	char guessBuf;
	int guesses = strlen(secretword);
	char board[guesses];

	for(int i=0; i<strlen(secretword); i++){
		board[i] = '_';
		board[i+1] = '\0';
	}

	while(guesses>0){

		/*send guesses (if the client wins, send 255 instead)*/
		outputBuf = htons(guesses);
		if(send(sd, &outputBuf, sizeof(outputBuf),0)<=0){exit(1);}

		/*send board*/
		if(send(sd, &board, sizeof(board),0)<=0){exit(1);}
		break;//here for debug only

		//receive the guess
		if(recv(sd, &guessBuf, sizeof(guessBuf), 0)<=0) {exit(1);} 
		
		printf("%c\n", guessBuf);

		verifyAndUpdate(guess, secretword, board, &guesses);

/*
		if(!strcmp(secretword,board)){
			send a 255 next loop around for winning
		}
*/
	}
	
	//send a 0 for losing
}



/* verify
 *	-checks if guess is in secretword, and updates board 
 *	 and guesses accordingly
 * 
 * guess: client-input character
 * secretword: server-input string
 * board: the "board" of underscores and letters
 * guesses: number of guesses a player has left.
*/
void verifyAndUpdate(char guess, char* secretword, char* board, int* guesses){
	bool correct = false;

	for(int i=0; i<strlen(secretword);i++){
		if(guess==secretword[i]){
			board[i] = guess;
			correct = true;
		}
	}

	if(!correct){
//		printf("wrong\n");
		(*guesses)--;
	}
}

