#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <semaphore.h>
#include <dirent.h>
#include <regex.h>
#include "headers/server_utils.h"


/* * * * * * * * * * *
                        GETTERS
                                * * * * * * * * * * */

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


/* * * * * * * * * * *
                        MESSAGES
                                * * * * * * * * * * */

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
        strcat(affichage, msg + strlen(name) + 1); // moves the pointer after the name
        printf("affichage : %s\n", affichage );
        send(num_socket, affichage, MAX_MSG_SIZE, 0);
    }
    bzero(affichage,MAX_MSG_SIZE+15);
}

/**
 * Sends a private message to another client based on it's id.
 * @param msg the message to send
 * @param clients the clients array
 * @param to_client the receiver of the message
 * @param from_client the sender of the message
 */
void send_message_to_client(char *msg, Client clients[], int to_client, int from_client) {
    int num_socket = clients[to_client].client_msg_socket;
    //char copy_msg[MAX_MSG_SIZE];
    //strcpy(copy_msg, msg);
    char affichage[MAX_MSG_SIZE+15];
    if (num_socket > 0) {
        strcat(affichage,"[");
        strcat(affichage, clients[from_client].pseudo); // peut etre segmentation fault
        strcat(affichage,"] : ");
        strcat(affichage, msg);
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
void broadcast_message(char *msg, Client clients[], int from_client_index) {
    int client_socket = clients[from_client_index].client_msg_socket;
    char nom[12];
    char aff[MAX_MSG_SIZE+15];
    get_name_by_socket(clients, clients[from_client_index].client_msg_socket, nom);
    printf("Pseudo de l'emetteur du message %s\n", nom);
    strcat(aff,"[");
    strcat(aff,nom);
    strcat(aff,"] : ");
    strcat(aff,msg);
    printf("[%s](%d): %s", clients[from_client_index].pseudo, from_client_index, msg);
    for (int j = 0; j < MAX_CLIENTS; j++) { // pour tous les clients du tableau
        printf("- client : %d, socket : %d\n", j,  clients[j].client_msg_socket);
        if (clients[j].client_msg_socket != client_socket && clients[j].client_msg_socket != 0) { // envoi
            send(clients[j].client_msg_socket, aff, MAX_MSG_SIZE, 0); // modifié le j en clients[j]
            printf("-----> envoyé au client : %s\n", msg);
        } else {
            printf("--X--> on n'envoie pas\n");
        }
    }
    bzero(aff,MAX_MSG_SIZE+15);
}

/**
 * Broadcasts a message in a given room.
 */
void broadcast_message_in_room(char* msg, Client clients[], Room rooms[], int to_room, int from_client_index) {
    Room client_room = rooms[to_room];
    for (int i = 0; i < sizeof(client_room.membres); i++) {
        if (client_room.membres[i] == 1 && i!=from_client_index && is_in_room(from_client_index, to_room, clients)==1) { // the client is in the room
            send_message_to_client(msg, clients, i, from_client_index);
        }
    }
}


/* * * * * * * * * * *
                        SOCKETS
                                * * * * * * * * * * */

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



/* * * * * * * * * * *
                        ROOMS
                              * * * * * * * * * * */

/**
 * @brief Puts a formatted list of the available rooms in a given buffer.
 * @param rooms the rooms array
 * @param list the buffer in which the room list will be stored
 */
void list_Rooms(Room rooms[], char *list) {
    strcpy(list,"Liste des salons :\n");
    for(int r = 0; r < NB_MAX_ROOM; r++) {
        if (rooms[r].created == 1){
            strcat(list, "- ");
            char room_id[5];
            sprintf(room_id, "[%d] ", r);
            strcat(list, room_id);
            strcat(list, "Nom: ");
            strcat(list, rooms[r].room_name);
            strcat(list, "\n");
        }
    }
}


/**
 * @brief Makes a given client join a given room.
 * @param client_id the client's id
 * @param room_id the room's id
 * @param clients the clients array
 * @param rooms the rooms array
 */
void join_room(int client_id, int room_id, Client clients[], Room rooms[]) {
    clients[client_id].rooms[room_id] = 1;
    rooms[room_id].membres[client_id] = 1;
}

/**
 * @brief Makes a given client leave a given room.
 * @param client_id the client's id
 * @param room_id the room's id
 * @param clients the clients array
 * @param rooms the rooms array
 */
void leave_room(int client_id, int room_id, Client clients[], Room rooms[]) {
    clients[client_id].rooms[room_id] = 0;
    rooms[room_id].membres[client_id] = 0;
}

/**
* Returns 1 if a room is full; 0 otherwise.
*/
int is_room_complete(int room_id, Room rooms[]) {
    int compteur=0; //nb de clients connectés
    for (int a = 0; a<MAX_CLIENTS; a++) {
        if (rooms[room_id].membres[a]==1) {
            compteur+=1;
        }
    }
    if (compteur==rooms[room_id].nb_max_membre) {
        return 1;
    }
    return 0;
}

/**
* If the message is a message that needs to be sent to a room, returns the room id; -1 otherwise.
*/
int get_room_id_from_message(char* msg) {
    regex_t regex;
    int reg_result;
    regcomp(&regex, "^/[0-2]", 0); // compiles the regex
    reg_result = regexec(&regex, msg, 0, NULL, 0); // checks if the msg matches the regex
    if (reg_result == 0) {
        int room_id = atoi(&msg[1]);
        return room_id;
    } else {
        return -1;
    }
}

/**
 * @brief Tells if a given client is in a given room.
 * @param client_id the client's id
 * @param id_room the room's id
 * @param clients the clients array
 * @return 1 if the client is in the room; 0 otherwise
 */
int is_in_room(int client_id, int id_room, Client clients[]){
    if (clients[client_id].rooms[id_room] == 1){
        return 1;
    }
    return 0;
}

/**
 * @brief Manages the creation of a room on the server's side. This
 * function receives the necessary information in order to create a room.
 * @param socket the socket from which the information will be received
 */
void server_room_creation(int socket, Room rooms[]) {
    printf("## Création d'un salon\n");
    char room_name[20];
    recv(socket, room_name, 20, 0);
    printf("Nom du salon : %s\n", room_name);

    int max_members;
    recv(socket, &max_members, sizeof(int), 0);
    printf("Nb. max. de membres du salon : %d\n", max_members);

    char response[MAX_MSG_SIZE];
    // Finds an available empty slot to create the room
    for (int i = 0; i < NB_MAX_ROOM; i++) {
        if (rooms[i].created == 0) {
            rooms[i].created = 1;
            rooms[i].nb_max_membre = max_members;
            strcpy(rooms[i].room_name, room_name);
            strcpy(response, "Salon créé.");
            send(socket, response, MAX_MSG_SIZE, 0);
            // NE PAS METTRE DE PRINT ICI SINON ÇA BLOQUE
            return;
        }
    }
    strcpy(response, "Nombre maximum de rooms atteint.");
    send(socket, response, MAX_MSG_SIZE, 0);
}

/**
 * @brief Deletes a room and removes all its members.
 * @param room_id the room's id
 * @param clients the clients array
 * @param rooms the rooms array
 */
void delete_room(int room_id, Client clients[], Room rooms[]) {

    // Removes members from the room's members list
    for (int i = 0; i < MAX_CLIENTS; i++) {
        rooms[room_id].membres[i] = 0;
        clients[i].rooms[room_id] = 0;
    }

    rooms[room_id].created = 0;
    printf("Salon n°%d (%s) supprimé !\n", room_id, rooms[room_id].room_name);
}

/**
 * @brief Manages the modification of a room on the server's side. This
 * function receives the necessary information in order to modify a room and
 * sends everything to the server.
 * @param socket the socket from which the information will be received
 */
void server_room_modification(int socket, int choice, int room_id, Room *rooms) {
    printf("## Modification du salon n° %d.\n", room_id);
    char response[MAX_MSG_SIZE];

    switch(choice){
        case 1: { // modifies the room's name
            char room_name[20];
            recv(socket, room_name, 20, 0);
            printf("Nom du salon : %s\n", room_name);
            strcpy(rooms[room_id].room_name, room_name);

            strcpy(response, "Nom du salon modifié.");
            break;
        }
        case 2: { // modifies the room's max members number
            int members;
            recv(socket, &members, sizeof(int), 0);
            printf("Nb. max. de membres du salon : %d\n", members);
            rooms[room_id].nb_max_membre = members;

            strcpy(response, "Nb. max. de membres du salon modifié.");
            break;
        }
        default:
            strcpy(response, "Aucune modification effectuée (mauvaise commande).\n");
            break;
    }
    send(socket, response, MAX_MSG_SIZE, 0);
}