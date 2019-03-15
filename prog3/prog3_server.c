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

		// for(int i=0; i<MAX_CLIENTS; i++){
		// 	FD_SET(allParts->partSD, &rfds);
		// 	FD_SET(allPairs->obsSd, &rfds);
		// }

		if ( select(FD_SETSIZE, &rfds, NULL, NULL, NULL) == -1) {
			perror("select");
		 	exit(EXIT_FAILURE);
		}

		//if a particpant joined, set flag and add to the list of participants
		if(FD_ISSET(partsd, &rfds)){
			allParts[pIndex] = newPart();
			if ( (allParts[pIndex]->partSD=accept(partsd, (struct sockaddr *)&partcad, &partalen)) < 0) {
				fprintf(stderr, "Error: Accept failed\n");
				exit(EXIT_FAILURE);
			}
			ispart = true;
			pIndex++;
		}
		//if an observer joined, set flag and add to the list of observers
		else if(FD_ISSET(obssd, &rfds)){
			allObs[oIndex] = newPair();
			if ((allObs[oIndex]->obsSD = accept(obssd, (struct sockaddr *)&obscad, &obsalen)) < 0) {
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
				//close(partsd);
				//close(obssd);

				if(ispart)
					addParticipant(allParts[pIndex-1]);
				else
					//addObserver(allObs[oIndex-1]);
		 		break;
			}
			default: {
				//close(sd2);
		  	break;
			}
		}

	}

}



/* add participant
 * check if we've reached the max number of clients
 * otherwise negotiate a username for this client
 * and then tell all observers that $username has joined
*/
void addParticipant(part* thisPart){

	char Y = 'Y';
	char N = 'N';

	if(pIndex >= MAX_CLIENTS){
		if(send(thisPart->partSD, &N, sizeof(char),0)<0){perror("send");exit(1);}
		closeParticipant(thisPart);
		exit(0);
	}

	if(send(thisPart->partSD, &Y, sizeof(char),0)<0){perror("send");exit(1);}

	char usernameBuf[MAX_CLIENTS];

	for(int i=0; i<MAX_CLIENTS; i++){
		usernameBuf[i] = '\0';
	}
	char* username = negotiateUserName(thisPart->partSD, usernameBuf);
	strcpy(thisPart->name, username);
	
	char msg[strlen(username)+16];
	sprintf(msg, "User %s has joined", username);
	printf("%s\n", msg);
	sendAll(msg);

	//chat(thisPart);
}


// /* add observer
//  * check if we've reached the max number of clients
//  * otherwise negotiate a username for this client
//  * and then tell all observers that $username has joined
// */
// void addObserver(pair* thisPair){

// 	char Y = 'Y';
// 	char N = 'N';

// 	if(pIndex==MAX_CLIENTS){
// 		if(send(thisPair->obsSD, &N, sizeof(char),0)<0){perror("send");exit(1);}
// 		closeObserver(thisPair);
// 		exit(0);
// 	}

// 	if(send(thisPair->obsSD, &Y, sizeof(char),0)<0){perror("send");exit(1);}

// 	char usernameBuf[MAX_CLIENTS];
// 	char* username = negotiateUserName(thisPair->obsSD, usernameBuf);

// 	char result = canPairWithParticipant(thisPair->obsSD, username);

// 	if(result=='T'){
// 		//reset Timer( just call negotiate in a while loop?)
// 	}
// 	else if(result=='N'){
// 		close(thisPair->obsSD);
// 	}else{
// 		char msg[25] = "A new observer has joined";
// 		sendAll(msg);
// 		observe(thisPair->obsSD);
// 	}
// }


// /* can pair with participant?
//  * checks to see if an observer's username matches
//  * a participants username. if so, also checks that 
//  * that participant doesn't already have an observer.
//  * 
//  * RETURN: a letter representing what to do, based on the result
//  * of this.
//  *
//  * Y: can pair
//  * N: can't pair
//  * T: this participant already has an observer
//  */
// char canPairWithParticipant(int obsSD, char* username){

// 	char Y = 'Y';
// 	char N = 'N';
// 	char T = 'T';

// 	if(nameTaken(username)){

// 		for(int j=0; j<oIndex; j++){

// 			if(strcmp(username, allObs[j].name)){

