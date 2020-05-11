/**
 * @vibhavvi_assignment3
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
 * This contains the main function. Read the CONTROL_PORT and start
 * the connection manager.
 */

#include "../include/global.h"
#include "../include/connection_manager.h"
#include <string.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>
#include <iostream>
/**
 * main function
 *
 * @param  argc Number of arguments
 * @param  argv The argument list
 * @return 0 EXIT_SUCCESS
 */

uint16_t CONTROL_PORT;
uint16_t ROUTER_PORT;
uint16_t DATA_PORT;

int main(int argc, char **argv)
{
    /*Start Here*/
/*	uint16_t noOfRouters, update_interval, myRouterID;
vector<routerInfo> routers(5);
vector<vector<uint16_t>> DVMatrix(5, vector<uint16_t>(5,0));
vector<vector<int>> HopMatrix(5, vector<int>(5,0));

int myIndex;
*/
    sscanf(argv[1], "%" SCNu16, &CONTROL_PORT);
//	LOG_PRINT("Initialize in main");
    init(); // Initialize connection manager; This will block

    return 0;
}
