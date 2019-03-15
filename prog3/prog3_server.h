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


typedef struct part{
	int partSD;
	int obsSD;
	char name[11];
	bool hasPartner;
} part;

typedef struct pair{
	int obsSD;
	part* partner;
} pair;

part* allParts[MAX_CLIENTS];
int pIndex = 0;
pair* allObs[MAX_CLIENTS];//array of pairs of observers watching participants
int oIndex = 0;
char* allNames[MAX_CLIENTS];


initServerStruct initServer(int argc, char** argv);
void mainAcceptLoop(struct sockaddr_in partcad, struct sockaddr_in obscad, int partsd, int obssd);
void addParticipant(part* thisPart);
void addObserver(pair* thisPair);
void closeParticipant(part* thisPart);
void closeObserver(pair* thisPair);
char* negotiateUserName(int sd, char[]);
void sendAll(char* username);
bool nameTaken(char* username);
bool validateName(char* username);
void chat(part*);
void observe(int sd);
void sendPublicMsg(int sd, char* msg, char* username);
void sendPrivateMsg(part* p, char* msg);
char* parseRecipient(char* msg);
bool recipientIsValid(char* recipient);
part* getParticipantByName(char* name);
int getObserver(int sd);
char canPairWithParticipant(int obsSD, char* username);
part* newPart();
pair* newPair();
