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

#include "../include/connection_manager.h"
#include "../include/global.h"
#include "../include/control_handler.h"

fd_set master_list, watch_list;
int head_fd;
int control_socket, router_socket, data_socket;

void main_loop()
{
	cout << "Starting main loop " << endl;
    int selret, sock_index, fdaccept;

    while(TRUE){

        watch_list = master_list;
        selret = select(head_fd+1, &watch_list, NULL, NULL, NULL);
	cout << "Select returned" << endl;
        if(selret < 0)
            ERROR("select failed.");
	if(selret == 0) {
	/* Timeout here, call the call back function */
	}

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
                    //else if isData(sock_index);
                    else ERROR("Unknown socket index");
                }
            }
        }
    }
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
