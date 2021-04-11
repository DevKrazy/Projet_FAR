#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include "headers/utils.h"

//pthread_mutex_t thread_mutex = PTHREAD_MUTEX_INITIALIZER;
int clients[MAX_CLIENTS]; // list of connected clients
pthread_t thread[MAX_THREADS];
int client_index;

// TODO: faire une fonction send et recv qui envoie et recoivent le nb d'octets à transférer
// TODO: gérer le mutex

void *send_thread(void *socket) {
    printf("Thread clients créé !\n");
    char send_buffer[MAX_SIZE];
    int client_socket = (int) (long) socket;
    while (1) {
        int recv_res = recv(client_socket, send_buffer, MAX_SIZE, 0);
        if (recv_res == 0 || strcmp(send_buffer, "fin\n") == 0) {
            for (int l = 0; l < MAX_CLIENTS; l++) {

                // frees the clients array from the clients who disconnected
                if (client_socket == clients[l]) {
                    clients[l] = 0; // clients reset
                    client_index-=1;
                    shutdown(client_socket,2);
                    close(client_socket);
                    pthread_exit(0);
                }
            }
        }
        printf("Reçu : %s", send_buffer);
        for (int j = 0; j < MAX_CLIENTS; j++) {
            printf("clients j : %d\n", clients[j]);
            if (clients[j] != client_socket && clients[j] != 0) {
                send(clients[j], send_buffer, MAX_SIZE, 0); // modifié le j en clients[j]
                printf("Envoyé au clients : %s", send_buffer);
            } else {
                printf("On n'envoie pas\n");
            }
        }
    }
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


    /*
     * Server socket setup
     */

    // creates a socket in the IPV4 domain using TCP protocol
    int server_socket = socket(PF_INET, SOCK_STREAM, 0);
    check_error(server_socket, "Erreur lors de la création de la socket serveur.\n");
    printf("Socket serveur créée avec succès.\n");

    // server address configuration
    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET; // address type
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons(atoi(argv[1])); // address port (converted from the CLI)

    // bind
    int bind_res = bind(server_socket, (struct sockaddr*) &server_address, sizeof(server_address)); // binds address to server socket
    check_error(bind_res, "Erreur lors du bind\n");
    printf("Bind réussi !\n");

    // listen
    int listen_res = listen(server_socket, MAX_CLIENTS); // listens for incoming connections (maximum 2 waiting connections)
    check_error(listen_res, "Erreur lors du listen\n");
    printf("Le serveur écoute sur le port %s.\n", argv[1]);

    client_index = 0;

    while (1) {
        // clients address initialization
        struct sockaddr_in client_address;
        socklen_t client_address_len = sizeof(struct sockaddr_in);

        // accept
        if (client_index==MAX_CLIENTS){
            printf("Nombre de clients maximum atteint !\n");
            int client_socket = accept(server_socket, (struct sockaddr *) &client_address, &client_address_len);
            send(client_socket, "trop de clients aurevoir", MAX_SIZE, 0);
            shutdown(client_socket, 2) ;

        }
        else{
            for (int k = 0; k < MAX_CLIENTS; k++) {
                if (clients[k] == 0) { // 0 means clients not connected
                    int client_socket = accept(server_socket, (struct sockaddr *) &client_address, &client_address_len);
                    if (client_socket != -1){
                        pthread_create(&thread[client_index], NULL, send_thread, (void *) (long) client_socket);
                        clients[client_index]= (int) client_socket;
                        client_index+=1;
                        printf("Un clients connecté de plus ! %d clients \n", client_index);
                    }
                    // break;
                }
            }
        }

    }
}