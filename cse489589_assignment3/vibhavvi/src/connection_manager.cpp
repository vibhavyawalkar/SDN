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
