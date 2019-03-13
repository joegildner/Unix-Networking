// Quentin Jensen and Joseph Gildner
//

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

typedef struct initParticipantStruct{
	int init_sd;
} initParticipantStruct;

initParticipantStruct initParticipant(int argc, char** argv);
void chat(int sd);
void negotiateUserName(int sd);
char* promptUsername();
