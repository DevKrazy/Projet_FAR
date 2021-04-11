#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "utils.h"

//pthread_mutex_t thread_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_t thread[1];

void *send_thread(void *socket) {
    char send_buffer[MAX_SIZE];
    int server_socket = (int) (long) socket;
    while (1) {
        //printf("Entrez votre message: ");
        fgets(send_buffer, MAX_SIZE, stdin);
        send(server_socket, send_buffer, MAX_SIZE, 0);
        printf("[Vous] : %s", send_buffer);
    }
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
    printf("Before connect\n");
    int connect_res = connect(server_socket, (struct sockaddr*) &server_address, server_address_len); // opens the socket with the configured address
    check_error(connect_res, "Erreur lors de la connexion au serveur.\n");
    printf("Connexion au serveur réussie !\n");

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


    /*
     * Communication with the server
     *//*

    // reception of the client id
    int client_id = 0;
    int recv_res = recv(server_socket, &client_id, sizeof(int), 0);
    check_error(recv_res, "Erreur lors de la réception de l'id client.\n");
    printf("Id client reçu : %d !\n", client_id);

    char buffer[MAX_SIZE];

    while (1) {

        if (client_id == 1) {

            printf("Entrez votre message : ");
            fgets(buffer, MAX_SIZE, stdin);
            send(server_socket, buffer, MAX_SIZE, 0);
            if (strcmp(buffer, END_WORD) == 0) {
               terminate_program(0);
            }
            printf("Message envoyé (1)(%d) !\n", client_id);

            printf("Attente d'un message du client 2. (1)(%d)\n", client_id);
            int recv1_res = recv(server_socket, buffer, MAX_SIZE, 0);
            printf("Réponse (1)(%d) : %s", client_id, buffer);
            if (recv1_res == 0) {
                terminate_program(0);
            }

        } else if (client_id == 2) {

            printf("Attente d'un message du client 1. (2)(%d)\n", client_id);
            int recv2_res = recv(server_socket, buffer, MAX_SIZE, 0);
            printf("Réponse (2)(%d) : %s", client_id, buffer);
            if (recv2_res == 0) {
                terminate_program(0);
            }

            printf("Entrez votre message : ");
            fgets(buffer, MAX_SIZE, stdin);
            send(server_socket, buffer, MAX_SIZE, 0);
            if (strcmp(buffer, END_WORD) == 0) {
                terminate_program(0);
            }
            printf("Message envoyé (2)(%d) !\n", client_id);
        } else {
            printf("ID de client incorrect (%d).\n", client_id);
            terminate_program(-1);
        }
    }*/
}