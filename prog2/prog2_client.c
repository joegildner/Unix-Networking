// Quentin Jensen and Joseph Gildner
//

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

typedef struct initClientStruct{
	int init_sd;
	char init_player;
	uint8_t init_boardSize;
	uint8_t init_sec;
} initClientStruct;

//int recvGuesses(int);
//void recvBoard(int, const int, int);
//void guess(int);
void mainGameLoop(int, char, uint8_t, uint8_t);
initClientStruct initClient(int, char**);


/* main
 */
int main( int argc, char **argv) {

	initClientStruct c = initClient(argc, argv);

	mainGameLoop(c.init_sd, c.init_player, c.init_boardSize, c.init_sec);

	close(c.init_sd);
	exit(EXIT_SUCCESS);

}



void mainGameLoop(int sd, char pNum, uint8_t boardSize, uint8_t sec){
	uint8_t score1;
	uint8_t score2;
	uint8_t round = 0;
	char board[boardSize+1];


	int loops =0;
	while(loops<8){

		if(recv(sd, &score1, sizeof(uint8_t), MSG_WAITALL)<0){exit(1);}
		if(recv(sd, &score2, sizeof(uint8_t), MSG_WAITALL)<0){exit(1);}

		printf("%d - %d\n", score1, score2);
		int received;

		if(recv(sd, &round, sizeof(uint8_t), MSG_WAITALL)<0){exit(1);}
		received = recv(sd, board, sizeof(char)*boardSize, MSG_WAITALL);//<0){exit(1);}

		printf("%s\n",board);
		//printf("%s\n",board);

		loops++;

	}

}


/* play hangman
 * handles the main game logic on the client side, and receives
 * incoming info from the server
*
void playHangman(int sd){

	int guesses;
	char board[256];
	for(int i=0; i<256; i++){
		board[i] = '\0';
	}

	int totalguesses = recvGuesses(sd);
	recvBoard(sd, totalguesses, totalguesses);
	guess(sd);

	while(1){
		guesses = recvGuesses(sd);
		recvBoard(sd, totalguesses, guesses);
		guess(sd);
	}
}


	guess
 * gets user input, sends it to the server

void guess(int sd){
	char inputBuf[256];

	printf("Enter guess: ");

	if (fgets(inputBuf, sizeof(inputBuf), stdin) == NULL) {
		printf("Exiting.\n");
		exit(1);
	}
	char guess = inputBuf[0];

	//send guess
	if(send(sd, &guess, sizeof(char),0)<0){
		perror("send");
		exit(1);
	}

}



 receive guesses
 * 	handles the game-state message the server sends.
 *		if 0 or 255, receives and prints the board one last time,
 *		then closes the socket, then exits.

int recvGuesses(int sd){

	uint8_t intBuf;

	if(recv(sd, &intBuf, sizeof(intBuf), MSG_WAITALL)<0) {
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
06�


 receive board
 * receives the current board from the server

void recvBoard(int sd, const int totalGuesses, int remainGuesses){
	char board[totalGuesses+1];
	for(int i=0; i<=totalGuesses; i++) board[i] = '\0';

	int n = recv(sd, board, (sizeof(char) * totalGuesses), MSG_WAITALL);

	printf("Board: %s (%d guesses left)\n", board, remainGuesses);
}
*/


/* initialize client
 * handles the many procedures of initializing the client
 * so it can talk to a server
 */
initClientStruct initClient(int argc, char** argv){

	if( argc != 3 ) {
		fprintf(stderr,"Error: Wrong number of arguments\n");
		fprintf(stderr,"usage:\n");
		fprintf(stderr,"./client server_address server_port\n");
		exit(EXIT_FAILURE);
	}


	struct hostent *ptrh;
	struct protoent *ptrp;
	struct sockaddr_in sad;
	int sd;

	int port = atoi(argv[2]);
	char* host = argv[1];


	memset((char *)&sad,0,sizeof(sad));
	sad.sin_family = AF_INET;

	if (port > 0)
		sad.sin_port = htons((u_short)port);
	else {
		fprintf(stderr,"Error: bad port number %s\n",argv[2]);
		exit(EXIT_FAILURE);
	}



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

	char player;
	if(recv(sd, &player, sizeof(player), MSG_WAITALL) == -1){
		perror("recv");
		exit(1);
	}

	uint8_t boardSize;
	if(recv(sd, &boardSize, sizeof(boardSize), MSG_WAITALL) == -1){
		perror("recv");
		exit(1);
	}

	uint8_t sec;
	if(recv(sd, &sec, sizeof(sec), MSG_WAITALL) == -1){
		perror("recv");
		exit(1);
	}

	initClientStruct c = { 	.init_sd = sd,
												.init_player = player,
			  						.init_boardSize = boardSize,
									.init_sec = sec};

	return c;

}
