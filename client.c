#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "utils/headers/utils.h"
#include "utils/headers/server_utils.h"

pthread_t msg_thread;

/**
 * The client's messaging thread.
 * @param socket the server socket
 */
void *messaging_thread(void *socket) {
    char send_buffer[MAX_MSG_SIZE];
    int server_socket = (int) (long) socket;

    // gets and sends the client's name to the server
    printf("Entrez votre pseudo (max 10 lettres) : ");
    fgets(send_buffer, MAX_MSG_SIZE, stdin);
    send_buffer[strcspn(send_buffer, "\n")] = 0; // removes the \n at the end
    send(server_socket, send_buffer, MAX_MSG_SIZE, 0);
    printf("Bienvenue %s !\n", send_buffer);

    while (1) {
        fgets(send_buffer, MAX_MSG_SIZE, stdin);
        send(server_socket, send_buffer, MAX_MSG_SIZE, 0);
        printf("[Vous] : %s", send_buffer);
    }
}

 /**
  * Configures the server's socket and updates the socket_return and addr_return values with the
  * created socket and the created address.
  * @param address the IPV4 address we want to use
  * @param port the port we want to use
  * @param socket_return the pointer where the created socket will be stored at
  * @param addr_return the address where the created sockaddr_in will be stored at
  * @return 0 if everything was successful; -1 if there was an error during socket creation
  */
int configure_server_socket(char* address, char* port, int* socket_return, struct sockaddr_in *addr_return) {

    // creates a socket in the IPV4 domain using TCP protocol
    int server_socket = socket(PF_INET, SOCK_STREAM, 0);
    if (server_socket == -1) {
        printf("Erreur lors de la création de la socket serveur pour les messages.\n");
        return -1;
    }
    printf("Socket serveur créée avec succès.\n");

    // server address configuration
    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET; // address type
    inet_pton(AF_INET, address, &(server_address.sin_addr)); //converts the address from the CLI to the correct format
    server_address.sin_port = htons(atoi(port)); // address port (converted from the CLI)
    printf("Adresse du serveur configurée avec succès !\n");

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
        printf("Erreur lors de la connexion au serveur.\n");
        return -1;
    }
    printf("En attente de l'acceptation du serveur...\n");
    return 0;
}

int main(int argc, char *argv[]) {

    // checks for the correct args number
    if (argc != 3) {
        printf("Nombre d'arguments incorrect. Utilisation :\n");
        printf("%s <adresse_ip_serveur> <num_port>\n", argv[0]);
        exit(0);
    } else {
        printf("Lancement du client...\n");
    }

    // configures the server's socket and connects to it
    int server_msg_socket;
    struct sockaddr_in server_msg_address;
    configure_server_socket(argv[1], argv[2], &server_msg_socket, &server_msg_address);
    connect_on(server_msg_socket, server_msg_address);

    // receives the connection confirmation message from the server
    char recv_buffer[MAX_MSG_SIZE];
    recv(server_msg_socket, recv_buffer, MAX_MSG_SIZE, 0);
    printf("%s",recv_buffer );

    // starts the messaging thread
    pthread_create(&msg_thread, NULL, messaging_thread, (void *) (long) server_msg_socket);

    while (1) {
        int recv_res = recv(server_msg_socket, recv_buffer, MAX_MSG_SIZE, 0);
        printf("[Serveur] : %s\n", recv_buffer);
        if (recv_res == 0) {
            // the server closed the connection
            terminate_program(0);
        }
    }
}