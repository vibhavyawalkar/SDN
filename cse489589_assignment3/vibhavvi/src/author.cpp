/**
 * @author
 * @author  Vibhav Virendra Yawalkar <vibhavvi@buffalo.edu>
 * @version 1.0
 *
 * @section LICENSE
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details at
 * http://www.gnu.org/copyleft/gpl.html
 *
 * @section DESCRIPTION
 *
 * AUTHOR [Control Code: 0x00]
 */

#include <string.h>

#include "../include/global.h"
#include "../include/control_header_lib.h"
#include "../include/network_util.h"
#include "../include/connection_manager.h"

#define AUTHOR_STATEMENT "I, vibhavvi, have read and understood the course academic integrity policy."

#define SIZE_OF_ONE_ROUTER_ENTRY 12
/*
uint16_t noOfRouters, update_interval, myRouterID;
vector<routerInfo> routers(5);
vector<vector<uint16_t>> DVMatrix(5, vector<uint16_t>(5,0));
vector<vector<int>> HopMatrix(5, vector<int>(5,0));

int myIndex;
*/
void author_response(int sock_index)
{
	cout << "Entering author response" << endl;
	LOG_PRINT("Entering author response");
	uint16_t payload_len, response_len;
	char *cntrl_response_header, *cntrl_response_payload, *cntrl_response;

	payload_len = sizeof(AUTHOR_STATEMENT)-1; // Discount the NULL chararcter
	cntrl_response_payload = (char *) malloc(payload_len);
	memcpy(cntrl_response_payload, AUTHOR_STATEMENT, payload_len);

	cntrl_response_header = create_response_header(sock_index, 0, 0, payload_len);

	response_len = CNTRL_RESP_HEADER_SIZE+payload_len;
	cntrl_response = (char *) malloc(response_len);
	/* Copy Header */
	memcpy(cntrl_response, cntrl_response_header, CNTRL_RESP_HEADER_SIZE);
	free(cntrl_response_header);
	/* Copy Payload */
	memcpy(cntrl_response+CNTRL_RESP_HEADER_SIZE, cntrl_response_payload, payload_len);
	free(cntrl_response_payload);

	sendALL(sock_index, cntrl_response, response_len);

	free(cntrl_response);
	cout << "Exiting author response" << endl;
	LOG_PRINT("Entering author response");
}
/*
void do_init(char * cntrl_payload) {
	cout << "Entering do init " << endl;
	LOG_PRINT("Entering do init");
	uint16_t bytes; 
	memcpy(&bytes, cntrl_payload+0x00, sizeof(noOfRouters));
	noOfRouters = ntohs(bytes); // Network to host order
	memcpy(&bytes, cntrl_payload+0x02, sizeof(update_interval));
	update_interval = ntohs(bytes);

	cout << "Number of routers :" << noOfRouters << endl;
	cout << "Update Interval:" << update_interval << endl;

	 LOG_PRINT("Number of routers :%d", noOfRouters);
         LOG_PRINT("Update Interval:%d", update_interval);
	/* Populate the router information array */
