#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>
#include <dirent.h>
#include <fcntl.h>
#include "utils/headers/utils.h"
#include "utils/headers/server_utils.h"


// TODO: utiliser des puts plutot que des printf
// TODO: corriger le retour a la ligne en trop quand on reçoit un message (ça vient du strtok surement)

sem_t semaphore;
//sem_t file_semaphore;
Client clients[MAX_CLIENTS];
Room rooms[NB_MAX_ROOM];
char file_content[MAX_FILE_SIZE];
char fileName[MAX_MSG_SIZE];
int file_size;
//pour enregitrer les fichiers dans le serveur
int server_file_socket;
struct sockaddr_in server_file_address;
int client_file_receiving_socket;

//pour envoyer les fichiers au client
int server_send_file_socket;
struct sockaddr_in server_send_file_address;

void* file_sending_thread_func(void* socket);
void* file_receiving_thread_func(void* socket);
void* room_thread_func(void* socket);

void* room_thread_func(void* socket){
  int client_socket = (int) (long) socket;
  for(int m =0; m<NB_MAX_ROOM;m++){ 
  }

  
}


void* file_receiving_thread_func(void* socket) {
    int client_socket = (int) (long) socket;
    char fileName[MAX_MSG_SIZE];
    int size;
    char* file_content = NULL;

        int recv1 = recv(client_socket, fileName, MAX_MSG_SIZE, 0);
        if (recv1 == 0) {
            shutdown(client_socket, 2);
            pthread_exit(0);
        }
        int recv2 = recv(client_socket, &size, sizeof(int), 0);
        if (recv2 == 0) {
            shutdown(client_socket, 2);
            pthread_exit(0);
        }
        file_content = malloc(size);
        int recv3 = recv(client_socket,file_content, size , 0);
        if (recv3 == 0) {
            shutdown(client_socket, 2);
            pthread_exit(0);
        }

        // saves the file
        char folder[200] = "./recv/";
        strcat(folder, fileName);
        int fp = open(folder,  O_WRONLY | O_CREAT, S_IRWXU);
        printf("RECEIVED FILE CONTENT FROM CLIENT\n");
        printf("%s\n", file_content);
        write(fp, file_content, size);
        close(fp);
        free(file_content);
        pthread_exit(0);
}

/**
 * The server's messaging thread.
 * @param socket the server's socket
 * @return
 */

void *messaging_thread_func(void *socket) {
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
            clients[client_index].client_msg_socket = 0; // client_msg_socket reset
            sem_post(&semaphore);
            shutdown(client_socket, 2);
            pthread_exit(0);
        }
        if (strcmp(send_buffer, "file\n")==0)
        {
            client_file_receiving_socket = accept_client(server_file_socket);
            pthread_create(&clients[client_index].file_receiving_thread, NULL, file_receiving_thread_func, (void *) (long) client_file_receiving_socket);
            
        }

        else if (strcmp(send_buffer, "filesrv\n") == 0) {
           printf("IN FILESRV\n");
           char* file_list = malloc(1); // malloc(1) because list_files reallocate the memory
           list_files(SERVER_DIR, &file_list);
           printf("LISTE DE FICHIERS : \n %s", file_list);
           //printf("Before send file_list\n");
           send(client_socket,file_list,MAX_MSG_SIZE, 0); // envoi list des fichiers
           //printf("After send file_list\n");

            recv(client_socket, fileName, MAX_MSG_SIZE, 0); // reception du nom du fichier
            //Ptit problème !
            server_send_file_socket = accept_client(server_send_file_socket);
            pthread_create(&clients[client_index].file_sending_thread, NULL, file_sending_thread_func, (void *) (long) server_send_file_socket);
        }else if (strcmp(send_buffer, "room\n") == 0) {
          //envoyer la liste au client
          char* listRooms= malloc(MAX_MSG_SIZE); 
          list_Rooms(&rooms[NB_MAX_ROOM], &listRooms);
          printf("LISTE DES ROOMS : \n %s", listRooms);
          send(client_socket,listRooms,MAX_MSG_SIZE, 0); // envoi list des rooms (nom+port)
          int num_room= recv(faut que le client envoie qq chose qui nous permettent de retrouver l'indice du salon);
          //accept client faut mettre condition si taille max atteinte
          clients[client_index].client_room_socket = accept_client(rooms[num_room].socket_room_server);
          pthread_create(&clients[client_index].room_thread, NULL, room_thread_func, (void *) (long)clients[client_index].client_room_socket);

        } else if (is_private_message(send_buffer, clients) == 1) {
            send_message_to(send_buffer, clients,client_socket);
        } else {
            broadcast_message(send_buffer, clients, client_index);
        }

    }
}



