#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
//#include "comm_utils.h"
#include <pthread.h>
#define NB_THREADS 60
#define MAX_SIZE 10000
#define MAX_CLIENTS 2

pthread_mutex_t thread_mutex = PTHREAD_MUTEX_INITIALIZER;
char buffer[MAX_SIZE];
/*Global variable */
int clients[MAX_CLIENTS]; //listes des clients connectés

/*receiving message from a clients and sending them to other clients*/
void *Sendthread (void* client_socket_descriptor_void){
    long client_socket_descriptor = (long) client_socket_descriptor_void;
    //pthread_mutex_lock(&thread_mutex);
    recv((int) client_socket_descriptor, buffer, MAX_SIZE + 1, 0); //reception message
    printf("Reçu du clients 1 : %s", buffer);
    for(int j=0; j < MAX_CLIENTS; j++)
    {
        if(clients[j] != client_socket_descriptor){
            send(j, buffer, 4, 0);
            printf("Envoyé au clients : %s", buffer);
        }else {
            printf("Client_ID");
        }
    }
    pthread_exit(0);
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
    printf("Le serveur écoute sur le port %s.\n", argv[1]);


    int i =0;
    while(i<MAX_CLIENTS){
        printf("Boucle while\n");
        struct sockaddr_in client_address;
        socklen_t client_address_len = sizeof(struct sockaddr_in);
        long client_socket_descriptor = accept(socket_descriptor, (struct sockaddr*) &client_address, &client_address_len);
        printf("Un clients de plus !\n");
        if (client_socket_descriptor!=-1){
            pthread_create(&thread[i],NULL,Sendthread,(void *)client_socket_descriptor);
            clients[i]= (int) client_socket_descriptor;
        }
        i++;
    }
}