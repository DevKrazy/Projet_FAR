#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#define NB_THREADS 60
#define MAX_SIZE 10000
#define MAX_CLIENTS 2
#include "../comm_utils.h"
#include "../remake/utils.h"

//pthread_mutex_t thread_mutex = PTHREAD_MUTEX_INITIALIZER;

void *Sendthread (void* socket_descriptor_void) {
    // on cast on long parce que void a la même taille
    long socket_descriptor = (long) socket_descriptor_void;
    char buffer_send[MAX_SIZE];
    while (strcmp(buffer_send, "fin\n") != 0){
        fgets(buffer_send, MAX_SIZE, stdin);
        send(socket_descriptor, buffer_send, 4, 0);
        printf("Envoyé au client: %s", buffer_send);
    }
    pthread_exit(0);
}

int main(int argc, char *argv[]) {

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

    // connection to the server
    socklen_t server_address_len = sizeof(struct sockaddr_in);
    int connect_res = connect(server_socket, (struct sockaddr*) &server_address, server_address_len); // opens the socket with the configured address
    check_error(connect_res, "Erreur lors de la connexion au serveur.\n");
    printf("Connexion au serveur réussie !\n");

    /*
     * Discussion between clients
     */

    char buffer_recv[MAX_SIZE];
    pthread_t thread[1];
    pthread_create(&thread[0], NULL, Sendthread, (void *)server_socket);

    while (strcmp(buffer_recv, "fin\n") != 0) {
        printf("Entrée dans la boucle while.\n");
        recv((int) server_socket, buffer_recv, MAX_SIZE + 1, 0); //reception message
        printf("Reçu du serveur : %s", buffer_recv);

    }
    shutdown((int) server_socket, 2);
}