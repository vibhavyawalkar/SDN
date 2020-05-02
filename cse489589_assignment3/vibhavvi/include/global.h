#ifndef GLOBAL_H_
#define GLOBAL_H_

#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <iostream>
#include <vector>
#include <netinet/in.h>
#include <arpa/inet.h>

using namespace std;

typedef enum {FALSE, TRUE} boolean;

#define ERROR(err_msg) {perror(err_msg); exit(EXIT_FAILURE);}

/* https://scaryreasoner.wordpress.com/2009/02/28/checking-sizeof-at-compile-time/ */
#define BUILD_BUG_ON(condition) ((void)sizeof(char[1 - 2*!!(condition)])) // Interesting stuff to read if you are interested to know how this works

#define INF 65536
extern uint16_t CONTROL_PORT;
extern uint16_t ROUTER_PORT;
extern uint16_t DATA_PORT;
/* INIT payload Structure */
extern uint16_t noOfRouters;
extern uint16_t update_interval;
extern int myIndex;

class routerInfo {
public:
	uint16_t ID;
	uint16_t router_port;
	uint16_t data_port;
	uint16_t cost;
	char ip[INET_ADDRSTRLEN];
	//uint32_t ip;
	bool neighbour;
};

vector<routerInfo> routers(5);

extern uint16_t myRouterId;
#endif
