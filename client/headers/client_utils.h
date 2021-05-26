#ifndef PROJET_FAR_CLIENT_UTILS_H
#define PROJET_FAR_CLIENT_UTILS_H
#endif

int configure_connecting_socket(char* address, int port, int* socket_return, struct sockaddr_in *addr_return);

int connect_on(int socket, struct sockaddr_in address);

void client_room_creation(int socket);

void print_room_modification_actions();

void client_room_modification(int socket, int choice);

void print_room_actions ();

int get_action_id(char* command);

void print_man();

void print_title(char* title);

void print_separator(int middle_size);

int is_string_a_number(char * saisie);