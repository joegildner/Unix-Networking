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


void mainAcceptLoop(struct sockaddr_in partcad, struct sockaddr_in obscad,
										int partsd, int obssd){

	bool ispart;

	fd_set rfds;
	int retval;

	socklen_t partalen = sizeof(partcad);
	socklen_t obsalen = sizeof(obscad);

	while (1) {

		FD_ZERO(&rfds);
		FD_SET(partsd, &rfds);
		FD_SET(obssd, &rfds);

		ispart = false;

		if ( (retval = select(FD_SETSIZE, &rfds, NULL, NULL, NULL) == -1)) {
			fprintf(stderr, "Error: Accept failed\n");
		 	exit(EXIT_FAILURE);
		}

		if(FD_ISSET(partsd, &rfds)){
			if ( (partsds[iparts]=accept(partsd, (struct sockaddr *)&partcad, &partalen)) < 0) {
				fprintf(stderr, "Error: Accept failed\n");
				exit(EXIT_FAILURE);
			}
			printf("part\n");
			ispart = true;
			iparts++;
		}

		else if(FD_ISSET(obssd, &rfds)){
			if ( (obssds[iobs]=accept(obssd, (struct sockaddr *)&obscad, &obsalen)) < 0) {
				fprintf(stderr, "Error: Accept failed\n");
				exit(EXIT_FAILURE);
			}
			printf("obs\n");
			iobs++;
		}

		const pid_t cpid = fork();
		switch(cpid) {

			//error
			case -1: {perror("fork");break;}

			case 0: {
				close(partsd);
				close(obssd);

				if(ispart)
					addToChat(partsds[iparts-1]);
				else
					observeChat(obssds[iobs-1]);

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



void addToChat(int sd){

	if(iparts==MAX_CLIENTS){
		char temp = 'N';
		if(send(sd, &temp, sizeof(char),0)<0){perror("send");exit(1);}
		closeSocket(sd);
		exit(0);
	}

	//send Y

	negotiateUserName(sd);
	sendAll(char* username);

}


/*
 *
*/
sendAll(char* username){
	//run through observer list and send
}



/*
 *
*/
negotiateUserName(int sd){
	//start 60 second timer
	//if timer runs out, close sd

	//recv length of username
	//recv username

	if(!nameTaken()){
		if(validateName()){
			//send Y
		}
		else{
			//send I
		}
	}
	else{
		//send t
		//reset timer
	}
	//if good, send Y
	//if not, send T and reset timer
}


void observeChat(int sd){
	printf("eat my ass\n");
}




//Iterates through both the observer and participant socket arrays
//until the desired socket descriptor is found, then removes this
//descriptor, decrements the descriptor count, and shifts everything
//else in the array to the left.
void closeSocket(int sd){
	int i;

	for(i=0; i<MAX_CLIENTS; i++){
		if(partsds[i] == sd){
			iparts--;
			break;
		}
	}
	for(int j = i; j < MAX_CLIENTS; j++){
		partsds[j] = partsds[j+1];
	}

	for(i=0; i<MAX_CLIENTS; i++){
		if(obssds[i] == sd){
			iobs--;
			break;
		}
	}
	for(int j = i; j < MAX_CLIENTS; j++){
		obssds[j] = obssds[j+1];
	}

}






/* initialize server
 * handles the many procedures of initializing our server,
 * and returns a struct of everything the main game needs to know
 * about
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
