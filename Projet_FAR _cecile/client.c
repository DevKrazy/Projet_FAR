#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "utils/headers/utils.h"
#include "utils/headers/server_utils.h"
#include <unistd.h>
#include <limits.h>
#include <dirent.h>


//pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
int clients[MAX_CLIENTS]; // list of connected clients
pthread_t thread;
char contenu_fichier[MAX_FILE_SIZE];

void envoi_fichier(){
    FILE* fp1 = new_tty();
          fprintf(fp1,"%s\n","Ce terminal sera utilisé uniquement pour l'affichage");

          // Demander à l'utilisateur quel fichier afficher
          DIR *dp;
          struct dirent *ep;     
          dp = opendir ("./");
          if (dp != NULL) {
            fprintf(fp1,"Voilà la liste de fichiers :\n");
            while (ep = readdir (dp)) {
              if(strcmp(ep->d_name,".")!=0 && strcmp(ep->d_name,"..")!=0) 
            fprintf(fp1,"%s\n",ep->d_name);
            }    
            (void) closedir (dp);
          }
          else {
            perror ("Ne peux pas ouvrir le répertoire");
          }
          printf("Indiquer le nom du fichier : ");
          char fileName[1023];
          fgets(fileName,sizeof(fileName),stdin);
          fileName[strlen(fileName)-1]='\0';
          FILE *fps = fopen(fileName, "r");
          if (fps == NULL){
            printf("Ne peux pas ouvrir le fichier suivant : %s",fileName);
          }
          else {
            char str[1000];    
            // Lire et afficher le contenu du fichier
            while (fgets(str, 1000, fps) != NULL) {
              fprintf(fp1,"%s",str);
              strcat(contenu_fichier,str);

            }
            //printf("contenu fichier : %s\n", contenu_fichier );
          }
          fclose(fps);  
}


void *client_message_thread(void *socket) {
    char send_buffer[MAX_MSG_SIZE];
    int server_socket = (int) (long) socket;

    // gets and sends the client's name to the server
    printf("Entrez votre pseudo(max 10 lettres) : ");
    fgets(send_buffer, MAX_MSG_SIZE, stdin);
    send_buffer[strcspn(send_buffer, "\n")] = 0; // removes the \n at the end
    send(server_socket, send_buffer, MAX_MSG_SIZE, 0);
    printf("Bienvenue %s !\n", send_buffer);

    while (1) {
        //pthread_mutex_lock(&mutex);
        fgets(send_buffer, MAX_MSG_SIZE, stdin);
        //cas ou faut envoyer un fichier
        if (strcmp("file\n", send_buffer)==0){
            
        //pthread_mutex_unlock(&mutex);
    }
    else{
        send(server_socket, send_buffer, MAX_MSG_SIZE, 0);
        printf("[Vous] : %s", send_buffer);
    }
    }
}

void *client_file_thread(void *socket) {
    char send_buffer[MAX_FILE_SIZE];
    int server_socket_file = (int) (long) socket;
    
}


int  create_server_socket(char *argv[],int num_port){
        // creates a socket in the IPV4 domain using TCP protocol
    int server_socket = socket(PF_INET, SOCK_STREAM, 0);
    check_error(server_socket, "Erreur lors de la création de la socket serveur.\n");
    printf("Socket serveur créée avec succès sur le port %d.\n",num_port);

    // server address configuration
    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET; // address type
    inet_pton(AF_INET, argv[1], &(server_address.sin_addr)); //converts the address from the CLI to the correct format
    server_address.sin_port = htons(num_port); // address port (converted from the CLI)
    printf("Adresse du serveur configurée avec succès !\n");

    // connection to the server
    socklen_t server_address_len = sizeof(struct sockaddr_in);
    int connect_res = connect(server_socket, (struct sockaddr*) &server_address, server_address_len); // opens the socket with the configured address
    printf("En attente de l'acceptation du serveur...\n");
    check_error(connect_res, "Erreur lors de la connexion au serveur.\n");
    char msgServeur[MAX_MSG_SIZE];
    recv(server_socket, msgServeur, MAX_MSG_SIZE, 0);
    printf("%s\n",msgServeur );


    // send thread start
    //pthread_create(&thread, NULL, client_message_thread, (void *) (long) server_socket);

    return server_socket;
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

    int server_socket_file=create_server_socket(argv,atoi(argv[2])+1);
    // send thread start
    pthread_create(&thread, NULL, client_file_thread, (void *) (long) server_socket_file);

    int server_socket=create_server_socket(argv,atoi(argv[2]));
    // send thread start
    pthread_create(&thread, NULL, client_message_thread, (void *) (long) server_socket);

    while (1) {

        // main process = messages reception
        char recv_buffer[MAX_MSG_SIZE];
        //printf("Attente d'un message du serveur...\n");
        int recv_res = recv(server_socket, recv_buffer, MAX_MSG_SIZE, 0);
        printf("[Serveur] : %s\n", recv_buffer);
        if (recv_res == 0) {
            terminate_program(0);
        }
    }
}