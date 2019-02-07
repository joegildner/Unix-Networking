// Quentin Jensen and Joseph Gildner
// CSCI 367
// 02/06/2019

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

void playHangman(int sd);
int recvGuesses(int sd);
void recvBoard(int sd, const int size, int remain);
void guess(int sd);

/* Main entrypoint,
* The majority of the main method is code to set up the connection with the server
* once established, the game code immediately begins execution
*/
int main( int argc, char **argv) {
	struct hostent *ptrh;
	struct protoent *ptrp;
	struct sockaddr_in sad;
	int sd;
	int port;
	char *host;
	int n;
	char buf[256];

	memset((char *)&sad,0,sizeof(sad));
	sad.sin_family = AF_INET;

	if( argc != 3 ) {
		fprintf(stderr,"Error: Wrong number of arguments\n");
		fprintf(stderr,"usage:\n");
		fprintf(stderr,"./client server_address server_port\n");
		exit(EXIT_FAILURE);
	}

	port = atoi(argv[2]);
	if (port > 0)
		sad.sin_port = htons((u_short)port);
	else {
		fprintf(stderr,"Error: bad port number %s\n",argv[2]);
		exit(EXIT_FAILURE);
	}

	host = argv[1];


	ptrh = gethostbyname(host);
	if ( ptrh == NULL ) {
		fprintf(stderr,"Error: Invalid host: %s\n", host);
		exit(EXIT_FAILURE);
	}

	memcpy(&sad.sin_addr, ptrh->h_addr, ptrh->h_length);

	if ( ((long int)(ptrp = getprotobyname("tcp"))) == 0) {
		fprintf(stderr, "Error: Cannot map \"tcp\" to protocol number");
		exit(EXIT_FAILURE);
	}

	sd = socket(PF_INET, SOCK_STREAM, ptrp->p_proto);
	if (sd < 0) {
		fprintf(stderr, "Error: Socket creation failed\n");
		exit(EXIT_FAILURE);
	}

	if (connect(sd, (struct sockaddr*)&sad, sizeof(sad)) < 0) {
		fprintf(stderr,"connect failed\n");
		exit(EXIT_FAILURE);
	}

//==== actual game starts here ====================

	playHangman(sd);
	close(sd);
	exit(EXIT_SUCCESS);
}

/*playHangman
 *Play hangman runs the game of hangman with the data received from the server
*/
void playHangman(int sd){

	int guesses;
	char board[256];
	for(int i=0; i<256; i++){
		board[i] = '\0';
	}

	//Initialize board with the first turn
	int totalguesses = recvGuesses(sd);
	recvBoard(sd, totalguesses, totalguesses);
	guess(sd);

	//main game loop, exits once the server sends 255 guess signal
	while(1){
		guesses = recvGuesses(sd);
		recvBoard(sd, totalguesses, guesses);
		guess(sd);
	}
}


/*obtain character guess from stdin from the user
 *
*/
void guess(int sd){
	char inputBuf[256];

	printf("Enter guess: ");

	if (fgets(inputBuf, sizeof(inputBuf), stdin) == NULL) {
		printf("Exiting.\n");
		exit(1);
	}
	char guess = inputBuf[0];

	//send guess to server
	if(send(sd, &guess, sizeof(char),0)<0){
		perror("send");
		exit(1);
	}

}



/* receive guesses
 * 	handles the game-state message the server sends.
 *		if 0 or 255, receives and prints the board one last time,
 *		then closes the socket, then exits.
*/
int recvGuesses(int sd){

	uint8_t intBuf;

	if(recv(sd, &intBuf, sizeof(uint8_t), MSG_WAITALL)<0) {
		perror("recv");
		exit(1);
	}
	int guesses = intBuf;

	if(guesses==0){
		printf("%s\n", "You Lost");
		close(sd);
		exit(0);
	}

	else if(guesses==255){
		printf("%s\n", "You Win");
		close(sd);
		exit(0);
	}

	return guesses;
}


/* get the current game board from the server, with the
* letters that have been correctly guessed filled in!
*/
void recvBoard(int sd, const int totalGuesses, int remainGuesses){
	char board[totalGuesses+1];
	for(int i=0; i<=totalGuesses; i++) board[i] = '\0';

	int n = recv(sd, board, (sizeof(char) * totalGuesses), MSG_WAITALL);

	printf("Board: %s (%d guesses left)\n", board, remainGuesses);
}
