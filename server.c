#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include "utils.h"


// fonction pour send un message avec le pseudo 
// fonction pour recv un emssage avec le pseudo
// fonction pour récup un client à partir de son socket
// 


//int clients[MAX_CLIENTS]; // list of connected clients
pthread_t thread[MAX_THREADS];
sem_t semaphore;
int client_index;
char clients_pseudo[MAX_CLIENTS];

// TODO: gérer le mutex
struct Client {
  int client_socket;
  char pseudo[10];
};

struct Client clients[MAX_CLIENTS]; 

void *send_thread(void *socket) {
    printf("Thread clients créé !\n");
    char send_buffer[MAX_SIZE];
    int client_socket = (int) (long) socket;
    
    // ajout du pseudo 
    recv(client_socket, send_buffer, MAX_SIZE, 0);
    for(int i = 0; i<MAX_CLIENTS; i++){
      if (clients[i].client_socket == client_socket){
        strcpy(clients[i].pseudo, send_buffer);
        printf("pseudo enregistré : %s\n", clients[i].pseudo);
        }
    }
    
    
    while (1) {
        int recv_res = recv(client_socket, send_buffer, MAX_SIZE, 0);
        if (recv_res == 0 || strcmp(send_buffer, "fin\n") == 0) { // if the client stopped the discussion
            for (int l = 0; l < MAX_CLIENTS; l++) {

                // frees the clients array from the clients who disconnected
                if (client_socket == clients[l].client_socket) {
                  // TODO : modifier la structure
                    clients[l].client_socket = 0; // clients reset
                    client_index -= 1;
                    sem_post(&semaphore);
                    shutdown(client_socket, 2);
                    pthread_exit(0);
                }
            }
        }
        printf("Reçu : %s", send_buffer);
        for (int j = 0; j < MAX_CLIENTS; j++) { // pour tous les clients du tableau
            printf("clients j : %d\n", clients[j].client_socket);
            if (clients[j].client_socket != client_socket && clients[j].client_socket != 0) { // envoi 
                send(clients[j].client_socket, send_buffer, MAX_SIZE, 0); // modifié le j en clients[j]
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

    // sem init
    sem_init(&semaphore, PTHREAD_PROCESS_SHARED, MAX_CLIENTS);
    // TODO: check return value

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
    int sem_value;

    while (1) {
      // clients address initialization
      struct sockaddr_in client_address;
      socklen_t client_address_len = sizeof(struct sockaddr_in);
     
      // accept
      sem_getvalue(&semaphore,&sem_value);
      printf("Before : sem_value: %d\n",sem_value);

      int sem_wait_res = sem_wait(&semaphore); // decrements the sem, waits if it is 0
      check_error(sem_wait_res, "Erreur lors du sem_wait.\n");

      int client_socket = accept(server_socket, (struct sockaddr *) &client_address, &client_address_len);
      send(client_socket, "Connexion acceptée\n", MAX_SIZE, 0);


      // met la socket client dans le tableau au premier 0 disponible
      for (int k = 0; k < MAX_CLIENTS; k++) {
        if (clients[k].client_socket == 0) { // 0 means clients not connected
            clients[k].client_socket = client_socket;
            break;
          }
      }   

      // creation du thread client
      if (client_socket != -1) {
             pthread_create(&thread[client_index], NULL, send_thread, (void *) (long) client_socket);
             client_index+=1;
             printf("Un clients connecté de plus ! %d clients \n", client_index);
        }
  }
}