/*	for(int i = 0; i < noOfRouters; i++) {
		memcpy(&bytes, cntrl_payload+0x04 + SIZE_OF_ONE_ROUTER_ENTRY*i, sizeof(bytes));
		routers[i].ID = ntohs(bytes);
		memcpy(&bytes, cntrl_payload+0x06 + SIZE_OF_ONE_ROUTER_ENTRY*i, sizeof(bytes));
		routers[i].router_port = ntohs(bytes);
		memcpy(&bytes, cntrl_payload+0x08 + SIZE_OF_ONE_ROUTER_ENTRY*i, sizeof(bytes));
		routers[i].data_port = ntohs(bytes);
		memcpy(&bytes, cntrl_payload+0x0a + SIZE_OF_ONE_ROUTER_ENTRY*i, sizeof(bytes));
		routers[i].cost = ntohs(bytes);
		uint32_t fourBytes;
		memcpy(&routers[i].ip, cntrl_payload+0x0c + SIZE_OF_ONE_ROUTER_ENTRY*i, sizeof(routers[i].ip));
		memcpy(&fourBytes, cntrl_payload+0x0c + SIZE_OF_ONE_ROUTER_ENTRY*i, sizeof(fourBytes));
		inet_ntop(AF_INET, &fourBytes, routers[i].ipPrintable, INET_ADDRSTRLEN);
		cout << "Router ID:" << routers[i].ID << endl;
		cout << "Router Info:" << endl;
		cout << "Router Port:" << routers[i].router_port << "|";
		cout << "Data Port:" << routers[i].data_port << "|";
		cout << "Cost:" << routers[i].cost << "|";
		cout << "IP:" << routers[i].ipPrintable << "|" << endl;
		LOG_PRINT("Router ID:%d", routers[i].ID);
                LOG_PRINT("Router Info:");
                LOG_PRINT("Router Port:%d|", routers[i].router_port);
                LOG_PRINT("Data Port:%d|", routers[i].data_port);
                LOG_PRINT("Cost:%d|", routers[i].cost);
                LOG_PRINT("IP:%s|", routers[i].ipPrintable);
		routers[i].nextUpdateTime = update_interval;
		routers[i].noOfTimeouts = 3; 
	}

*/	/* Get this node details and set its neighbours(adjacent routers) */
/*	for(int i = 0; i < noOfRouters; i++) {
		if(routers[i].cost == 0) {
			ROUTER_PORT = routers[i].router_port;
			DATA_PORT = routers[i].data_port;
			myIndex = i;			
		} else if(routers[i].cost == INF) {
			routers[i].neighbour = false;
		} else {
			routers[i].neighbour = true;
		}
	}

*/	/* Initialize the Distance Vector matrix*/
/*	for(int i = 0; i < noOfRouters; i++) {
		for(int j = 0; j < noOfRouters; j++) {
			if(i == myIndex) { *//* Initialize the row for this router */
/*				DVMatrix[i][j] =  routers[j].cost;
			} else {
				DVMatrix[i][j] = INF;
			}
		}
	}
*/
	/* Initialize the Hop matrix*/
  /*      for(int i = 0; i < noOfRouters; i++) {
                for(int j = 0; j < noOfRouters; j++) {
                        if(i == myIndex && DVMatrix[i][j] != INF) {*/ /* Initialize the row for this router */
                              //  HopMatrix[i][j] = routers[j].ID; //(j + 1); /* Since router ID doesn't start from 1..*/
/*                        } else {
                                HopMatrix[i][j] = INF;
                        }
                }
        }


	for(int i = 0; i < noOfRouters; i++) {
        	for(int j = 0; j < noOfRouters; j++) {
                	cout << "DV[" << i << "][" << j << "]:" << DVMatrix[i][j] << endl;
                }
        }

	for(int i = 0; i < noOfRouters; i++) {
                for(int j = 0; j < noOfRouters; j++) {
                        cout << "HoP[" << i << "][" << j << "]:" << HopMatrix[i][j] << endl;
                }
        }
	cout << "Exiting do init" << endl;
	LOG_PRINT("Exiting do init");
}*/
/*
void init_response(int sock_index) {
	cout << "Entering init response" << endl;
	LOG_PRINT("Entering init response");
	uint16_t payload_len, response_len;
	char *cntrl_response_header, *cntrl_response_payload, *cntrl_response;

	payload_len = 0; *//*No need to send payload in the response of the INIT message */
