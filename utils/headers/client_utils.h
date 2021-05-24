#ifndef PROJET_FAR_CLIENT_UTILS_H
#define PROJET_FAR_CLIENT_UTILS_H
#endif

int configure_connecting_socket(char* address, int port, int* socket_return, struct sockaddr_in *addr_return);

int connect_on(int socket, struct sockaddr_in address);

void client_room_creation(int socket);

void Modification_choice();

void modification_Room(int socket, int choix);

void list_Choices ();

int getCommandChoice(char* command);