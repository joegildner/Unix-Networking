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
#include <sys/time.h>
#include <errno.h>

typedef struct initServerStruct {
	struct sockaddr_in init_cad;
	int init_sd;
	uint8_t init_boardSize;
	uint8_t init_sec;
} initServerStruct;

void startGameSession(int p1, int p2, uint8_t boardSize, uint8_t sec);
initServerStruct initServer(int, char**);
void mainServerLoop(struct sockaddr_in, int, uint8_t, uint8_t);
char* generateBoard(uint8_t boardSize);
char* validateWord(char word[], char board[], char** usedWords);
void takeTurns(int p1, int p2, uint8_t* score1, uint8_t* score2, 
              char* board, uint8_t boardSize, uint8_t sec, uint8_t round);


#define QLEN 6



/* main
 */
int main(int argc, char **argv) {

	initServerStruct c = initServer(argc, argv);

	mainServerLoop(c.init_cad, c.init_sd, c.init_boardSize, c.init_sec);

	exit(EXIT_SUCCESS);
}



/* main server loop
 * accepts 2 incoming connections at a time, then forks. As soon as a client connects, 
 * send them their player number, the size of the board, and the seconds of each
 * round. The child then plays the game with those 2 clients. The parent returns 
 * to accepting more connections
 */
void mainServerLoop(struct sockaddr_in cad, int sd, uint8_t boardSize, uint8_t sec){

	int sd2;
	int sd3;
	char c1 = '1';
	char c2 = '2';

	while (1) {


		socklen_t alen = sizeof(cad);
		if ( (sd2=accept(sd, (struct sockaddr *)&cad, &alen)) < 0) {
			fprintf(stderr, "Error: Accept failed\n");
			exit(EXIT_FAILURE);
		}
		if(send(sd2, &c1, sizeof(char),0)<=0){exit(1);}
		if(send(sd2, &boardSize, sizeof(uint8_t),0)<=0){exit(1);}
		if(send(sd2, &sec, sizeof(uint8_t),0)<=0){exit(1);}
		printf("Client 1 connected, waiting for Client 2...\n");


		if ( (sd3=accept(sd, (struct sockaddr *)&cad, &alen)) < 0) {
			fprintf(stderr, "Error: Accept failed\n");
			exit(EXIT_FAILURE);
		}	
		if(send(sd3, &c2, sizeof(char),0)<=0){exit(1);}
		if(send(sd3, &boardSize, sizeof(uint8_t),0)<=0){exit(1);}
		if(send(sd3, &sec, sizeof(uint8_t),0)<=0){exit(1);}
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
				startGameSession(sd2, sd3, boardSize, sec);
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



/* start game session
 * each round consists of sending both player's both player's scores,
 * and the round number. Then generate a new board, and send it to them.
 *
 */
void startGameSession(int p1, int p2, uint8_t boardSize, uint8_t sec){

	uint8_t round = 0;
	uint8_t score1 = 0;
	uint8_t score2 = 0;

	char* board;
	
	//begin new round
	while(score1<3 || score2<3){
		round++;	
	
		if(send(p1, &score1, sizeof(uint8_t),0)<=0){exit(1);}
		if(send(p1, &score2, sizeof(uint8_t),0)<=0){exit(1);}

		if(send(p2, &score1, sizeof(uint8_t),0)<=0){exit(1);}
		if(send(p2, &score2, sizeof(uint8_t),0)<=0){exit(1);}

		if(send(p1, &round, sizeof(uint8_t),0)<=0){exit(1);}
		if(send(p2, &round, sizeof(uint8_t),0)<=0){exit(1);}

		board = generateBoard(boardSize);//no null byte

		if(send(p1, board, sizeof(char)*boardSize,0)<=0){exit(1);}
		if(send(p2, board, sizeof(char)*boardSize,0)<=0){exit(1);}

		takeTurns(p1, p2, &score1, &score2, board, boardSize, sec, round);
	}

	// Send scores to notify client someone won.
	if(send(p1, &score1, sizeof(uint8_t),0)<=0){exit(1);}
	if(send(p1, &score2, sizeof(uint8_t),0)<=0){exit(1);}
	if(send(p2, &score1, sizeof(uint8_t),0)<=0){exit(1);}
	if(send(p2, &score2, sizeof(uint8_t),0)<=0){exit(1);}

}



/* take turns
 * each player takes turns guessing a word, on a timelimit set by
 * setsockopt. server then validates the word. This session of turn ends
 * as soon as a player times out, or submits an invalid word.
 *
 * NOTE: modifies the scores passed in
 */
void takeTurns(int p1, int p2, uint8_t* score1, uint8_t* score2, 
              char* board, uint8_t boardSize, uint8_t sec, uint8_t round){

	char** usedWords;
	int activePlayer = 0;
	int inactivePlayer = 0;
	int recvValue = 0;
	bool running = true;
	uint8_t one = 1;
	uint8_t zero = 0;
	char y = 'Y';
	char n = 'N';
	uint8_t wordSize = 0;
	char word[255];

	struct timeval tv;
	tv.tv_sec = sec; 
	tv.tv_usec = 0;
	//set active and inactive player
	if(round%2==1){
		activePlayer=p1;
		inactivePlayer=p2;
	}
	else{
		activePlayer=p2;
		inactivePlayer=p1;
	}

	while(running){
		
		if(send(activePlayer, &y, sizeof(char),0)<=0){exit(1);}
		if(send(inactivePlayer, &n, sizeof(char),0)<=0){exit(1);}

	  	//set timer
		setsockopt(activePlayer, SOL_SOCKET,SO_RCVTIMEO, &tv, sizeof(struct timeval));

		//wait sec seconds for player to respond
		recv(activePlayer, &wordSize, sizeof(uint8_t), MSG_WAITALL);
		recvValue = recv(activePlayer, &word, sizeof(char)*wordSize, MSG_WAITALL);
		
		if( recvValue == -1 && errno == EAGAIN ) {
			running = false;
			printf("%s\n", "recv returned due to timeout!");
		}
	

		//validate
		char* validatedWord = validateWord(&word[0], board, usedWords);
		if(strcmp(validatedWord, "") && running){
			
			//TODO: add validatedWord to usedWords

			if(send(activePlayer, &one, sizeof(uint8_t),0)<=0){exit(1);}
			if(send(inactivePlayer, &boardSize, sizeof(uint8_t),0)<=0){exit(1);}
			if(send(inactivePlayer, board, sizeof(char)*boardSize,0)<=0){exit(1);}

			if(activePlayer==p1){ score1++; }
			else{ score2++; } 
		}
		else{
			running = false;
		}
		

		//switch players
		int temp;
		temp = activePlayer;
		activePlayer = inactivePlayer;
		inactivePlayer = temp;

	}

	//turn is over
	if(send(activePlayer, &zero, sizeof(uint8_t),0)<=0){exit(1);}
	if(send(inactivePlayer, &zero, sizeof(uint8_t),0)<=0){exit(1);}

	if(inactivePlayer==p1){ score1++; }
	else{ score2++; } 

}



/* validate word
 * checks if the word is in the dictionary, and hasn't already
 * been used this round. Then checks that every letter in word
 * corresponds to a letter in board
 */
char* validateWord(char* word, char board[], char** usedWords){
	bool isValid = false;

	//twl06.txt
	//do something with trie.c

	if(isValid){return word;}
	return "";
}


char* generateBoard(uint8_t boardSize){
	struct timeval nanotime;
	gettimeofday(&nanotime, NULL);
	char* board = malloc(sizeof(char)*boardSize);
	srand(nanotime.tv_usec);

	bool containsVowel = false;
	char vowels[5] = {'a','e','i','o','u'};

	for(int i=0; i<boardSize; i++){
		char randChar = 'a' + (rand()%26);
		for(int j=0; j<5; j++){
			if(randChar == vowels[j]) containsVowel = true;
		}
		board[i] = randChar;
	}

	while(!containsVowel){
		char randChar = 'a' + (rand()%26);
		for(int i=0; i<5; i++){
			if(randChar == vowels[i]) containsVowel = true;
		}
		board[boardSize-1] = randChar;
	}

	return board;
}





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
