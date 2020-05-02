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

#define AUTHOR_STATEMENT "I, vibhavvi, have read and understood the course academic integrity policy."

#define SIZE_OF_ONE_ROUTER_ENTRY 12

uint16_t noOfRouters, update_interval, myRouterID;

void author_response(int sock_index)
{
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
}

void do_init(char * cntrl_payload) {
	uint16_t bytes; 
	memcpy(&bytes, cntrl_payload+0x00, sizeof(noOfRouters));
	noOfRouters = ntohs(bytes); // Network to host order
	memcpy(&bytes, cntrl_payload+0x02, sizeof(update_interval));
	update_interval = ntohs(bytes);

	cout << "Number of routers :" << noOfRouters << endl;
	cout << "Update Interval:" << update_interval << endl;

	/* Populate the router information array */
	for(int i = 0; i < noOfRouters; i++) {
		memcpy(&bytes, cntrl_payload+0x04 + SIZE_OF_ONE_ROUTER_ENTRY*i, sizeof(bytes));
		routers[i].ID = ntohs(bytes);
		memcpy(&bytes, cntrl_payload+0x04 + SIZE_OF_ONE_ROUTER_ENTRY*i, sizeof(bytes));
		routers[i].router_port = ntohs(bytes);
		memcpy(&bytes, cntrl_payload+0x04 + SIZE_OF_ONE_ROUTER_ENTRY*i, sizeof(bytes));
		routers[i].data_port = ntohs(bytes);
		memcpy(&bytes, cntrl_payload+0x04 + SIZE_OF_ONE_ROUTER_ENTRY*i, sizeof(bytes));
		routers[i].cost = ntohs(bytes);
		uint32_t fourBytes;
		memcpy(&fourBytes, cntrl_payload+0x04 + SIZE_OF_ONE_ROUTER_ENTRY*i, sizeof(fourBytes));
		inet_ntop(AF_INET, &fourBytes, routers[i].ip, INET_ADDRSTRLEN);
		cout << "Router ID:" << routers[i].ID << endl;
		cout << "Router Info:" << endl;
		cout << "Router Port:" << routers[i].router_port << "|";
		cout << "Data Port:" << routers[i].data_port << "|";
		cout << "Cost:" << routers[i].cost << "|";
		cout << "IP:" << routers[i].ip << "|" << endl;		 
	}

	/* Get this node details and set its neighbours(adjacent routers) */
	for(int i = 0; i < noOfRouters; i++) {
		if(routers[i].cost == 0) {
			ROUTER_PORT = routers[i].router_port;
			DATA_PORT = routers[i].data_port;
			//myIndex = i;			
		} else if(routers[i].cost == INF) {
			routers[i].neighbour = false;
		} else {
			routers[i].neighbour = true;
		}
	}
}

void init_response(int sock_index) {
	uint16_t payload_len, response_len;
	char *cntrl_response_header, *cntrl_response_payload, *cntrl_response;

	payload_len = 0; /*No need to send payload in the response of the INIT message */

	cntrl_response_header = create_response_header(sock_index, 0, 0, payload_len);

	response_len = CNTRL_RESP_HEADER_SIZE+payload_len;
	cntrl_response = (char*) malloc(response_len);
	/* Copy Header */
	memcpy(cntrl_response, cntrl_response_header, CNTRL_RESP_HEADER_SIZE);
	free(cntrl_response_header);

	sendALL(sock_index, cntrl_response, response_len);

	free(cntrl_response);
}
