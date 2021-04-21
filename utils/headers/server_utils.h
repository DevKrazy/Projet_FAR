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
};

int get_client_count(sem_t semaphore);

int get_index_by_socket(Client clients[], int socket);

int get_name_by_socket(Client clients[], int socket, char *buffer);

