#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

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
    int client_id;
    recv(socket_descriptor, &client_id, 4, 0);
    printf("Votre ID de client boucle est : %d\n", client_id);


    /*
     * Discussion between clients
     */

    int malloc_size = 32 * sizeof(char);
    char *buffer;

    while (1) {
        if (client_id == 1) {

            // Sends a message to the server
            buffer = malloc(malloc_size);
            printf("Mon message : ");
            fgets(buffer, malloc_size, stdin);
            send(socket_descriptor, buffer, malloc_size + 1, 0);
            free(buffer);

            // Receives a message from the server
            buffer = malloc(malloc_size);
            recv(socket_descriptor, buffer, 32 + 1, 0);
            printf("Réponse : %s", buffer);
            free(buffer);


        } else if (client_id == 2) {

            // Receives a message from the server
            buffer = malloc(malloc_size);
            recv(socket_descriptor, buffer, 32 + 1, 0);
            printf("Réponse : %s", buffer);
            free(buffer);

            // Sends a message to the server
            buffer = malloc(malloc_size);
            printf("Mon message : ");
            fgets(buffer, malloc_size, stdin);
            send(socket_descriptor, buffer, malloc_size + 1, 0);
            free(buffer);

        } else {
            printf("Mauvais id de client : %d\n", client_id);
            exit(-1);
        }
    }

    shutdown(socket_descriptor, 2);
}

