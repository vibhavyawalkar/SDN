/**
 * @connection_manager
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
 * Connection Manager listens for incoming connections/messages from the
 * controller and other routers and calls the desginated handlers.
 */

#include <sys/select.h>
#include <sys/time.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>
#include <iostream>
using namespace std;
#include "../include/connection_manager.h"
#include "../include/global.h"
#include "../include/control_handler.h"
#include "../include/network_util.h"
#include "../include/control_header_lib.h"

#define SIZE_OF_ONE_ROUTER_ENTRY 12
fd_set master_list, watch_list;
int head_fd;
int control_socket, router_socket, data_socket;

uint16_t noOfRouters, update_interval, myRouterID;
vector<routerInfo> routers(5);
vector<vector<uint16_t>> DVMatrix(5, vector<uint16_t>(5,0));
vector<vector<int>> HopMatrix(5, vector<int>(5,0));

int myIndex;

void init();
void timeOutHandler();
void localTimeoutHandler();
time_t mostRecentTimeout();
std::string getDV();

void main_loop()
{
	cout << "Starting main loop " << endl;
    int selret, sock_index, fdaccept;
	struct timeval t;
	t.tv_sec = 3600;
	t.tv_usec = 0;

    while(TRUE){

        watch_list = master_list;
        selret = select(head_fd+1, &watch_list, NULL, NULL, &t);
	cout << "Select returned" << endl;
        if(selret < 0)
            ERROR("select failed.");
	if(selret == 0) {
	/* Timeout here, call the call back function */
		LOG_PRINT("Select timed out");
		localTimeoutHandler();
		timeOutHandler();
	} else {

        	/* Loop through file descriptors to check which ones are ready */
        	for(sock_index=0; sock_index<=head_fd; sock_index+=1){

            	if(FD_ISSET(sock_index, &watch_list)){

                	/* control_socket */
                	if(sock_index == control_socket){
				cout << "Connection on control socket" << endl;
                    		fdaccept = new_control_conn(sock_index);

                    	/* Add to watched socket list */
                    	FD_SET(fdaccept, &master_list);
                    	if(fdaccept > head_fd) head_fd = fdaccept;
                	}

                	/* router_socket */
                	else if(sock_index == router_socket){
                    	//call handler that will call recvfrom() .....
                    		struct sockaddr recvaddr;
				socklen_t len;
				char buffer[1024];
				memset(buffer, '\0', 1024);
				string str = "";
				recvfrom(router_socket, buffer, sizeof(buffer), 0, &recvaddr, &len);
				str = std::string(buffer);
				std::size_t pos = str.find(" ");
				string index_ = str.substr(0, pos);
				int index = stoi(index_);
				struct timeval timenow;
				gettimeofday(&timenow, NULL);

				routers[index].nextUpdateTime = timenow.tv_sec + update_interval;
				routers[index].noOfTimeouts = 3;

				cout << "Message received from ID: " << routers[index].ID <<
					" Msg:" << str.c_str() << endl;
				LOG_PRINT("Message received from ID: %d, Msg:%s", routers[index].ID, str.c_str());
			
				// Process the message received	
                     
                	}

                	/* data_socket */
                	else if(sock_index == data_socket){
                    	//new_data_conn(sock_index);
                	}

                	/* Existing connection */
                	else{
                    		if(isControl(sock_index)){
                        		if(!control_recv_hook(sock_index)) FD_CLR(sock_index, &master_list);
					if(router_socket > 0 && 
						!FD_ISSET(router_socket, &master_list)) { /* After Init we have created router_socket */
						/*Register the router socket */
						FD_SET(router_socket, &master_list);
                				if(router_socket > head_fd)
                        				head_fd = router_socket;
					}

                			if(data_socket > 0 && 
						!FD_ISSET(data_socket, &master_list)) {
                				/* Register the data socket */
                				FD_SET(data_socket, &master_list);
                				if(data_socket > head_fd)
                        				head_fd = data_socket;

                    			}
				}	
                    		/* else if isData(sock_index) {
 				 * }
 				 */
                    		else ERROR("Unknown socket index");
                	}
            	} /* End of FD_ISSET */
		} /* End of for loop for sock_index */
	} /* End of else for selret = 0 */

	t.tv_sec = mostRecentTimeout();
        t.tv_usec = 0;
	LOG_PRINT("Most recent timeout for select is:%d", t.tv_sec);
    } /*Infinite while loop for select*/
}

void init()
{
    control_socket = create_control_sock();

    //router_socket and data_socket will be initialized after INIT from controller

    FD_ZERO(&master_list);
    FD_ZERO(&watch_list);

    /* Register the control socket */
    FD_SET(control_socket, &master_list);
    head_fd = control_socket;

    router_socket = -1;
    data_socket = -1;
    main_loop();
}

/* Iterate over all timeout values for all routers 
 * and return the shortest one, since we would want 
 * select call block for that much amount of time
 */

time_t mostRecentTimeout() {
	struct timeval timenow;
	gettimeofday(&timenow, NULL);
	time_t ret = 65535; 
	for(int i = 0; i < noOfRouters; i++) {
		if(routers[i].nextUpdateTime - timenow.tv_sec < ret
		&& routers[i].nextUpdateTime - timenow.tv_sec > 0) {
			ret = routers[i].nextUpdateTime;
		}
	}
	return ret;
}

