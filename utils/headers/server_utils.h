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

/**
 * Returns the amount of clients currently connected to the server.
 * @param max_clients_number
 * @param semaphore the semaphore counter
 * @return the amount of clients currently connected to the server
 */
int get_client_count(sem_t semaphore);

/**
 * Returns a client's index based on its socket number.
 * @param clients the clients array
 * @param socket the client's socket number
 * @return the client's index if the client is in the array; -1 if the client was not found
 */
int get_index_by_socket(Client clients[], int socket);

/**
 * Returns a client's socket based on its name.
 * @param clients the clients array
 * @param socket the client's socket number
 * @return the client's socket if the client is in the array; -1 if the client was not found
 */

int get_socket_by_name(Client clients[], char *name);

/**
 * Returns a client's name based in its socket number.
 * @param clients the clients array
 * @param socket the client's socket number
 * @param buffer the client's name
 * @return the client's name if the client is in the array; -1 if the client was not found
 */
int get_name_by_socket(Client clients[], int socket, char buffer[MAX_NAME_SIZE]);

/**
 * Sends a private message to another client based on it's name. The name must be the
 * first word of the message. If the first word isn't a client name this function may cause problems.
 * @param msg the message to send (the first word must be the receiver's name)
 * @param clients the clients array
 */
void send_message_to(char *msg, Client clients[],int from_client_socket);

/**
 * Checks if a message is a private message (if the first word is a client's nickname).
 * @param msg the message to check
 * @param clients the clients array
 * @return 1 if the message is a private message; 0 otherwise
 */
int is_private_message(char *msg, Client clients[]);

/**
 * Broadcasts a message from a given client based on its index.
 * @param msg the message to check
 * @param clients the clients array
 * @return 1 if the message is a private message; 0 otherwise
 */
void broadcast_message (char *msg, Client clients[], int from_client_index);

void register_client(Client clients[], int client_socket, void * thread_function);