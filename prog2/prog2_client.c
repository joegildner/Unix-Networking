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
bool takeTurn(int sd);
bool waitTurn(int sd);
void playRound(int sd);


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
	char board[boardSize];

	while(1){
		if(recv(sd, &score1, sizeof(uint8_t), MSG_WAITALL)<0){exit(1);}
		if(recv(sd, &score2, sizeof(uint8_t), MSG_WAITALL)<0){exit(1);}


		if(recv(sd, &round, sizeof(uint8_t), MSG_WAITALL)<0){exit(1);}
		if(recv(sd, board, sizeof(char)*boardSize, MSG_WAITALL)<0){exit(1);}

		board[boardSize] = '\0';

		printf("Round %d...\n",round);
		printf("Score is %d-%d\n", score1, score2);
		printf("Board: %s\n",board);

		playRound(sd);

		if(score1==3){
			printf("Player 1 won!\n");
			break;	
		}				
		if( score2==3){
			printf("Player 1 won!\n");
			break;
		}
	}


}

void playRound(int sd){
	bool roundIsRunning = true;

	while(roundIsRunning){
		char isActive;
		if(recv(sd, &isActive, sizeof(uint8_t), MSG_WAITALL)<0){
			exit(1);
		}
		if(isActive == 'Y')
			roundIsRunning = takeTurn(sd);
		else if(isActive == 'N')
			roundIsRunning = waitTurn(sd);
	}
}


bool takeTurn(int sd){

	uint8_t isValidWord =1;
	printf("Your turn, enter word: ");

		char input[1024];
		char word[255];

		for(int i=0; i<1024; i++){
			input[i] = '\0';
		}

		scanf("%s",input);

		for(int i=0; i<255; i++){
			word[i] = input[i];
		}

		int wordSize = strlen(word);

		if(send(sd, &wordSize, sizeof(uint8_t),0)<0){exit(1);}
		if(send(sd, word, sizeof(char)*wordSize,0)<0){exit(1);}
		if(recv(sd, &isValidWord, sizeof(uint8_t), MSG_WAITALL)<0){exit(1);}

		if(isValidWord){
			printf("Valid word!\n");
		}
		else{
			printf("Invalid word!\n");
		}

		return isValidWord;

}

bool waitTurn(int sd){
	uint8_t wordSize = 0;
	char word[255];

	printf("Please wait for opponent to enter word...\n");

	if(recv(sd, &wordSize, sizeof(uint8_t), MSG_WAITALL)<0){exit(1);}
	printf("%d\n", wordSize);
	if(wordSize > 0){
		if(recv(sd, word, sizeof(char)*wordSize, MSG_WAITALL)<0){exit(1);}
		word[wordSize] = '\0';
		printf("Oppenent entered \"%s\"\n",word);
		return 1;
	}else{
		return 0;
	}

}




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
	if(recv(sd, &player, sizeof(char), MSG_WAITALL) == -1){
		perror("recv");
		exit(1);
	}

	uint8_t boardSize;
	if(recv(sd, &boardSize, sizeof(char), MSG_WAITALL) == -1){
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

	printf("You are Player %c...\n", player);
	printf("Board size: %d\n", boardSize);
	printf("Seconds per turn: %d\n", sec);

	return c;

}
