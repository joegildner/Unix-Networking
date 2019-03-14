/* Authors: Joe Gilder, Quentin Jensen */

#include "prog3_server.h"


/* main
 *
 */
int main( int argc, char **argv) {

	initServerStruct c = initServer(argc, argv);

	mainAcceptLoop(c.init_partcad, c.init_obscad, c.init_partsd, c.init_obssd);

	close(c.init_partsd);
	close(c.init_obssd);
	exit(EXIT_SUCCESS);

}

/* main accept loop
 * sets up connection parameters for any and all incoming connections.
 * either a participant or observer can join, which are handled differently.
 *
 * forks, child takes care of client while parent goes back to listening for
 * new connections
*/
void mainAcceptLoop(struct sockaddr_in partcad, struct sockaddr_in obscad,
										int partsd, int obssd){

	bool ispart;
	fd_set rfds;
	socklen_t partalen = sizeof(partcad);
	socklen_t obsalen = sizeof(obscad);

	while (1) {

		ispart = false;

		FD_ZERO(&rfds);
		FD_SET(partsd, &rfds);
		FD_SET(obssd, &rfds);

		if ( select(FD_SETSIZE, &rfds, NULL, NULL, NULL) == -1) {
			fprintf(stderr, "Error: select failed\n");
		 	exit(EXIT_FAILURE);
		}



		//if a particpant joined, set flag and add to the list of participants
		if(FD_ISSET(partsd, &rfds)){
			if ( (allParts[pIndex]=accept(partsd, (struct sockaddr *)&partcad, &partalen)) < 0) {
				fprintf(stderr, "Error: Accept failed\n");
				exit(EXIT_FAILURE);
			}
			//DEBUG: printf("part\n");
			ispart = true;
			pIndex++;
		}
		//if an observer joined, set flag and add to the list of observers
		else if(FD_ISSET(obssd, &rfds)){
			if ( (allObs[oIndex]=accept(obssd, (struct sockaddr *)&obscad, &obsalen)) < 0) {
				fprintf(stderr, "Error: Accept failed\n");
				exit(EXIT_FAILURE);
			}
			//DEBUG: printf("obs\n");
			oIndex++;
		}



		const pid_t cpid = fork();
		switch(cpid) {

			//error
			case -1: {perror("fork");break;}

			case 0: {
				close(partsd);
				close(obssd);

				if(ispart)
					addToChat(allParts[pIndex-1]);
				else
					observeChat(allObs[oIndex-1]);

		  	exit(0);
		 		break;
			}
			default: {
				//close(sd2);
		  	break;
			}
		}

	}

}


/* add to chat
 * for participants only!
 * check if we've reached the max number of clients
 * otherwise negotiate a username for this client
 * and then tell all observers that $username has joined
*/
void addToChat(int sd){

	char Y = 'Y';
	char N = 'N';

	if(pIndex==MAX_CLIENTS){
		if(send(sd, &N, sizeof(char),0)<0){perror("send");exit(1);}
		closeSocket(sd);
		exit(0);
	}

	if(send(sd, &Y, sizeof(char),0)<0){perror("send");exit(1);}

	char* username = negotiateUserName(sd);
	sendAll(username);

}


/*
 *
*/
void sendAll(char* username){
	//run through observer list and send
	printf("some guy just joined. point and laugh at: %s\n",username);
}



/* negotiateUserName
 * on a timer, waits for participant to send a username.
 * sends a 'Y' if it's all good.
 * sends a 'I' if the username is just plain invalid
 * sends a 'T' if the name has been taken, and resets the timer
*/
char* negotiateUserName(int sd){
	//if timer runs out, close sd

	struct timeval tv;
		tv.tv_sec = 60;
		tv.tv_usec = 0;
	char Y = 'Y';
	char I = 'I';
	char T = 'T';
	uint8_t usernameSize;
	char username[255];

	//set the timer, receive username size and username
	setsockopt(sd, SOL_SOCKET,SO_RCVTIMEO, &tv, sizeof(struct timeval));
	recv(sd, &usernameSize, sizeof(uint8_t), MSG_WAITALL);
	int recvValue = recv(sd, &username, sizeof(char)*usernameSize, MSG_WAITALL);

	//if timer ran out
	if( recvValue == -1 && errno == EAGAIN ) {
		//running = false;
		printf("%s\n", "recv returned due to timeout!");
	}

	username[usernameSize] = '\0';

	if(!nameTaken(&username[0])){
		if(validateName(&username[0])){
			if(send(sd, &Y, sizeof(char),0)<0){perror("send");exit(1);}
		}
		else{
			if(send(sd, &I, sizeof(char),0)<0){perror("send");exit(1);}
		}
	}
	else{
		if(send(sd, &T, sizeof(char),0)<0){perror("send");exit(1);}
		tv.tv_sec = 60;
	}
}


/* name taken?
 * iterates through all usernames known to the server, checking if
 * a username matches any
*/
bool nameTaken(char* username){
	bool isTaken = false;

	for(int i=0; i<MAX_CLIENTS; i++){
		if(strcmp(username, allNames[i])==0){
			isTaken = true;
		}
	}

	return isTaken;
}


