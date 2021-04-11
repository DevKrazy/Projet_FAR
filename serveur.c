#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "comm_utils.h"

int main(int argc, char *argv[]) {

    /*
     * Sockets setup
     */

    int socket_descriptor = socket(PF_INET, SOCK_STREAM, 0);

    // server address configuration
    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET; // address type
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons(atoi(argv[1])); // address port (converted from the CLI)

    bind(socket_descriptor, (struct sockaddr*)&server_address, sizeof(server_address)); // binds the configured address to the socket descriptor
    listen(socket_descriptor, 7); // listens for incoming connections
    printf("Le serveur malloc écoute sur le port %s.\n", argv[1]);

    // client1 address config
    struct sockaddr_in client1_address;
    socklen_t client1_address_len = sizeof(struct sockaddr_in);
    // if accept() is successful, creates a new socket descriptor already connected
    int client1_socket_descriptor = accept(socket_descriptor, (struct sockaddr*) &client1_address, &client1_address_len);

    int client_id1 = 1;
    send(client1_socket_descriptor, &client_id1, 4, 0);
    printf("ID envoyé au clients 1.\n");


    // client2 address config
    struct sockaddr_in client2_address;
    socklen_t client2_address_len = sizeof(struct sockaddr_in);
    // if accept() is successful, creates a new socket descriptor already connected
    int client2_socket_descriptor = accept(socket_descriptor, (struct sockaddr*) &client2_address, &client2_address_len);

    int client_id2 = 2;
    send(client2_socket_descriptor, &client_id2, 4, 0);
    printf("ID envoyé au clients 2.\n");


    /*
     * Discussion between clients
     */

    //int malloc_size = 32 * sizeof(char);
    char buffer[MAX_SIZE];

    while (strcmp(buffer, "fin\n") != 0) {

        // Reception from clients 1
        recv(client1_socket_descriptor, buffer, MAX_SIZE + 1, 0); //reception message clients 1
        printf("Reçu du clients 1 : %s", buffer);

        // Sends the message to clients 2
        send(client2_socket_descriptor, buffer, MAX_SIZE + 1, 0);
        printf("Envoyé au clients 2 : %s", buffer);

        // Reception from clients 2
        recv(client2_socket_descriptor, buffer, MAX_SIZE + 1, 0); //reception message 2e clients
        printf("Reçu du clients 2: %s", buffer);

        // Sends the message to clients 1
        send(client1_socket_descriptor, buffer, MAX_SIZE + 1, 0); //send to clients 1
        printf("Envoyé au clients 1 : %s", buffer);
    }

    /*
     * Sockets shutdown
     */
    shutdown(client2_socket_descriptor, 2) ;
    shutdown(client1_socket_descriptor, 2) ;


    close(client2_socket_descriptor);
    close(client1_socket_descriptor);
    shutdown(socket_descriptor, 2) ;
    close(socket_descriptor);
}
