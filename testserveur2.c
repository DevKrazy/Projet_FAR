#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "comm_utils.h"
#include <pthread.h>
#define NB_THREADS 60;

pthread_mutex_t thread_mutex = PTHREAD_MUTEX_INITIALIZER;
char buffer[MAX_SIZE];
/*Global variable */
int client[10]; //listes des clients connectés 

/*sending messages to client*/
void *Sendthread (int client_id){
    //pthread_mutex_lock(&thread_mutex);
    recv(client_id, &buffer, MAX_SIZE + 1, 0); //reception message 
    printf("Reçu du client 1 : %s", buffer);
    send(client_id, &buffer, 4, 0);
    printf("Envoyé au client 2 : %s", buffer);
}




int main(int argc, char *argv[]) {

    pthread_t thread[NB_THREADS];
    int socket_descriptor = socket(PF_INET, SOCK_STREAM, 0);

    // server address configuration
    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET; // address type
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons(atoi(argv[1])); // address port (converted from the CLI)

    bind(socket_descriptor, (struct sockaddr*)&server_address, sizeof(server_address)); // binds the configured address to the socket descriptor
    listen(socket_descriptor, 10); // listens for incoming connections
    printf("Le serveur malloc écoute sur le port %s.\n", argv[1]);

 
    int i =0;
    while(i<10){

        struct sockaddr_in client_address;
        socklen_t client_address_len = sizeof(struct sockaddr_in);
        int client_socket_descriptor = accept(socket_descriptor, (struct sockaddr*) &client_address, &client_address_len);
        if (client_socket_descriptor!=-1){
            pthread_create(&thread[i],NULL,Sendthread,(void *)client_socket_descriptor);
        }
        i++;
    }

    // client2 address config
    struct sockaddr_in client2_address;
    socklen_t client2_address_len = sizeof(struct sockaddr_in);
    // if accept() is successful, creates a new socket descriptor already connected
    int client2_socket_descriptor = accept(socket_descriptor, (struct sockaddr*) &client2_address, &client2_address_len);

    int client_id2 = 2;
    send(client2_socket_descriptor, &client_id2, 4, 0);
    printf("ID envoyé au client 2.\n");


    /*
     * Discussion between clients
     */

    //int malloc_size = 32 * sizeof(char);
    char buffer[MAX_SIZE];

    while (strcmp(buffer, "fin\n") != 0) {

        // Reception from client 1
        recv(client1_socket_descriptor, buffer, MAX_SIZE + 1, 0); //reception message client 1
        printf("Reçu du client 1 : %s", buffer);

        // Sends the message to client 2
        send(client2_socket_descriptor, buffer, MAX_SIZE + 1, 0);
        printf("Envoyé au client 2 : %s", buffer);

        // Reception from client 2
        recv(client2_socket_descriptor, buffer, MAX_SIZE + 1, 0); //reception message 2e client
        printf("Reçu du client 2: %s", buffer);

        // Sends the message to client 1
        send(client1_socket_descriptor, buffer, MAX_SIZE + 1, 0); //send to client 1
        printf("Envoyé au client 1 : %s", buffer);
    }

    /*
     * Sockets shutdown
     */
    shutdown(client2_socket_descriptor, 2) ;
    shutdown(client1_socket_descriptor, 2) ;
    shutdown(socket_descriptor, 2) ;

    close(client2_socket_descriptor);
    close(client1_socket_descriptor);
    close(socket_descriptor);
}
