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

#define MAX_MSG_SIZE 1000

typedef struct initParticipantStruct{
	int init_sd;
} initParticipantStruct;

initParticipantStruct initParticipant(int argc, char** argv);
void setup(int sd);
void negotiateUserName(int sd);
void chat(int sd);
