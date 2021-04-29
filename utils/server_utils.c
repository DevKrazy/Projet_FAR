#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
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
 * @return the client's index if the client is in the array; -1 if the client was not found
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
 * Returns a client's socket based on its name.
 * @param clients the clients array
 * @param socket the client's socket number
 * @return the client's socket if the client is in the array; -1 if the client was not found
 */

int get_socket_by_name(Client clients[], char *name) {
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (strcmp(clients[i].pseudo, name) == 0)  {
            return clients[i].client_socket;
        }
    }
    return -1;
}

/**
 * Returns a client's name based in its socket number.
 * @param clients the clients array
 * @param socket the client's socket number
 * @param buffer the client's name
 * @return the client's name if the client is in the array; -1 if the client was not found
 */
int get_name_by_socket(Client clients[], int socket, char buffer[MAX_NAME_SIZE]) {
    int client_index = get_index_by_socket(clients, socket);
    if (client_index != -1) {
        strcpy(buffer, clients[client_index].pseudo);
        return 0;
    } else {
        return -1;
    }
}

/**
 * Sends a private message to another client based on it's name. The name must be the
 * first word of the message. If the first word isn't a client name this function will alter the
 * passed string.
 * @param msg the message to send (the first word must be the receiver's name)
 * @param clients the clients array
 */
void send_message_to(char *msg, Client clients[]) {
    char *name = strtok(msg, " "); // extracts the name from the message
    int num_socket = get_socket_by_name(clients, name);
    if (num_socket > 0) {
        send(num_socket, msg + strlen(name) + 1, MAX_MSG_SIZE, 0); // increments the pointer to remove the name
    }
}

/**
 * Checks if a message is a private message (if the first word is a client's nickname).
 * @param msg the message to check
 * @param clients the clients array
 * @return 1 if the message is a private message; 0 otherwise
 */
int is_private_message(char *msg, Client clients[]) {
    char msg_copy[MAX_MSG_SIZE];
    strcpy(msg_copy, msg);
    char *name = strtok(msg, " "); // extracts the name from the message
    if (get_socket_by_name(clients, name) > 0) {
        // a socket was found for the client, so it exists in the clients array
        return 1;
    } else {
        // no socket was found, we resets the msg to its initial value
        strcpy(msg, msg_copy);
        return 0;
    }
}

/**
 * Broadcasts a message from a given client based on its index.
 * @param msg the message to check
 * @param clients the clients array
 * @return 1 if the message is a private message; 0 otherwise
 */
void broadcast_message (char *msg, Client clients[], int from_client_index) {
    int client_socket = clients[from_client_index].client_socket;
    printf("[%s](%d): %s", clients[from_client_index].pseudo, from_client_index, msg);
    for (int j = 0; j < MAX_CLIENTS; j++) { // pour tous les clients du tableau
        printf("client %d : %d\n", j,  clients[j].client_socket);
        if (clients[j].client_socket != client_socket && clients[j].client_socket != 0) { // envoi
            send(clients[j].client_socket, msg, MAX_MSG_SIZE, 0); // modifié le j en clients[j]
            printf("Envoyé au clients : %s\n", msg);
        } else {
            printf("On n'envoie pas\n");
        }
    }
}