// 				if(send(obsSD, &Y, sizeof(char),0)<0){perror("send");exit(1);}
// 			}
// 			else{
// 				if(send(obsSD, &T, sizeof(char),0)<0){perror("send");exit(1);}
// 			}
// 		}
// 	}
// 	else{
// 		if(send(obsSD, &N, sizeof(char),0)<0){perror("send");exit(1);}
// 	}


// 	//if username doesn't exist, send 'N' and close(sd)
// }

// void chat(part* thisPart){

// 	uint16_t msgSize;
// 	char msg[MAX_MSG_SIZE];

// 	while(1){
// 		//ntohs here?
// 		recv(sd, &msgSize, sizeof(uint8_t), MSG_WAITALL);
// 			if(msgSize>MAX_MSG_SIZE){ close(sd); exit(0);}//exit here?
// 		recv(sd, msg, sizeof(char)*msgSize, MSG_WAITALL);
// 			msg[msgSize] = '\0';

// 		if(msg[0]=='@')
// 			sendPrivateMsg(sd, msg);
// 		else
// 			sendPublicMsg(sd, msg, username);

// 	}	
// }


// /* send private message
//  * checks if the recipient exists, then formats the message to send.
//  * sends that message to both the sender's and recipient's observer
//  *
//  * if the recipient doesn't exist, send an error message to the sender
//  */
// void sendPrivateMsg(int sd, char* msg){
	
// 	char* recipient = parseRecipient(msg);

	
// 	if(recipientIsValid(recipient)){

// 		char outMsg[strlen(msg)+14];
		
// 		outMsg[0] = '>';
// 		for(int i=1; i<(12-strlen(recipient)); i++){
// 			outMsg[i] = ' ';
// 		}
// 		int j=0;
// 		for(int i=12-strlen(recipient); i<12; i++){
// 			outMsg[i] = msg[j++];
// 		}
// 		outMsg[12] = ':';
// 		outMsg[13] = ' ';
	
// 		uint16_t msgSize = htons(strlen(outMsg));

// 		// send to the sender's observer
// 		int senderOSD = getObserver(sd);	
// 		if(send(senderOSD, &msgSize, sizeof(uint16_t),0)<0){perror("send");exit(1);}
// 		if(send(senderOSD, &outMsg, sizeof(char)*msgSize,0)<0){perror("send");exit(1);}	
	
// 		//send to recipient's observer
// 		int recipientOSD = getObserver(getParticipantByName(recipient));
// 		if(send(recipientOSD, &msgSize, sizeof(uint16_t),0)<0){perror("send");exit(1);}
// 		if(send(recipientOSD, &outMsg, sizeof(char)*msgSize,0)<0){perror("send");exit(1);}

// 	}
// 	else{
	
// 		char outMsg[strlen(recipient)+33];  
// 		sprintf(outMsg, "Warning: user %s doesn't exist...", recipient);

// 		uint16_t msgSize = htons(strlen(outMsg));

// 		// send to the sender's observer
// 		int senderOSD = getObserver(sd);	
// 		if(send(senderOSD, &msgSize, sizeof(uint16_t),0)<0){perror("send");exit(1);}
// 		if(send(senderOSD, &outMsg, sizeof(char)*msgSize,0)<0){perror("send");exit(1);}	
	

// 	}
// }

// /* get participant by name
//  * finds a participant in the list of participant-observer pairs
//  */
// int getParticipantByName(char* name){
// 	int retVal = 0;

// 	for(int i=0; i<oIndex; i++){
// 		if(!strcmp(allObs[i].name, name)){
// 			retVal = allObs[i].partSD;
// 		}
// 	}

// 	return retVal;
// }

// /* get observer
//  * finds the observer sd associated with a participant sd
//  * returns observer sd, or 0 if not found
//  */
// int getObserver(int sd){
// 	int retVal = 0;	

// 	for(int i=0; i<oIndex; i++){
// 		if(allObs[i].partSD == sd){
// 			retVal = allObs[i].obsSD;
// 		}
// 	}
// 	return retVal;
// }

char* parseRecipient(char* msg){
	//DEBUG:
	return "dad";
}

/* recipient is valid?
 * checks that a recipient exists in allNames
 * (possibly we need to check that the participant has an observer attached?)
 */ 
bool recipientIsValid(char* recipient){
	return true;//DEBUG
}

