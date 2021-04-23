#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>
#include <inttypes.h> // to print types such as in_port_t (which is a uint16_t)
#include "utils/headers/utils.h"
#include "utils/headers/server_utils.h"

// TODO: quand on reçoit un mp : remplacer "server" par le pseudo
// TODO: utiliser des puts plutot que des printf
// TODO: corriger le print du port

sem_t semaphore;
Client clients[MAX_CLIENTS];

void *messaging_thread(void *socket) {
    printf("Thread clients créé !\n");
    char send_buffer[MAX_MSG_SIZE];
    int client_socket = (int) (long) socket;
    int client_index = get_index_by_socket(clients, client_socket);

    // receives and adds the client's name to its structure
    recv(client_socket, send_buffer, MAX_MSG_SIZE, 0);
    strcpy(clients[client_index].pseudo, send_buffer);

    while (1) {
        int recv_res = recv(client_socket, send_buffer, MAX_MSG_SIZE, 0);
        //check_error(-1, "Erreur lors de la réception du message du client.\n");

        // Checks if the client closed the discussion or the program
        if (recv_res == 0 || strcmp(send_buffer, "fin\n") == 0) {
            clients[client_index].client_socket = 0; // client_socket reset
            sem_post(&semaphore);
            shutdown(client_socket, 2);
            pthread_exit(0);
        }

        //send message
        if (is_private_message(send_buffer, clients)==1) {
            send_message_to(send_buffer,clients);
        } else {
            broadcast_message(send_buffer, clients, client_index);
        }
    }
}

/**
 * Configures the server's socket and returns it.
 * @param port the listening port
 * @return the server socket
 */
int configure_server_socket(int port) {

    // creates a socket in the IPV4 domain using TCP protocol
    int server_socket = socket(PF_INET, SOCK_STREAM, 0);
    check_error(server_socket, "Erreur lors de la création de la socket serveur.\n");
    printf("Socket serveur créée avec succès.\n");

    // server address configuration
    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET; // address type
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons(port); // address port (converted from the CLI)
    //server_address.sin_port = htons(atoi(argv[1])); // address port (converted from the CLI)

    // bind
    int bind_res = bind(server_socket, (struct sockaddr*) &server_address, sizeof(server_address)); // binds address to server socket
    check_error(bind_res, "Erreur lors du bind\n");
    printf("Bind réussi !\n");

    // listen
    int listen_res = listen(server_socket, MAX_CLIENTS); // listens for incoming connections (maximum 2 waiting connections)
    check_error(listen_res, "Erreur lors du listen\n");
    printf("Le serveur écoute sur le port %d\n", port);
    //printf("Le serveur écoute sur le port %s.\n", argv[1]);

    return server_socket;
}


int main(int argc, char *argv[]) {

    // checks for the correct args number
    if (argc != 2) {
        printf("Nombre d'arguments incorrect. Utilisation :\n");
        printf("%s <num_port>\n", argv[0]);
        exit(0);
    } else {
        printf("Lancement du serveur...\n");
    }

    // sem init
    sem_init(&semaphore, PTHREAD_PROCESS_SHARED, MAX_CLIENTS);
    //check_error(-1, "Erreur lors de l'initialisation du semaphore.\n");
    int server_socket = configure_server_socket(atoi(argv[1]));
    int server_socket_file = configure_server_socket(atoi(argv[1]) + 1);
    int sem_value;

    while (1) {
        // clients address initialization
        struct sockaddr_in client_address;
        socklen_t client_address_len = sizeof(struct sockaddr_in);

        sem_getvalue(&semaphore, &sem_value);
        int sem_wait_res = sem_wait(&semaphore); // decrements the semaphore, waits if it is 0
        check_error(sem_wait_res, "Erreur lors du sem_wait.\n");

        int client_socket = accept(server_socket, (struct sockaddr *) &client_address, &client_address_len);
        check_error(client_socket, "Erreur lors de l'acceptation du client.\n");
        send(client_socket, "Connexion acceptée\n", MAX_MSG_SIZE, 0);


        // puts
        for (int k = 0; k < MAX_CLIENTS; k++) {
            if (clients[k].client_socket == 0) {
                // we found a client not connected
                clients[k].client_socket = client_socket;
                pthread_create(&clients[k].msg_thread, NULL, messaging_thread, (void *) (long) client_socket);
                printf("Un clients connecté de plus ! %d client(s)\n", get_client_count(semaphore));
                break;
            }
        }
    }
}