void* file_sending_thread_func(void* socket) {
    int client_socket = (int) (long) socket;
        int len = (int) strlen(file_content);

        char folder[200] = SERVER_DIR;
        strcat(folder, fileName);
        printf("Server is trying to open file: %s\n", folder);
        int fp = open(folder, O_RDONLY);
        int size=0;
        if (fp == -1){
            printf("Ne peux pas ouvrir le fichier suivant : %s\n", fileName);
            perror("Erreur lors de l'ouverture du fichier");
        } else {

            size = read(fp, file_content, MAX_FILE_SIZE);
            file_content[size] = 0;
            printf("size %d\n",size);

        }
        close(fp);

        send(client_socket, &size, sizeof(int), 0); // envoi taille fichier
        printf("taille envoyée\n");
        send(client_socket, file_content,size, 0); // envoi du contenu du fichier
        printf("file_content: %s\n",file_content);
        printf("Fichier envoyé !\n");
        bzero(file_content, len);
        pthread_exit(0);
}


int main(int argc, char *argv[]) {

    int sem_value;

    // checks for the correct args number
    if (argc != 2) {
        printf("Nombre d'arguments incorrect. Utilisation :\n");
        printf("%s <num_port>\n", argv[0]);
        exit(0);
    } else {
        printf("Lancement du serveur...\n");
    }

    // semaphore initialisation
    sem_init(&semaphore, PTHREAD_PROCESS_SHARED, MAX_CLIENTS);
    //sem_init(&file_semaphore, PTHREAD_PROCESS_SHARED, 0);

    // configures the server messaging socket
    int server_msg_socket;
    struct sockaddr_in server_msg_address;
    configure_listening_socket(atoi(argv[1]), &server_msg_socket, &server_msg_address);
    bind_and_listen_on(server_msg_socket, server_msg_address);

    // configures the server receving file socket
    configure_listening_socket(atoi(argv[1]) + 1, &server_file_socket, &server_file_address);
    bind_and_listen_on(server_file_socket, server_file_address);

    //configures the server sending file socket on port + 2
    configure_listening_socket(atoi(argv[1]) + 2, &server_send_file_socket, &server_send_file_address);
    bind_and_listen_on(server_send_file_socket, server_send_file_address);

    //creation des rooms
    for (int w=0;w<NB_MAX_ROOM; w++){
      rooms[w].num_port=atoi(argv[1]) + 3+w;
      configure_listening_socket(rooms[w].num_port, &rooms[w].socket_room_server, &rooms[w].room_address);
      bind_and_listen_on(rooms[w].socket_room_server, rooms[w].room_address);
      char nom [20];
      char num[MAX_MSG_SIZE];
      strcat(nom, "SALON N°");
      sprintf(num, "%d", w);
      puts(num);
      strcat(nom, num);
      rooms[w].room_name=nom;
      rooms[w].nb_max_membre = 2;
    }


    while (1) {

        // decrements the semaphore before accepting a new connection
        sem_getvalue(&semaphore, &sem_value);
        int sem_wait_res = sem_wait(&semaphore); // decrements the semaphore, waits if it is 0
        check_error(sem_wait_res, "Erreur lors du sem_wait.\n");

        int client_msg_socket = accept_client(server_msg_socket);
        printf("client accepte\n");


        for (int k = 0; k < MAX_CLIENTS; k++) {
            if (clients[k].client_msg_socket == 0) {
                // we found a client not connected
                clients[k].client_msg_socket = client_msg_socket;
                pthread_create(&clients[k].messaging_thread, NULL, messaging_thread_func, (void *) (long) client_msg_socket);
                printf("Un client connecté de plus ! %d client(s)\n", get_client_count(semaphore));
                break;
            }
        }
    }
}