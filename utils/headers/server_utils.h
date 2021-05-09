#ifndef PROJET_FAR_SERVER_UTILS_H
#define PROJET_FAR_SERVER_UTILS_H
#endif

#include <semaphore.h>
#include "utils.h"

#define MAX_THREADS 10
#define MAX_CLIENTS 3


typedef struct Client Client;
struct Client {
    int client_socket;
    char pseudo[MAX_NAME_SIZE];
    pthread_t msg_thread;
    pthread_t file_thread;
    pthread_t file_send_thread;
};

int get_client_count(sem_t semaphore);

int get_index_by_socket(Client clients[], int socket);

int get_socket_by_name(Client clients[], char *name);

int get_name_by_socket(Client clients[], int socket, char buffer[MAX_NAME_SIZE]);

void send_message_to(char *msg, Client clients[],int from_client_socket);

int is_private_message(char *msg, Client clients[]);

void broadcast_message (char *msg, Client clients[], int from_client_index);

void register_client(Client clients[], int client_socket, void * thread_function);