/* IF there is a timeout on the local node then we
 * send the DV to neighbouring routers */
void sendDVtoNeighbours() {
	for(int i = 0; i < noOfRouters; i++) {
		if(routers[i].neighbour == true && i != myIndex) {
			struct sockaddr_in addr;
			bzero(&addr, sizeof(addr));
			addr.sin_family = AF_INET;	
			addr.sin_addr.s_addr = routers[i].ip; // Need to check conversion for this IP
			addr.sin_port = htons(routers[i].router_port);
			string dv = getDV();
			sendto(router_socket, dv.c_str(), dv.length(), 0, (struct sockaddr*)&addr, sizeof(addr));
		}
	}
}

string getDV() {
	string s = to_string(myIndex); // Can send ROuterID here
	for(int i = 0; i < noOfRouters; i++) {
		s = s + " ";
		s = s + to_string(DVMatrix[myIndex][i]);
	}
	cout << "DV string:" << s.c_str();
	LOG_PRINT("DV String %s:", s.c_str());
	return s;
}

void localTimeoutHandler() {
	struct timeval timenow;
	gettimeofday(&timenow, NULL);
	if(routers[myIndex].nextUpdateTime == timenow.tv_sec) {
		sendDVtoNeighbours();
		routers[myIndex].nextUpdateTime = timenow.tv_sec + update_interval;
	}
}

void timeOutHandler() {
	struct timeval timenow;
	gettimeofday(&timenow, NULL);
	for(int i = 0; i < noOfRouters; i++) {
		if(routers[i].nextUpdateTime == timenow.tv_sec && i != myIndex) {
			routers[i].nextUpdateTime = timenow.tv_sec + update_interval;
			routers[i].noOfTimeouts--;
			if(routers[i].noOfTimeouts == 0) {
			/* Mark this router as failed, since timedout
 			 * 3 times */
				routers[i].cost = INF;
				routers[i].neighbour = false;
				DVMatrix[myIndex][i] = INF;
			}

		}
	}
}

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
        for(int i = 0; i < noOfRouters; i++) {
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

        /* Get this node details and set its neighbours(adjacent routers) */
        for(int i = 0; i < noOfRouters; i++) {
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

       /* Initialize the Distance Vector matrix*/
        for(int i = 0; i < noOfRouters; i++) {
                for(int j = 0; j < noOfRouters; j++) {
                        if(i == myIndex) { /* Initialize the row for this router */
                                DVMatrix[i][j] =  routers[j].cost;
                        } else {
                                DVMatrix[i][j] = INF;
                        }
                }
        }

        /* Initialize the Hop matrix*/
        for(int i = 0; i < noOfRouters; i++) {
                for(int j = 0; j < noOfRouters; j++) {
                        if(i == myIndex && DVMatrix[i][j] != INF) { /* Initialize the row for this router */
                                HopMatrix[i][j] = routers[j].ID; //(j + 1); /* Since router ID doesn't start from 1..*/
                        } else {
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
}


void init_response(int sock_index) {
        cout << "Entering init response" << endl;
        LOG_PRINT("Entering init response");
        uint16_t payload_len, response_len;
        char *cntrl_response_header, *cntrl_response_payload, *cntrl_response;

        payload_len = 0; /*No need to send payload in the response of the INIT message */

        cntrl_response_header = create_response_header(sock_index, 1, 0, payload_len);

        response_len = CNTRL_RESP_HEADER_SIZE+payload_len;
        cntrl_response = (char*) malloc(response_len);
        /* Copy Header */
        memcpy(cntrl_response, cntrl_response_header, CNTRL_RESP_HEADER_SIZE);
        free(cntrl_response_header);

        sendALL(sock_index, cntrl_response, response_len);

        free(cntrl_response);
        cout<< "Exiting init response"<< endl;
        LOG_PRINT("Exiting init response");
}

void routing_response(int sock_index) {
        cout << "Entering routing response" << endl;
        LOG_PRINT("ENtering routing response");
        uint16_t payload_len, response_len;
        char * cntrl_response_header, *cntrl_response_payload, *cntrl_response;

        /* Four 2 byte fields in the payload for each router
 *          * Router ID, Padding, Next-HopID and cost */
        payload_len = 8 * noOfRouters;

        cntrl_response_header = create_response_header(sock_index, 2, 0, payload_len);

        response_len = CNTRL_RESP_HEADER_SIZE+payload_len;
        cntrl_response = (char*)malloc(response_len);

        /* Copy Header */
        memcpy(cntrl_response, cntrl_response_header, CNTRL_RESP_HEADER_SIZE);
        free(cntrl_response_header);

        /* Form payload */
        cntrl_response_payload = (char*)malloc(payload_len);
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
        /* Copy Payload */
        memcpy(cntrl_response+CNTRL_RESP_HEADER_SIZE, cntrl_response_payload, payload_len);
        free(cntrl_response_payload);

        sendALL(sock_index, cntrl_response, response_len);
        free(cntrl_response);
        cout << "Exiting router response" << endl;
        LOG_PRINT("Exiting router response");
}

