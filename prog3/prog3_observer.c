/* Authors: Joe Gildner, Quentin Jensen */

#include "prog3_observer.h"



/* main
 */
int main( int argc, char **argv) {

	initObserverStruct c = initObserver(argc, argv);

	setup(c.init_sd);
	observeMsg(c.init_sd);

	close(c.init_sd);
	exit(EXIT_SUCCESS);

}


/* setup
 * sets up an account with the server
 */
void setup(int sd){
	char result;
	printf("%s\n", "Waiting for the server");
	recv(sd, &result, sizeof(char), MSG_WAITALL);

	if(result=='N'){
		printf("The server is full, try again later\n");
		exit(0);
	} else if(result == 'Y'){
		printf("Negotiating...\n");
		negotiateUserName(sd);
	}
}


void negotiateUserName(int sd){
	char input[1024];
	char username[255];
	uint8_t usernameSize;
	char result;
	bool isValid = false;


	while(!isValid){
		usernameSize = 255;

		for(int i=0; i<1024; i++){
			input[i] = '\0';
		}

		while(usernameSize > 10){

			printf("type a username: ");
			fgets(input, MAX_MSG_SIZE, stdin);

			if ((strlen(input) > 0) && (input[strlen (input) - 1] == '\n'))
				input[strlen (input) - 1] = '\0';

			for(int i=0; i<255; i++){
				username[i] = input[i];
			}

			usernameSize = strlen(username);
			if(usernameSize>10){printf("Username too long, try again\n");}
		}

		if(send(sd, &usernameSize, sizeof(uint8_t),0)<0){perror("send");exit(1);}
		if(send(sd, &username, sizeof(char)*usernameSize,0)<0){perror("send");exit(1);}

		int recvValue = recv(sd, &result, sizeof(char), MSG_WAITALL);

		//not sure if this is right
		if(recvValue<=0){
			printf("60 seconds is up, server has disconnected you\n");
			exit(0);
		}
		if(result=='Y'){
			isValid = true;
		}
		else if(result == 'I'){
			printf("Invalid username, please try again\n");
		}
		else if(result == 'T'){
			printf("User already has an observer, please try again.\n");
		}
	}
}


/* observe msg
 * only use when expecting a msg from the server
 * receives a size and a msg.
*/
void observeMsg(int sd){

	uint16_t msgSize;
	char msg[1024];
	int recvValue;

	while(1){

		for(int i=0; i<1024; i++){
			msg[i] = '\0';
		}

		recv(sd, &msgSize, sizeof(uint16_t), MSG_WAITALL);
			msgSize = ntohs(msgSize);
		recvValue = recv(sd, msg, sizeof(char)*msgSize, MSG_WAITALL);

		if(recvValue<=0){
			printf("The server has disconnected you\n");
			exit(0);
		}

		msg[msgSize] = '\0';
		
		printf("%s\n", msg);
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
