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

typedef struct initObserverStruct{
	int init_sd;
} initObserverStruct;

initObserverStruct initObserver(int argc, char** argv);
void chat(int sd);
void observeMsg(int sd);
void negotiateUserName(int sd);
void setup(int sd);
