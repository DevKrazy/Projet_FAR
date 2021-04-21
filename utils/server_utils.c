#include <stdio.h>
#include <stdlib.h>
#include "headers/server_utils.h"


/**
 * Returns the amount of clients currently connected to the server.
 * @param max_clients_number
 * @param semaphore the semaphore counter
 * @return the amount of clients currently connected to the server
 */
int get_client_count(sem_t semaphore) {
    int sem_value;
    sem_getvalue(&semaphore, &sem_value);
    return MAX_CLIENTS - sem_value;
}


/**
 * Returns a client's index based on its socket number.
 * @param clients the clients array
 * @param socket the client's socket number
 * @return the index if the client is in the array; -1 if the client was not found
 */
int get_index_by_socket(Client clients[], int socket) {
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i].client_socket == socket) {
            return i;
        }
    }
    return -1;
}

/**
 * Returns a client's name based on its socket number.
 * @param clients the clients array
 * @param socket the client's socket number
 * @return the name if the client is in the array; -1 if the client was not found
 */
int get_name_by_socket(Client clients[], int socket, char *buffer) {
    int client_index = get_index_by_socket(clients, socket);
    if (client_index != -1) {
        *buffer = clients[client_index].pseudo;
        return 0;
    } else {
        return -1;
    }
}