void sendPublicMsg(int sd, char* msg, char* username){
	char outMsg[strlen(msg)+14];
	
	outMsg[0] = '>';
	for(int i=1; i<(12-strlen(username)); i++){
		outMsg[i] = ' ';
	}
	int j=0;
	for(int i=12-strlen(username); i<12; i++){
		outMsg[i] = msg[j++];
	}
	outMsg[12] = ':';
	outMsg[13] = ' ';

	sendAll(outMsg);

}

void observe(int sd){
	
}



/* send all
 * sends all observers a generic string
*/
void sendAll(char* msg){
	uint16_t msgSize = htons(strlen(msg));

	//DEBUG: not sure if this needs to be oIndex+1 or not
	for(int i=0; i<oIndex; i++){
		if(send(allObs[i]->obsSD, &msgSize, sizeof(uint16_t),0)<0){perror("send");exit(1);}
		if(send(allObs[i]->obsSD, &msg, sizeof(char)*msgSize,0)<0){perror("send");exit(1);}
	}
}



/* negotiateUserName
 * on a timer, waits for participant to send a username.
 * sends a 'Y' if it's all good.
 * sends a 'I' if the username is just plain invalid
 * sends a 'T' if the name has been taken, and resets the timer
*/
char* negotiateUserName(int sd, char* usernameBuf){
	//if timer runs out, close sd

	struct timeval tv;
		tv.tv_sec = 60;
		tv.tv_usec = 0;
	char Y = 'Y';
	char I = 'I';
	char T = 'T';
	uint8_t usernameSize;

	bool validName = false;

	while(!validName){
		//set the timer, receive username size and username
		setsockopt(sd, SOL_SOCKET,SO_RCVTIMEO, &tv, sizeof(struct timeval));
		recv(sd, &usernameSize, sizeof(uint8_t), MSG_WAITALL);
		int recvValue = recv(sd, usernameBuf, sizeof(char)*usernameSize, MSG_WAITALL);

		usernameBuf[usernameSize] = '\0';

		//if timer ran out
		if( recvValue == -1 && errno == EAGAIN ) {
			close(sd);
			printf("%s\n", "recv returned due to timeout!");
			exit(1);
		}
		if(!nameTaken(usernameBuf)){
			if(validateName(usernameBuf)){
				if(send(sd, &Y, sizeof(char),0)<0){perror("send");exit(1);}
				validName = true;
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
	

	//DEBUG: just send Y's for now until stupid string manipulation gets resolved
	//if(send(sd, &Y, sizeof(char),0)<0){perror("send");exit(1);}
	return usernameBuf;
}


/* name taken?
 * iterates through all usernames known to the server, checking if
 * a username matches any. If not, add to the list.
*/
bool nameTaken(char* username){
	bool isTaken = false;
	for(int i=0; i<pIndex; i++){
		if(allParts[i]->name != NULL){
			printf("%s\n",allParts[i]->name);
			if(strcmp(username, allParts[i]->name)==0){
				isTaken = true;
			}
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




/* close socket
 * Iterates through both the observer and participant socket arrays
 * until the desired socket descriptor is found, then removes this
 * descriptor, decrements the descriptor count, and shifts everything
 * else in the array to the left.
*/
void closeParticipant(part* thisPart){
	int i;

	for(i=0; i<MAX_CLIENTS; i++){
		if(allParts[i] == thisPart){
			pIndex--;
			break;
		}
	}
	for(int j = i; j < MAX_CLIENTS-1; j++){
		allParts[j] = allParts[j+1];
	}

	

}

void closeObserver(pair* thisPair){
	int i; 
	for(i=0; i<MAX_CLIENTS; i++){
		if(allObs[i] == thisPair){

			if(thisPair->partner != NULL) thisPair->partner->hasPartner = false;
			
			oIndex--;
			break;
		}
	}
	for(int j = i; j < MAX_CLIENTS; j++){
		allObs[j] = allObs[j+1];
	}

}

pair* newPair(){
	pair* thisPair = malloc(sizeof(pair));

	if(thisPair == NULL){
		perror("Malloc");
		exit(1);
	}

	return thisPair;
}

part* newPart(){
	part* thisPart = malloc(sizeof(part));

	if(thisPart == NULL){
		perror("Malloc");
		exit(1);
	}

	for(int i=0; i<11; i++){
		thisPart->name[i] = '\0';
	}

	thisPart->hasPartner = false;

	return thisPart;
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
