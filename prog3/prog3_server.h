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

typedef struct initServerStruct {
	struct sockaddr_in init_cad;
	int init_sd;
	uint8_t init_boardSize;
	uint8_t init_sec;
} initServerStruct;


