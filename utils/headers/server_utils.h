#ifndef PROJET_FAR_SERVER_UTILS_H
#define PROJET_FAR_SERVER_UTILS_H
#endif

#include <semaphore.h>
#include "utils.h"

#define MAX_CLIENTS 3


typedef struct Room Room;
struct Room {
  int nb_max_membre;
  int membres[MAX_CLIENTS];
  char room_name[20];
  int socket_room_server;
  int num_port;
  struct sockaddr_in room_address;
};

typedef struct Client Client;
struct Client {
    int client_msg_socket;
    int client_file_socket;
    int client_room_socket;
    char pseudo[MAX_NAME_SIZE];
    pthread_t messaging_thread;
    pthread_t file_receiving_thread;
    pthread_t file_sending_thread;
    pthread_t room_thread;
};

int get_client_count(sem_t semaphore);

int get_index_by_socket(Client clients[], int socket);

int get_socket_by_name(Client clients[], char *name);

int get_name_by_socket(Client clients[], int socket, char buffer[MAX_NAME_SIZE]);

void send_message_to(char *msg, Client clients[],int from_client_socket);

int is_private_message(char *msg, Client clients[]);

void broadcast_message (char *msg, Client clients[], int from_client_index);

int configure_listening_socket(int port, int* socket_return, struct sockaddr_in *addr_return);

int bind_and_listen_on(int socket, struct sockaddr_in address);

int accept_client(int server_socket);

void list_Rooms (Room rooms [], char **list);
