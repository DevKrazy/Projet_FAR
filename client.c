#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sys/sem.h>
#include "utils.h"

//pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
int clients[MAX_CLIENTS]; // list of connected clients
pthread_t thread[1];

void *send_thread(void *socket) {
    char send_buffer[MAX_SIZE];
    int server_socket = (int) (long) socket;
    printf("Entrez votre pseudo(max 10 lettres) : ");
    while (1) {
        //printf("Entrez votre message: ");
        //pthread_mutex_lock(&mutex);
        fgets(send_buffer, MAX_SIZE, stdin);
        send(server_socket, send_buffer, MAX_SIZE, 0);
        printf("[Vous] : %s", send_buffer);
        //pthread_mutex_unlock(&mutex);
    }

}

int main(int argc, char *argv[]) {

    // checks for the correct args number
    if (argc != 3) {
        printf("Nombre d'arguments incorrect. Utilisation :\n");
        printf("%s <adresse_ip_serveur> <num_port>\n", argv[0]);
        exit(0);
    } else {
        printf("Lancement du clients...\n");
    }


    /*
     * Socket setup
     */

    // creates a socket in the IPV4 domain using TCP protocol
    int server_socket = socket(PF_INET, SOCK_STREAM, 0);
    check_error(server_socket, "Erreur lors de la création de la socket serveur.\n");
    printf("Socket serveur créée avec succès.\n");

    // server address configuration
    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET; // address type
    inet_pton(AF_INET, argv[1], &(server_address.sin_addr)); //converts the address from the CLI to the correct format
    server_address.sin_port = htons(atoi(argv[2])); // address port (converted from the CLI)
    printf("Adresse du serveur configurée avec succès !\n");

    // connection to the server
    socklen_t server_address_len = sizeof(struct sockaddr_in);
    printf("En attente...\n");
    int connect_res = connect(server_socket, (struct sockaddr*) &server_address, server_address_len); // opens the socket with the configured address
    check_error(connect_res, "Erreur lors de la connexion au serveur.\n");
    char msgServeur[MAX_SIZE];
    recv(server_socket, msgServeur, MAX_SIZE, 0);
     printf("%s\n",msgServeur );

    //send pseudo to server
    //printf("Entrez votre pseudo :")
    //char send_pseudo[MAX_SIZE];
    //fgets(send_pseudo, MAX_SIZE, stdin);
    //send(server_socket, send_pseudo, MAX_SIZE, 0);

    // send thread start
    pthread_create(&thread[0], NULL, send_thread, (void *) (long) server_socket);

    while (1) {

        // main process = messages reception
        char recv_buffer[MAX_SIZE];
        //printf("Attente d'un message du serveur...\n");
        int recv_res = recv(server_socket, recv_buffer, MAX_SIZE, 0);
        printf("[Serveur] : %s", recv_buffer);
        if (recv_res == 0) {
            terminate_program(0);
        }
    }
}
