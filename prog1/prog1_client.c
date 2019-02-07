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

//==== actual logic starts here ====================

	playHangman(sd);
	close(sd);
	exit(EXIT_SUCCESS);
}



/*
 *
*/
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


/*
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

	//send guess
	if(send(sd, &guess, sizeof(guess),0)<0){
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

	//TODO: change so it's receiving uint8_t's

	uint16_t intBuf;

	if(recv(sd, &intBuf, sizeof(intBuf), MSG_WAITALL)<0) {
		perror("recv");
		exit(1);
	}
	int guesses = ntohs(intBuf);

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



void recvBoard(int sd, const int totalGuesses, int remainGuesses){
	char board[totalGuesses+1];
	for(int i=0; i<=totalGuesses; i++) board[i] = '\0';

	int n = recv(sd, board, (sizeof(char) * totalGuesses), MSG_WAITALL);

	printf("Board: %s (%d guesses left)\n", board, remainGuesses);
}





/* reference *
	n = recv(sd, buf, sizeof(buf), 0);
	while (n > 0) {
		write(1,buf,n);
		n = recv(sd, buf, sizeof(buf), 0);
	}





	uint8_t guess;
	//if(recv(sd, guess, sizeof(uint8_t), 0)<=0) {exit(1);}

	char buffer[1000];
	if(recv(sd,guess,sizeof(buffer),0)<=0){exit(1);}

	close(sd);



*/
