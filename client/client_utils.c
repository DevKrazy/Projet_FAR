#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include "headers/client_utils.h"
#include "../common/headers/utils.h"
#include <ctype.h>


/* * * * * * * * * * *
                        SOCKETS
                                * * * * * * * * * * */

/**
 * Configures the server's socket and updates the socket_return and addr_return values with the
 * created socket and the created address.
 * @param address the IPV4 address we want to use
 * @param port the port we want to use
 * @param socket_return the pointer where the created socket will be stored at
 * @param addr_return the address where the created sockaddr_in will be stored at
 * @return 0 if everything was successful; -1 if there was an error during socket creation
 */
int configure_connecting_socket(char* address, int port, int* socket_return, struct sockaddr_in *addr_return) {

    // creates a socket in the IPV4 domain using TCP protocol
    int server_socket = socket(PF_INET, SOCK_STREAM, 0);
    if (server_socket == -1) {
        perror("Erreur lors de la création de la socket serveur pour les messages.\n");
        return -1;
    }
    //printf("Socket serveur créée avec succès.\n");

    // server address configuration
    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET; // address type
    inet_pton(AF_INET, address, &(server_address.sin_addr)); //converts the address from the CLI to the correct format
    server_address.sin_port = htons(port); // address port (converted from the CLI)
    //printf("Adresse du serveur configurée avec succès ! (%s:%d)\n", address, port);

    *socket_return = server_socket;
    *addr_return = server_address;
    return 0;
}

/**
 * Connects the given address to the given socket.
 * @param socket the server's socket
 * @param address the server's address (ip + port)
 * @return
 */
int connect_on(int socket, struct sockaddr_in address) {
    // connection to the server (message socket)
    socklen_t server_address_len = sizeof(struct sockaddr_in);

    int connect_res = connect(socket, (struct sockaddr*) &address, server_address_len); // opens the socket with the configured address
    if (connect_res == -1) {
        perror("Erreur lors de la connexion au serveur.\n");
        return -1;
    }

    //printf("En attente de l'acceptation du serveur...\n");

    char welcome_message[MAX_MSG_SIZE];
    recv(socket, welcome_message, MAX_MSG_SIZE, 0);
    //printf("Message de bienvenue du serveur : %s\n", welcome_message);

    return 0;
}



/* * * * * * * * * * *
                        ROOMS
                              * * * * * * * * * * */

/**
 * @brief Manages the creation of a room on the client's side. This
 * function asks for the necessary information in order to create a room and
 * sends everything to the server.
 * @param socket the socket on which the information will be sent
 */
void client_room_creation(int socket) {
    print_title("Création du salon");
    printf("Entrez le nom du salon : (20 caractères max)\n");
    char name[20];
    fgets(name, 20, stdin);
    name[strcspn(name, "\n")] = 0;
    send(socket, name, 20, 0);

    printf("Entrez le nombre de membres max :\n");
    char nb_max_members[sizeof(int)];
    fgets(nb_max_members, sizeof(int), stdin);

    int max_members = atoi(nb_max_members);
    while (max_members>MAX_CLIENTS ){
      printf("Nombre de clients trop elevé, réessayez\n");
      fgets(nb_max_members, MAX_MSG_SIZE, stdin);
      max_members = atoi(nb_max_members);
    }

    send(socket, &max_members, sizeof(int), 0);

    // receives the response from the server
    char response[MAX_MSG_SIZE];
    recv(socket, response, MAX_MSG_SIZE, 0);
    printf("%s\n", response);
    print_separator(strlen("Création du salon"));
}

/**
 * @brief Prints the modifications that can be applied to a room.
 */
void print_room_modification_actions() {
    printf("Que voulez-vous modifier sur ce salon ?\n");
    printf(" - Le nom :                             1\n");
    printf(" - Le nb. max. de membres :             2\n");
}

/**
 * @brief Prints the actions that can be executed on a room.
 */
void print_room_actions() {
    printf("Que voulez-vous faire avec ce salon ?\n");
    printf(" - Rejoindre le salon :     /join\n");
    printf(" - Quitter le salon :       /leave\n");
    printf(" - Modifier le salon :      /modify\n");
    printf(" - Supprimer le salon :     /delete\n");
}

int is_string_a_number(char * saisie){
  if (isdigit(saisie[0]) != 0){
    return 1;
  }
  return 0;
}

/**
 * @brief Manages the modification of a room on the client's side. This
 * function asks for the necessary information in order to modify a room and
 * sends everything to the server.
 * @param socket the socket on which the information will be sent
 */
void client_room_modification(int socket, int choice) {
    print_title("Modification du salon");
    switch(choice) {
        case 1: {
            printf("Entrez le nom du salon : (20 caractères max)\n");
            char name[20];
            fgets(name, 20, stdin);
            name[strcspn(name, "\n")] = 0;
            send(socket, name, 20, 0);
            break;
        }
        case 2: {
            printf("Entrez le nombre de membres max :\n");
            char nb_max_members[sizeof(int)];
            fgets(nb_max_members, sizeof(int), stdin);

            int max_members = atoi(nb_max_members);
            while (max_members>MAX_CLIENTS ){
              printf("Nombre de clients trop elevé, réessayez\n");
              fgets(nb_max_members, MAX_MSG_SIZE, stdin);
               max_members = atoi(nb_max_members);
            }
            send(socket, &max_members, sizeof(int), 0);
            break;
        }
        default:
            printf("Veuillez entrer un nombre entre 1 et 2. Abandon de la modification.\n");
            break;
    }
    // receives the response from the server
    char response[MAX_MSG_SIZE];
    recv(socket, response, MAX_MSG_SIZE, 0);
    printf("%s\n", response);
    print_separator(strlen("Modification du salon"));
}

/**
 * @brief Gets the id of a room command.
 * @param command the executed room command
 * @return the id of the command
 */
int get_action_id(char* command) {
    if (strcmp(command, "/join\n") == 0) {
        return 0;
    }
    else if (strcmp(command, "/leave\n") == 0) {
        return 1;
    }
    else if (strcmp(command, "/modify\n") == 0) {
        return 2;
    }
    else if (strcmp(command, "/delete\n") == 0) {
        return 3;
    }
    else{
        return 0;
    }
}

/**
 * @brief displays the help of the various commands by reading the man.txt file.
 */
void print_man() {
    FILE* file;
    char * line = NULL;
    size_t len = 0;

    file = fopen("./man.txt", "r");

    print_title("Aide");
    while (getline(&line, &len, file) != -1) {
        printf("%s", line);
    }
    print_separator(strlen("Aide"));

    fclose(file);
}

/**
 * @brief prints a title inside a separator
 * @param title the title to print
 */
void print_title(char* title) {
    char separator[MAX_MSG_SIZE];
    strcpy(separator, "\n=================<");
    strcat(separator, title);
    strcat(separator, ">=================\n");
    printf("%s\n", separator);
}

/**
 * @brief prints a separator with a given size in the middle
 * @param middle_size the number of "=" that will be written inside the 2 "<>"
 */
void print_separator(int middle_size) {
    char separator[MAX_MSG_SIZE];
    strcpy(separator, "\n=================<");
    for (int i = 0; i < middle_size; i++) {
        strcat(separator, "=");
    }
    strcat(separator, ">=================");
    printf("%s\n", separator);
}
