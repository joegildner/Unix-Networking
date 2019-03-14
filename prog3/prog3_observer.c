/* Authors: Joe Gildner, Quentin Jensen */

#include "prog3_observer.h"



/* main
 */
int main( int argc, char **argv) {

	initObserverStruct c = initObserver(argc, argv);
	observe(c.init_sd);
	close(c.init_sd);
	exit(EXIT_SUCCESS);

}



/*
 *
*/
void observe(int sd){

	uint16_t stringSize;
	char string[1024];
	int recvValue;

	while(1){
		
		for(int i=0; i<1024; i++){
			string[i] = '\0';
		}

		recv(sd, &stringSize, sizeof(uint16_t), MSG_WAITALL);
		recvValue = recv(sd, string, sizeof(char)*stringSize, MSG_WAITALL);

		//not sure if this is right
		if(recvValue<=0){
			printf("The server has disconnected you\n");
			exit(0);
		}

		string[stringSize] = '\0';
		
		printf("The following string is of size: %d\n", stringSize);
		printf("%s\n", string);
	}
}


































/* initialize client
 * handles the many procedures of initializing the client
 * so it can talk to a server
 */
initObserverStruct initObserver(int argc, char** argv){

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


	initObserverStruct c = {
		.init_sd = sd
	};

	return c;

}
