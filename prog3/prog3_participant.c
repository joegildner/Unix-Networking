/* Authors: Joe Gildner, Quentin Jensen */

#include "prog3_partipant.h"



/* main
 */
int main( int argc, char **argv) {

	initClientStruct c = initClient(argc, argv);
	mainGameLoop(c.init_sd, c.init_player, c.init_boardSize, c.init_sec);

	close(c.init_sd);
	exit(EXIT_SUCCESS);

}







/* initialize client
 * handles the many procedures of initializing the client
 * so it can talk to a server
 */
initParticipantStruct initParticipant(int argc, char** argv){

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