/*
	cntrl_response_header = create_response_header(sock_index, 1, 0, payload_len);

	response_len = CNTRL_RESP_HEADER_SIZE+payload_len;
	cntrl_response = (char*) malloc(response_len);
*/	/* Copy Header */
/*	memcpy(cntrl_response, cntrl_response_header, CNTRL_RESP_HEADER_SIZE);
	free(cntrl_response_header);

	sendALL(sock_index, cntrl_response, response_len);

	free(cntrl_response);
	cout<< "Exiting init response"<< endl;
	LOG_PRINT("Exiting init response");
}
*/
void create_router_udp_socket() {
	int sock;
	struct sockaddr_in router_addr;
	socklen_t addrlen = sizeof(router_addr);

	sock = socket(AF_INET, SOCK_DGRAM, 0);
	if(sock < 0)
		ERROR("socket() failed");

	int optval = 1;
	/* Make socket re-usable */
	if(setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &optval /*(int[]){1}*/, sizeof(int)) < 0)
		ERROR("setsockopt() failed");

	bzero(&router_addr, sizeof(router_addr));

	router_addr.sin_family = AF_INET;
	router_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	router_addr.sin_port = htons(ROUTER_PORT);

	if(bind(sock, (struct sockaddr*)&router_addr, sizeof(router_addr)) < 0)
		ERROR("bind() failed");

	router_socket = sock;
}

void create_data_tcp_socket() {
        int sock;
        struct sockaddr_in data_addr;
        socklen_t addrlen = sizeof(data_addr);

        sock = socket(AF_INET, SOCK_STREAM, 0);
        if(sock < 0)
                ERROR("socket() failed");

        int optval = 1;
        /* Make socket re-usable */
        if(setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &optval /*(int[]){1}*/, sizeof(int)) < 0)
                ERROR("setsockopt() failed");

        bzero(&data_addr, sizeof(data_addr));

        data_addr.sin_family = AF_INET;
        data_addr.sin_addr.s_addr = htonl(INADDR_ANY);
        data_addr.sin_port = htons(DATA_PORT);

        if(bind(sock, (struct sockaddr*)&data_addr, sizeof(data_addr)) < 0)
                ERROR("bind() failed");

	if(listen(sock, 5) < 0)
		ERROR("listen failed()");

        data_socket = sock;
}
/*
void routing_response(int sock_index) {
	cout << "Entering routing response" << endl;
	LOG_PRINT("ENtering routing response");
	uint16_t payload_len, response_len;
	char * cntrl_response_header, *cntrl_response_payload, *cntrl_response;
*/
	/* Four 2 byte fields in the payload for each router
 	 * Router ID, Padding, Next-HopID and cost */ 
/*	payload_len = 8 * noOfRouters;

	cntrl_response_header = create_response_header(sock_index, 2, 0, payload_len);

	response_len = CNTRL_RESP_HEADER_SIZE+payload_len;
	cntrl_response = (char*)malloc(response_len);

*/	/* Copy Header */
/*	memcpy(cntrl_response, cntrl_response_header, CNTRL_RESP_HEADER_SIZE);
	free(cntrl_response_header);

*/	/* Form payload */
/*	cntrl_response_payload = (char*)malloc(payload_len);
	bzero(cntrl_response_payload, sizeof(cntrl_response_payload));
	for(int i = 0; i < noOfRouters; i++) {
		uint16_t bytes;
		bytes = htons(routers[i].ID);
		memcpy(cntrl_response_payload+0x08*i, &bytes, sizeof(bytes));
		bytes = htons(HopMatrix[myIndex][i]);
		memcpy(cntrl_response_payload+0x04+0x08*i, &bytes, sizeof(bytes));
		bytes = htons(DVMatrix[myIndex][i]);
		memcpy(cntrl_response_payload+0x06+0x08*i, &bytes, sizeof(bytes));
		cout << "ID:" << routers[i].ID << "|NextHopID:" << HopMatrix[myIndex][i] << "|Cost:" << DVMatrix[myIndex][i] << endl;
		LOG_PRINT("ID:%d|NextHopID:%d|Cost:%d", routers[i].ID, HopMatrix[myIndex][i], DVMatrix[myIndex][i]);
	}
*/	/* Copy Payload */
/*	memcpy(cntrl_response+CNTRL_RESP_HEADER_SIZE, cntrl_response_payload, payload_len);
	free(cntrl_response_payload);

	sendALL(sock_index, cntrl_response, response_len);
	free(cntrl_response);
	cout << "Exiting router response" << endl;
	LOG_PRINT("Exiting router response");
}
*/
