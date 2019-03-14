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
#define MAX_MSG_SIZE 1000

typedef struct initServerStruct {
	struct sockaddr_in init_partcad;
	struct sockaddr_in init_obscad;
	int init_partsd;
	int init_obssd;
} initServerStruct;

typedef struct pair{
	int obsSD;
	int partSD;
	char* name;
} pair;

typedef struct part{
	int sd;
	char* name;
} part;

part allParts[MAX_CLIENTS];
int pIndex = 0;
pair allObs[MAX_CLIENTS];//array of pairs of observers watching participants
int oIndex = 0;
char* allNames[MAX_CLIENTS];


initServerStruct initServer(int argc, char** argv);
void mainAcceptLoop(struct sockaddr_in partcad, struct sockaddr_in obscad, int partsd, int obssd);
void addParticipant(int sd);
void addObserver(int sd);
void closeSocket(int sd);
char* negotiateUserName(int sd, char[]);
void sendAll(char* username);
bool nameTaken(char* username);
bool validateName(char* username);
void chat(int sd, char* username);
void observe(int sd);
void sendPublicMsg(int sd, char* msg, char* username);
void sendPrivateMsg(int sd, char* msg);
char* parseRecipient(char* msg);
bool recipientIsValid(char* recipient);
int getParticipantByName(char* name);
int getObserver(int sd);
char canPairWithParticipant(int obsSD, char* username);
