/* Authors: Joe Gildner, Quentin Jensen */

#include "prog3_participant.h"



/* main
 */
int main( int argc, char **argv) {

	initParticipantStruct c = initParticipant(argc, argv);
	chat(c.init_sd);
	close(c.init_sd);
	exit(EXIT_SUCCESS);
}



/* chat
 *
*/
void chat(int sd){
	//recv Y/N
	//if yes, continue, if no, abort
	negotiateUserName(sd);
}


void negotiateUserName(int sd){
	char input[1024];
	char username[255];
	int usernameSize = 255;
	char result;
	bool isValid = false;

	for(int i=0; i<1024; i++){
		input[i] = '\0';
	}

	while(!isValid){
		while(usernameSize > 10){

			scanf("%s",input);

			for(int i=0; i<255; i++){
				username[i] = input[i];
			}

			usernameSize = strlen(username);

		}

		if(send(sd, &usernameSize, sizeof(uint8_t),0)<0){perror("send");exit(1);}
		if(send(sd, &username, sizeof(char)*usernameSize,0)<0){perror("send");exit(1);}

		recv(sd, &result, sizeof(char), MSG_WAITALL);

		if(result=='Y'){
			isValid = true;
		}

		//the timer may run out on the server,
		//so if the last recv is -1, exit
	}
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


	initParticipantStruct c = {
		.init_sd = sd
	};


	return c;

}
