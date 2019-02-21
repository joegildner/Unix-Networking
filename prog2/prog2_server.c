//Joseph Gildner and Quentin Jensen

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <signal.h>

typedef struct initServerStruct {
	struct sockaddr_in init_cad;
	int init_sd;
	uint8_t init_boardSize;
	uint8_t init_sec;
} initServerStruct;

void startGameSession(int, int, uint8_t, uint8_t);
initServerStruct initServer(int, char**);
void mainServerLoop(struct sockaddr_in, int, uint8_t, uint8_t);
int takeTurn(int sd, char board[], uint8_t sec);

#define QLEN 6



/* main
 */
int main(int argc, char **argv) {

	initServerStruct c = initServer(argc, argv);

	mainServerLoop(c.init_cad, c.init_sd, c.init_boardSize, c.init_sec);

	exit(EXIT_SUCCESS);
}



/* main server loop
 * accepts an incoming connection, then forks. child plays the game,
 * parent returns to accept more connections
 */
void mainServerLoop(struct sockaddr_in cad, int sd, uint8_t boardSize, uint8_t sec){

	//int player = 0;
	//player = (player+1)%2 ???
	//how can the different threads know which player they are?

	//TODO: how to have 2 players enter the same game session?
	int sd2;
	int sd3;

	while (1) {

		socklen_t alen = sizeof(cad);
		if ( (sd2=accept(sd, (struct sockaddr *)&cad, &alen)) < 0) {
			fprintf(stderr, "Error: Accept failed\n");
			exit(EXIT_FAILURE);
		}
		printf("Client 1 connected, waiting for Client 2...\n");
		if ( (sd3=accept(sd, (struct sockaddr *)&cad, &alen)) < 0) {
			fprintf(stderr, "Error: Accept failed\n");
			exit(EXIT_FAILURE);
		}
		printf("Client 2 connected, Game Starting...\n");

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

				//i don't think this is how it works?
				//if player=0, block until signal is heard
				//if player=1, signal so both clients start the game simultaneously
				//this way, both players are connecting to the same socket before it's closed

				startGameSession(sd2, sd3, boardSize, sec);
				printf("HELLO?");

		  		exit(0);
		 		break;
			}

			//parent
			default: {
				close(sd2);
				close(sd3);
		  		break;
			}
		}
	}
}




void startGameSession(int p1, int p2, uint8_t boardSize, uint8_t sec){

	char c = player + '0';//fun trick for 0-9

	if(send(sd, &c, sizeof(char),0)<=0){exit(1);}
	if(send(sd, &boardSize, sizeof(uint8_t),0)<=0){exit(1);}
	if(send(sd, &sec, sizeof(uint8_t),0)<=0){exit(1);}


	//TODO: asynchronously receive a signal that the other player won

	uint8_t round = 0;
	uint8_t score = 0;
	while(score<3){

		//send score
		//send round number
		//char board[boardSize] = generateBoard(boardSize);//no null byte
		//send board

		//TODO: figure out how to tell a client its their turn

		//if round is odd, player 1 takes turn first. Afterward, block until player 2 finishes turn.
		//Likewise, player 2 blocks until player 1 finishes turn.

		//send 'Y' if its this players turn
		//else send 'N'

		//NOTE: this isn't right. as long as both players guess correctly
		//a round should go on forever

		if(round%2==1 && player==1){
			//takeTurn();
			//block send N
		}
		else if(round%2==1 && player==2){
			//block send N
			//takeTurn()
		}
		else if(round%2==0 && player==1){
			//block send N
			//takeTurn()
		}
		else{
			//takeTurn()
			//block send N
		}

	}

	//send signal that this client won

}

//return true if the turn was successful (active player gets a point)
//return false if the turn was a fail (inactive player gets a point)
int takeTurn(int sd, char board[], uint8_t sec){
	//char** usedWords;
	//send Y
	//start timer
	//	while timer is active
	//		recv word from client
	//
	//		if(validateWord(word, board, usedWord){
	//			send a uint8_t 1 to active player
	//			send board size to inactive player
	//			send board to inactive player
	//			return true;
	//		}
	//		else{
	//			break;
	//		}
	//	}
	//	return false;
	return 0;
}


//not sure if char** for usedWords is right
int validateWord(char word[], char board[], char* usedWords[]){
	//check if word is in the dictionary
	//check that this word hasn't been used this round
	//check if that every letter in word corresponds to a letter in board
	//		^kinda like hangman

	//return true if valid
	return 0;
}

/*
char[] generateBoard(uint8_t boardSize){
	//no null byte
}
*/




/*	startGame
 * -server-side logic of the game
 * -server can run multiple instances of this game for multiple clients
 *
 *	secretWord: user-input string that the client tries to guess
*/
/*void startGame(int sd, char* secretword) {


	uint8_t outputBuf;
	char inputBuf[256];
	char guessBuf;
	int guesses = strlen(secretword);
	char board[guesses];

	for(int i=0; i<strlen(secretword); i++){
		board[i] = '_';
		board[i+1] = '\0';
	}

	while(guesses>=0){

		outputBuf = guesses;
		if(send(sd, &outputBuf, sizeof(uint8_t),0)<=0){exit(1);}

		if(send(sd, &board, sizeof(board),0)<=0){exit(1);}

		fdatasync(sd);

		//receive the guess
		int received = 0;
		while(received == 0){
			received += recv(sd, &guessBuf, sizeof(guessBuf), MSG_WAITALL);
		}

		printf("%d\n", guesses);

		verifyAndUpdate(guessBuf, secretword, board, &guesses);

		if(!strcmp(secretword,board)){
			uint8_t win = 255;
			if(send(sd, &win, sizeof(uint8_t),0)<=0){exit(1);};
			break;
		}
	}
}



 verify
 *	-checks if guess is in secretword, and updates board
 *	 and guesses accordingly
 *
 * guess: client-input character
 * secretword: server-input string
 * board: the "board" of underscores and letters
 * guesses: number of guesses a player has left.

void verifyAndUpdate(char guess, char* secretword, char* board, int* guesses){
	bool correct = false;

	for(int i=0; i<strlen(secretword);i++){
		if(guess==secretword[i]){
			board[i] = guess;
			correct = true;
		}
	}

	if(!correct){
		(*guesses)--;
	}
}
*/


/* initialize server
 * handles the many procedures of initializing our server,
 * and returns a struct of everything the main game needs to know
 * about
 */
initServerStruct initServer(int argc, char** argv){

	if( argc != 5 ) {
		fprintf(stderr,"Error: Wrong number of arguments\n");
		fprintf(stderr,"usage:\n");
		fprintf(stderr,"./server server_port secretword\n");
		exit(EXIT_FAILURE);
	}

	struct protoent *ptrp;
	struct sockaddr_in sad;
	struct sockaddr_in cad;
	int sd = 0;
	int optval = 1;

	uint16_t port = atoi(argv[1]);
	uint8_t boardSize = atoi(argv[2]);
	uint8_t sec = atoi(argv[3]);
	//char* pathToDict = argv[4];


	memset((char *)&sad,0,sizeof(sad));
	sad.sin_family = AF_INET;
	sad.sin_addr.s_addr = INADDR_ANY;

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

	initServerStruct c =	{ .init_cad = cad,
		.init_sd = sd,
		.init_boardSize = boardSize,
		.init_sec = sec
	};

	signal(SIGCHLD,SIG_IGN);

	return c;
}
