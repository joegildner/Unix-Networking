//Joseph Gildner and Quentin Jensen

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <signal.h>
#include <sys/time.h>
#include <errno.h>

#define QLEN 6
#define MAX_CLIENTS 255

typedef struct initServerStruct {
	struct sockaddr_in init_partcad;
	struct sockaddr_in init_obscad;
	int init_partsd;
	int init_obssd;
} initServerStruct;

int partsds[MAX_CLIENTS];
int iparts = 0;
int obssds[MAX_CLIENTS];
int iobs = 0;


initServerStruct initServer(int argc, char** argv);

void mainAcceptLoop(struct sockaddr_in partcad, struct sockaddr_in obscad,
										int partsd, int obssd);

void addToChat(int sd);
void observeChat(int sd);
void closeSocket(int sd);
