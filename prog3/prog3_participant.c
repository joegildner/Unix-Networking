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
	bool usernameIsGoodLength = false;
	bool usernameIsValid = false;

	while(!usernameIsValid){

		while(!usernameIsGoodLength){
			char* username = promptUsername();
			//if length <= 10: (ONLY CHECK LENGTH, rest is server side)
				//usernameIsGoodLength = true;
				//send length of username
				//send username
			//else prompt again
		}

		//recv Y/T/I
		//if T or I
			//prompt again

		//the timer may run out on the server,
		//so if the last recv is -1, exit
	}



}



char* promptUsername(){

	return "flyinbrik";

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
