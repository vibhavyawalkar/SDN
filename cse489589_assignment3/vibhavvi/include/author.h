#ifndef AUTHOR_H_
#define AUTHOR_H_

void author_response(int sock_index);

/* Init related functions */
void do_init(char *cntrl_payload);
void init_response(int sock_index);
void create_router_udp_socket();
void create_data_tcp_socket();

void routing_response(int sock_index);
void handle_crash(int sock_index);
void updateRoutingTable(int sock_index);
#endif
