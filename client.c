#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "comm_utils.h"

int main(int argc, char *argv[]) {

    /*
     * Socket setup
     */

    int socket_descriptor = socket(PF_INET, SOCK_STREAM, 0); // creates a TCP socket

    //server address configuration
    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET; // address type
    inet_pton(AF_INET, argv[1], &(server_address.sin_addr)); //converts the address from the CLI to the correct format
    server_address.sin_port = htons(atoi(argv[2])); // address port (converted from the CLI)

    socklen_t server_address_len = sizeof(struct sockaddr_in);
    connect(socket_descriptor, (struct sockaddr*) &server_address, server_address_len); // opens the socket with the configured address
    printf("Connexion au serveur réussie !\n");

    // receives the client id from the server
    int temp_client_id;
    recv(socket_descriptor, &temp_client_id, 4, 0);
    const int client_id = temp_client_id;
    printf("Votre ID de client boucle est : %d\n", client_id);


    /*
     * Discussion between clients
     */

    //int malloc_size = 32 * sizeof(char);
    char buffer[MAX_SIZE];

    while (1) {
        printf("Entrée dans la boucle while. (%d)\n", client_id);
        if (client_id == 1) {

            // Sends a message to the server
            printf("Mon message (1)(%d) : ", client_id);
            fgets(buffer, MAX_SIZE, stdin);
            send(socket_descriptor, buffer, MAX_SIZE + 1, 0);
            printf("Message envoyé (1)(%d) !\n", client_id);

            // Receives a message from the server
            printf("Attente d'un message du client 2. (%d)\n", client_id);
            recv(socket_descriptor, buffer, MAX_SIZE + 1, 0);
            //client_id = 1; // le client_id prend la valeur 2 tout seul comme un grand après le recv (POURQUOI????)
            printf("Réponse (1)(%d) : %s", client_id, buffer);


        } else if (client_id == 2) {

            // Receives a message from the server
            printf("Attente d'un message du client 1 (%d).\n", client_id);
            recv(socket_descriptor, buffer, MAX_SIZE + 1, 0);
            //client_id = 2; // le client_id prend la valeur 1 tout seul comme un grand après le recv (POURQUOI????)
            printf("Réponse (2)(%d) : %s", client_id, buffer);

            // Sends a message to the server
            printf("Mon message (2)(%d) : ", client_id);
            fgets(buffer, MAX_SIZE, stdin);
            send(socket_descriptor, buffer, MAX_SIZE + 1, 0);
            printf("Message envoyé (2)(%d) !\n", client_id);

        } else {
            printf("Mauvais id de client : %d\n", client_id);
            exit(-1);
        }
    }

    shutdown(socket_descriptor, 2);
}

