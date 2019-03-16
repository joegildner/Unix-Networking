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
*/
void mainAcceptLoop(struct sockaddr_in partcad, struct sockaddr_in obscad,
										int partsd, int obssd){

	bool ispart;
	fd_set rfds;
	socklen_t partalen = sizeof(partcad);
	socklen_t obsalen = sizeof(obscad);

	while (1) {

		//checkTimers();

		ispart = false;

		FD_ZERO(&rfds);
		FD_SET(partsd, &rfds);
		FD_SET(obssd, &rfds);

		for(int i=0; i<pIndex; i++){
			FD_SET(allParts[i]->partSD, &rfds);
		}
		for(int i=0; i<oIndex; i++){
			FD_SET(allObs[i]->obsSD, &rfds);
		}

		if ( select(FD_SETSIZE, &rfds, NULL, NULL, NULL) == -1) {
			perror("select");
		 	exit(EXIT_FAILURE);
		}

		//if a particpant joined, add to the list of participants
		if(FD_ISSET(partsd, &rfds)){
			allParts[pIndex] = newPart();
			if ( (allParts[pIndex]->partSD=accept(partsd, (struct sockaddr *)&partcad, &partalen)) < 0) {
				fprintf(stderr, "Error: Accept failed\n");
				exit(EXIT_FAILURE);
			}
			addParticipant(allParts[pIndex]);
			pIndex++;
		}
		//if an observer joined, and add to the list of observers
		if(FD_ISSET(obssd, &rfds)){
			allObs[oIndex] = newPair();
			if ((allObs[oIndex]->obsSD = accept(obssd, (struct sockaddr *)&obscad, &obsalen)) < 0) {
				fprintf(stderr, "Error: Accept failed\n");
				exit(EXIT_FAILURE);
			}
			addObserver(allObs[oIndex]);
			oIndex++;
		}

		for(int i=0; i<pIndex; i++){
			if(FD_ISSET(allParts[i]->partSD, &rfds)){
				//printf("%s%d%s\n","Participant ",i," set");
				participantAction(allParts[i]);
			}
		}
		for(int i=0; i<oIndex; i++){
			if(FD_ISSET(allObs[i]->obsSD, &rfds)){
				//printf("%s%d%s\n","Observer ",i," set");
				observerAction(allObs[i]);
			}
		}

	}

}

