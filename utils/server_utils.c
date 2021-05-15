#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <semaphore.h>
#include <dirent.h>
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
        if (clients[i].client_msg_socket == socket) {
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
            return clients[i].client_msg_socket;
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
void send_message_to(char *msg, Client clients[], int from_client_socket) {
    char *name = strtok(msg, " "); // extracts the name from the message
    int num_socket = get_socket_by_name(clients, name);
    char nom[MAX_NAME_SIZE];
    char affichage[MAX_MSG_SIZE+15];
    if (num_socket > 0) {
        strcat(affichage,"[");
        get_name_by_socket(clients,from_client_socket,nom);
        strcat(affichage,nom);
        strcat(affichage,"] : ");
        strcat(affichage,msg + strlen(name) + 1);
        printf("affichage : %s\n", affichage );
        send(num_socket, affichage, MAX_MSG_SIZE, 0); 
    }
    bzero(affichage,MAX_MSG_SIZE+15);
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
    int client_socket = clients[from_client_index].client_msg_socket;
    char nom[12];
    char aff[MAX_MSG_SIZE+15];
    get_name_by_socket(clients, clients[from_client_index].client_msg_socket, nom);
    printf("pseudo de celui qui envoie %s\n", nom);
    strcat(aff,"[");
    strcat(aff,nom);
    strcat(aff,"] : ");
    strcat(aff,msg);
    printf("[%s](%d): %s", clients[from_client_index].pseudo, from_client_index, msg);
    for (int j = 0; j < MAX_CLIENTS; j++) { // pour tous les clients du tableau
        printf("client %d : %d\n", j,  clients[j].client_msg_socket);
        if (clients[j].client_msg_socket != client_socket && clients[j].client_msg_socket != 0) { // envoi
            send(clients[j].client_msg_socket, aff, MAX_MSG_SIZE, 0); // modifié le j en clients[j]
            printf("Envoyé aux clients : %s\n", msg);
        } else {
            printf("On n'envoie pas\n");
        }
    }
    bzero(aff,MAX_MSG_SIZE+15);
}

/**
 * Configures the server's socket and updates the socket_return and addr_return values with the
 * created socket and the created address.
 * @param port the port we want to use
 * @param socket_return the pointer where the created socket will be stored at
 * @param addr_return the address where the created sockaddr_in will be stored at
 * @return 0 if everything was successful; -1 if there was an error during socket creation
 */
int configure_listening_socket(int port, int* socket_return, struct sockaddr_in *addr_return) {

    // creates a socket in the IPV4 domain using TCP protocol
    int server_socket = socket(PF_INET, SOCK_STREAM, 0);
    if (server_socket == -1) {
        perror("Erreur lors de la création de la socket serveur pour les messages.\n");
        return -1;
    }
    printf("Socket serveur créée avec succès.\n");

    // server address configuration
    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET; // address type
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons(port); // address port (converted from the CLI)
    printf("Adresse du serveur configurée avec succès ! (port : %d)\n", port);

    *socket_return = server_socket;
    *addr_return = server_address;

    return 0;
}

/**
 * Binds the given sockaddr_in to the given socket.
 * @param socket the socket we want to bind the address to
 * @param address the address we want to bind
 * @return 0 if successful; -1 if bind or listen failed (with errno positioned)
 */
int bind_and_listen_on(int socket, struct sockaddr_in address) {
    // bind
    int bind_res = bind(socket, (struct sockaddr*) &address, sizeof(address)); // binds address to server socket
    if (bind_res == -1) {
        perror("Erreur lors du bind\\n");
        return -1;
    }

    // listen
    int listen_res = listen(socket, MAX_CLIENTS); // listens for incoming connections (maximum 2 waiting connections)
    if (listen_res == -1) {
        perror("Erreur lors du listen\\n");
        return -1;
    }
    printf("Le serveur écoute !\n");
    return 0;
}


int accept_client(int server_socket) {

    // clients address initialization
    struct sockaddr_in client_address;
    socklen_t client_address_len = sizeof(struct sockaddr_in);
    int client_socket = accept(server_socket, (struct sockaddr *) &client_address, &client_address_len);
    check_error(client_socket, "Erreur lors de l'acceptation du client.\n");
    send(client_socket, "Connexion acceptée\n", MAX_MSG_SIZE, 0);
    return client_socket;
}