/* validate name
 * validates according to these rules:
 * The username must consist only of upper-case letters, lower-case letters,
 * numbers and underscores.  No other symbols are allowed, nor is whitespace.
*/
bool validateName(char* username){
	bool isValid = true;

	//if any character falls outside of these ascii ranges, invalid
	for(int i=0; i<strlen(username); i++){
		if(!((username[i] >= 48 && username[i] <= 57) || //number
				(username[i] >= 65 && username[i] <= 90)  || //capital
				(username[i] >= 97 && username[i] <= 122) || //lower case
				(username[i] == 95))) //underscore
		{ isValid = false; }
	}
	return isValid;
}



void observeChat(int sd){
	printf("eat my ass\n");
}



/* close socket
 * Iterates through both the observer and participant socket arrays
 * until the desired socket descriptor is found, then removes this
 * descriptor, decrements the descriptor count, and shifts everything
 * else in the array to the left.
*/
void closeSocket(int sd){
	int i;

	for(i=0; i<MAX_CLIENTS; i++){
		if(allParts[i] == sd){
			pIndex--;
			break;
		}
	}
	for(int j = i; j < MAX_CLIENTS; j++){
		allParts[j] = allParts[j+1];
	}

	for(i=0; i<MAX_CLIENTS; i++){
		if(allObs[i] == sd){
			oIndex--;
			break;
		}
	}
	for(int j = i; j < MAX_CLIENTS; j++){
		allObs[j] = allObs[j+1];
	}

}






/* initialize server
 * handles the many procedures of initializing our server, for both
 * participant and observer in-sockets.
 * RETURN: struct containing sockaddr_in's and socket descriptors for them.
 */
initServerStruct initServer(int argc, char** argv){

	if( argc != 3 ) {
		fprintf(stderr,"Error: Wrong number of arguments\n");
		fprintf(stderr,"usage:\n");
		fprintf(stderr,"./server, participant port, observer port \n");
		exit(EXIT_FAILURE);
	}

	struct protoent *ptrp;
	int optval = 1;

	struct sockaddr_in partSad;
	struct sockaddr_in partCad;
	int partsd = 0;

	struct sockaddr_in obsSad;
	struct sockaddr_in obsCad;
	int obssd = 0;

	uint16_t partPort = atoi(argv[1]);
	uint16_t obsPort = atoi(argv[2]);

	//list of all observers and participants

	memset((char *)&partSad,0,sizeof(partSad));
	partSad.sin_family = AF_INET;
	partSad.sin_addr.s_addr = INADDR_ANY;

	memset((char *)&obsSad,0,sizeof(obsSad));
	obsSad.sin_family = AF_INET;
	obsSad.sin_addr.s_addr = INADDR_ANY;


	if (partPort > 0) {
		partSad.sin_port = htons(partPort);
	} else {
		fprintf(stderr,"Error: Bad port number %s\n",argv[1]);
		exit(EXIT_FAILURE);
	}
	if (obsPort > 0) {
		obsSad.sin_port = htons(obsPort);
	} else {
		fprintf(stderr,"Error: Bad port number %s\n",argv[1]);
		exit(EXIT_FAILURE);
	}


	if ( ((long int)(ptrp = getprotobyname("tcp"))) == 0) {
		fprintf(stderr, "Error: Cannot map \"tcp\" to protocol number");
		exit(EXIT_FAILURE);
	}


	partsd = socket(AF_INET, SOCK_STREAM, ptrp->p_proto);
	if (partsd < 0) {
		fprintf(stderr, "Error: Socket creation failed\n");
		exit(EXIT_FAILURE);
	}
	obssd = socket(AF_INET, SOCK_STREAM, ptrp->p_proto);
	if (obssd < 0) {
		fprintf(stderr, "Error: Socket creation failed\n");
		exit(EXIT_FAILURE);
	}


	if( setsockopt(partsd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) < 0 ) {
		fprintf(stderr, "Error Setting socket option failed\n");
		exit(EXIT_FAILURE);
	}
	if( setsockopt(obssd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) < 0 ) {
		fprintf(stderr, "Error Setting socket option failed\n");
		exit(EXIT_FAILURE);
	}


	if (bind(partsd, (struct sockaddr *)&partSad, sizeof(partSad)) < 0) {
		fprintf(stderr,"Error: Bind failed\n");
		exit(EXIT_FAILURE);
	}
	if (bind(obssd, (struct sockaddr *)&obsSad, sizeof(obsSad)) < 0) {
		fprintf(stderr,"Error: Bind failed\n");
		exit(EXIT_FAILURE);
	}


	if (listen(partsd, QLEN) < 0) {
		fprintf(stderr,"Error: Listen failed\n");
		exit(EXIT_FAILURE);
	}
	if (listen(obssd, QLEN) < 0) {
		fprintf(stderr,"Error: Listen failed\n");
		exit(EXIT_FAILURE);
	}

	initServerStruct c =	{
		.init_partcad = partCad,
		.init_obscad = obsCad,
		.init_partsd = partsd,
		.init_obssd = obssd,
	};

	signal(SIGCHLD,SIG_IGN);

	return c;
}