void checkTimers(){
	for(int i=0; i<pIndex; i++){
		long now = clock();
		if(allParts[i]->startTime > -1){
			time_t secs = allParts[i]->startTime;
			if((now - (allParts[i]->startTime))/CLOCKS_PER_SEC >= CLIENT_TIMEOUT){
				closeParticipant(allParts[i]);
			}
		}
	}
	for(int i=0; i<oIndex; i++){
		long now = clock();
		if(allObs[i]->startTime > -1){
			if((now - (allObs[i]->startTime))/CLOCKS_PER_SEC >= CLIENT_TIMEOUT){
				closeObserver(allObs[i]);
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
	}else if(send(thisPart->partSD, &Y, sizeof(char),0)<0){perror("send");exit(1);}
}


/* add observer
 * check if we've reached the max number of clients
 * otherwise negotiate a username for this client
 * and then tell all observers that $username has joined
*/
void addObserver(pair* thisPair){

	char Y = 'Y';
	char N = 'N';

	if(oIndex >= MAX_CLIENTS){
		if(send(thisPair->obsSD, &N, sizeof(char),0)<0){perror("send");exit(1);}
		closeObserver(thisPair);
		exit(0);
	}else if(send(thisPair->obsSD, &Y, sizeof(char),0)<0){perror("send");exit(1);}
}


void participantAction(part* p){
	//No username yet
	if(p->name[0] == '\0'){
		participantUsername(p);
	}else{
		chat(p);
	}
}

void observerAction(pair* o){
	if(o->partner == NULL){
		observerUsername(o);
	}else{
		closeObserver(o);
	}
}

void participantUsername(part* p){
	char usernameBuf[MAX_CLIENTS];
	char Y = 'Y';
	char I = 'I';
	char T = 'T';

	for(int i=0; i<MAX_CLIENTS; i++){
		usernameBuf[i] = '\0';
	}

	if(validateName(p->partSD, usernameBuf)){
		if(!nameTaken(usernameBuf)){
			strcpy(p->name, usernameBuf);
			if(send(p->partSD, &Y, sizeof(char),0)<0){perror("send");exit(1);}
			userJoined(p);
		}else{
			if(send(p->partSD, &T, sizeof(char),0)<0){perror("send");exit(1);}
			p->startTime = clock();
		}
	}else{
		if(send(p->partSD, &I, sizeof(char),0)<0){perror("send");exit(1);}
	}
}

void observerUsername(pair* o){
	char usernameBuf[MAX_CLIENTS];
	char Y = 'Y';
	char I = 'I';
	char T = 'T';

	for(int i=0; i<MAX_CLIENTS; i++){
		usernameBuf[i] = '\0';
	}

	if(validateName(o->obsSD, usernameBuf)){
		part* p = getParticipantByName(usernameBuf);

		if(p == NULL){
			if(send(o->obsSD, &I, sizeof(char),0)<0){perror("send");exit(1);}
		}
		else if(!p->hasPartner){
			o->partner = p;
			p->hasPartner = true;
			p->obsSD = o->obsSD;
			if(send(o->obsSD, &Y, sizeof(char),0)<0){perror("send");exit(1);}
			observerJoined(o);

		}
		else{
			if(send(o->obsSD, &T, sizeof(char),0)<0){perror("send");exit(1);}
			o->startTime = clock();
		}
	}else{
		if(send(o->obsSD, &I, sizeof(char),0)<0){perror("send");exit(1);}
	}
}

/* validate name
 * validates according to these rules:
 * The username must consist only of upper-case letters, lower-case letters,
 * numbers and underscores.  No other symbols are allowed, nor is whitespace.
*/
bool validateName(int sd, char* username){
	uint8_t usernameSize;

	recv(sd, &usernameSize, sizeof(uint8_t), MSG_WAITALL);
	int recvValue = recv(sd, username, sizeof(char)*usernameSize, MSG_WAITALL);
	username[usernameSize] = '\0';

	bool isValid = true;

	//if any character falls outside of these ascii ranges, invalid
	for(int i=0; i<strlen(username); i++){
		if(!((username[i] >= 48 && username[i] <= 57) || //number
				(username[i] >= 65 && username[i] <= 90)  || //capital
				(username[i] >= 97 && username[i] <= 122) || //lower case
				(username[i] == 95))) //underscore
		{ isValid = false; }
	}

	if(usernameSize > 10) isValid = false;

	return isValid;
}

/* name taken?
 * iterates through all usernames known to the server, checking if
 * a username matches any. If not, add to the list.
*/
bool nameTaken(char* username){
	bool isTaken = false;

	for(int i=0; i<pIndex; i++){
		if(allParts[i]->name != NULL){
			if(strcmp(username, allParts[i]->name)==0){
				isTaken = true;
			}
		}
	}
	return isTaken;
}

void userJoined(part* p){
	char* username = p->name;
	char msg[strlen(username)+16];
	sprintf(msg, "User %s has joined", username);
	//printf("%s\n", msg);
	sendAll(msg);
}

void observerJoined(pair* o){
	char msg[26] = "A new observer has joined";
	//printf("%s\n",msg);
	sendAll(msg);
}



void chat(part* thisPart){

	uint16_t msgSize;
	char msg[MAX_MSG_SIZE];

	int recvValue = recv(thisPart->partSD, &msgSize, sizeof(uint16_t), MSG_WAITALL);
	msgSize = ntohs(msgSize);
	//Client disconnected!
	if(msgSize>MAX_MSG_SIZE){
		char* username = thisPart->name;
		char msg[strlen(username)+16];
		sprintf(msg, "User %s has left", username);
		closeParticipant(thisPart);
		sendAll(msg);

		return;
	}

	recv(thisPart->partSD, msg, sizeof(char)*msgSize, MSG_WAITALL);
		msg[msgSize] = '\0';

	if(msg[0]=='@')
		sendPrivateMsg(thisPart, msg);
	else
		sendPublicMsg(thisPart, msg);

}


/* send private message
 * checks if the recipient exists, then formats the message to send.
 * sends that message to both the sender's and recipient's observer
 *
 * if the recipient doesn't exist, send an error message to the sender
 */
void sendPrivateMsg(part* p, char* msg){

	char buf[10];
	char* recipient = parseRecipient(msg, buf);
	char* senderName = p->name;

	if(recipientIsValid(recipient)){

		char outMsg[strlen(msg)+14];

		outMsg[0] = '>';
		for(int i=1; i<(12-strlen(senderName)); i++){
			outMsg[i] = ' ';
		}
		int j=0;
		for(int i=12-strlen(senderName); i<12; i++){
			outMsg[i] = senderName[j++];
		}
		outMsg[12] = ':';
		outMsg[13] = ' ';

		int h=14;
		for(int i=strlen(recipient)+2; i<strlen(msg)+1; i++){
			outMsg[h++] = msg[i];
		}

		uint16_t nMsgSize = htons(strlen(outMsg));
		int msgSize = strlen(outMsg);

		//send to our own observer
		if(p->obsSD > -1){
			if(send(p->obsSD, &nMsgSize, sizeof(uint16_t),0)<0){perror("send1");exit(1);}
			if(send(p->obsSD, &outMsg, sizeof(char)*msgSize,0)<0){perror("send2");exit(1);}
		}

		//send to recipient's observer
		part* TEMP = getParticipantByName(recipient);
		if(TEMP != NULL){
			int obs_sd = TEMP->obsSD;
			if(obs_sd > -1){
				if(send(obs_sd, &nMsgSize, sizeof(uint16_t),0)<0){perror("send3");exit(1);}
				if(send(obs_sd, &outMsg, sizeof(char)*msgSize,0)<0){perror("send4");exit(1);}
			}
		}
	}
	else{

		char outMsg[strlen(recipient)+33];
		sprintf(outMsg, "Warning: user %s doesn't exist...", recipient);

		uint16_t msgSize = htons(strlen(outMsg));

		// send to the sender's observer
		if(send(p->obsSD, &msgSize, sizeof(uint16_t),0)<0){perror("send");exit(1);}
		if(send(p->obsSD, &outMsg, sizeof(char)*msgSize,0)<0){perror("send");exit(1);}


	}
}

/* get participant by name
 * finds a participant in the list of participant-observer pairs
 */
part* getParticipantByName(char* name){
	part* retVal = NULL;

	for(int i=0; i<pIndex; i++){
		if(!strcmp(allParts[i]->name, name)){
			retVal = allParts[i];
			break;
		}
	}

	return retVal;
}


/* parse recipient
 * parses a message that begins with '@' for its
 * recipient, which is always between the @ and a space
 * ie: "@username message"
 */
char* parseRecipient(char* msg, char* buf){

	int i=1;

	bool currentlyParsing = true;
	while(currentlyParsing){

		if(msg[i]==' '){
			currentlyParsing = false;
			buf[i-1] = '\0';
		}
		else{
			buf[i-1] = msg[i];
		}

		i++;
	}

	return buf;

}

/* recipient is valid?
 * checks that a recipient exists in allNames
 * (possibly we need to check that the participant has an observer attached?)
 */
bool recipientIsValid(char* recipient){
	return true;//DEBUG
}

void sendPublicMsg(part* p, char* msg){

	char* senderName = p->name;

	char outMsg[strlen(msg)+14];

	outMsg[0] = '>';
	for(int i=1; i<(12-strlen(senderName)); i++){
		outMsg[i] = ' ';
	}
	int j=0;
	for(int i=12-strlen(senderName); i<12; i++){
		outMsg[i] = senderName[j++];
	}
	outMsg[12] = ':';
	outMsg[13] = ' ';

	int h=14;
	for(int i=0; i<strlen(msg)+1; i++){
		outMsg[h++] = msg[i];
	}
	sendAll(outMsg);

}

void observe(int sd){

}



/* send all
 * sends all observers a generic string
*/
void sendAll(char* msg){
	uint16_t nMsgSize = htons(strlen(msg));
	int msgSize = strlen(msg);

	//DEBUG: not sure if this needs to be oIndex+1 or not
	for(int i=0; i<oIndex; i++){
		if(send(allObs[i]->obsSD, &nMsgSize, sizeof(uint16_t),0)<0){perror("send");exit(1);}
		if(send(allObs[i]->obsSD, msg, sizeof(char)*msgSize,0)<0){perror("send");exit(1);}
	}
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

	pair* myObs = findObserver(thisPart);
	if(myObs != NULL) closeObserver(myObs);
	close(thisPart->partSD);

	free(thisPart);

}

void closeObserver(pair* thisPair){
	int i;
	for(i=0; i<MAX_CLIENTS; i++){
		if(allObs[i] == thisPair){

			if(thisPair->partner != NULL){
				thisPair->partner->hasPartner = false;
				thisPair->partner->obsSD = -1;
			}
			close(thisPair->obsSD);

			oIndex--;
			break;
		}
	}
	for(int j = i; j < MAX_CLIENTS; j++){
		allObs[j] = allObs[j+1];
	}
}

pair* findObserver(part* p){
	pair* thisPair = NULL;

	for(int i=0; i<oIndex; i++){
		if(allObs[i] != NULL && allObs[i]->partner == p)
			thisPair = allObs[i];
	}

	return thisPair;
}

pair* newPair(){
	pair* thisPair = malloc(sizeof(pair));

	if(thisPair == NULL){
		perror("Malloc");
		exit(1);
	}

	thisPair->partner = NULL;
	thisPair->startTime = clock();

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

	thisPart->obsSD = -1;
	thisPart->hasPartner = false;
	thisPart->startTime = clock();

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
