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

typedef struct initStruct {
	struct sockaddr_in init_cad;
	int init_sd;
	int  init_sd2;
	char* init_secretword;
} initStruct;

void verifyAndUpdate(char, char*, char*, int*);
void startGame(int, char*);
initStruct initServer(int, char**); 
void mainServerLoop(struct sockaddr_in, int, int, char*);

#define QLEN 6


/* main
 */
int main(int argc, char **argv) {

	initStruct c = initServer(argc, argv); 
		struct sockaddr_in cad 	= c.init_cad;
		int sd 						= c.init_sd;
		int sd2 						= c.init_sd2;
		char* secretword 			= c.init_secretword;

	signal(SIGCHLD,SIG_IGN);

	mainServerLoop(cad, sd, sd2, secretword);

	exit(EXIT_SUCCESS);
}


/* main server loop
 * accepts an incoming connection, then forks. child plays the game, 
 * parent returns to accepting more connections
 */
void mainServerLoop(struct sockaddr_in cad, int sd, int sd2, char* secretword){
		
	while (1) {

		int alen = sizeof(cad);
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
}



/*	startGame
 * -server-side logic of the game
 * -server can run multiple instances of this game for multiple clients
 *
 *	secretWord: user-input string that the client tries to guess
*/
void startGame(int sd, char* secretword) {


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
		(*guesses)--;
	}
}



/* initialize server
 * handles the many procedures of initializing our server,
 * and returns a struct of everything the main game needs to know
 * about
 */
initStruct initServer(int argc, char** argv){
	
	struct protoent *ptrp;
	struct sockaddr_in sad;
	struct sockaddr_in cad;
	int sd, sd2;
	int port;
	int optval = 1;

	if( argc != 3 ) {
		fprintf(stderr,"Error: Wrong number of arguments\n");
		fprintf(stderr,"usage:\n");
		fprintf(stderr,"./server server_port secretword\n");
		exit(EXIT_FAILURE);
	}

	char* secretword = argv[2];

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


	initStruct c =	{ .init_cad = cad, 
			  				.init_sd = sd,
							.init_sd2 = sd2,
							.init_secretword = secretword
						};	
	return c;
}







