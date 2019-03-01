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

typedef struct initClientStruct{
	int init_sd;
	char init_player;
	uint8_t init_boardSize;
	uint8_t init_sec;
} initClientStruct;


