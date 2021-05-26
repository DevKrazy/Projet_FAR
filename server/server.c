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
#include "../common/headers/utils.h"
#include "headers/server_utils.h"

// TODO: utiliser des puts plutot que des printf
// TODO: corriger le retour a la ligne en trop quand on reçoit un message (ça vient du strtok surement)

sem_t semaphore;
//sem_t file_semaphore;
Client clients[MAX_CLIENTS];
Room rooms[NB_MAX_ROOM];
char file_content[MAX_FILE_SIZE];
char fileName[MAX_MSG_SIZE];
int file_size;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

int server_msg_socket;

//pour enregitrer les fichiers dans le serveur
int server_file_socket;
struct sockaddr_in server_file_address;

//pour envoyer les fichiers au client
int server_send_file_socket;
struct sockaddr_in server_send_file_address;

void* file_sending_thread_func(void* socket);
void* file_receiving_thread_func(void* socket);


void* file_receiving_thread_func(void* socket) {
    int client_socket = (int) (long) socket;
    char fileName[MAX_MSG_SIZE];
    int size;
    char* file_content = NULL;

    recv(client_socket, fileName, MAX_MSG_SIZE, 0);
    recv(client_socket, &size, sizeof(int), 0);
    file_content = malloc(size);
    recv(client_socket,file_content, size , 0);

    // saves the file
    char* path = malloc(strlen(SERVER_DIR) + strlen(fileName) + 1);
    strcat(path, SERVER_DIR);
    strcat(path, fileName);
    int fp = open(path,  O_WRONLY | O_CREAT, S_IRWXU);
    write(fp, file_content, size);
    printf("Fichier %s reçu et enregistré.\n", fileName);
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
    for (int z=0; z<NB_MAX_ROOM; z++){
        clients[client_index].rooms[z]=0;
    }

    // receives and adds the client's name to its structure
    recv(client_socket, send_buffer, MAX_MSG_SIZE, 0);
    strcpy(clients[client_index].pseudo, send_buffer);

    while (1) {
        int recv_res = recv(client_socket, send_buffer, MAX_MSG_SIZE, 0);
        //check_error(-1, "Erreur lors de la réception du message du client.\n");

        // Checks if the client closed the discussion or the program

        if (recv_res == 0 || strcmp(send_buffer, "/fin\n") == 0) {
        	strcpy(send_buffer,"Je m en vais, a bientot ! ");
        	broadcast_message(send_buffer, clients, client_index);
            clients[client_index].client_msg_socket = 0; // client_msg_socket reset
            sem_post(&semaphore);

            shutdown(client_socket, 2);
            pthread_exit(0);
        }
        if (strcmp(send_buffer, "/destroyserver\n") == 0){

        	strcpy(send_buffer,"Serveur arreté, aurevoir ! ");
        	broadcast_message(send_buffer, clients, client_index);
        	shutdown(server_file_socket,2);
        	shutdown(server_send_file_socket,2);
        	shutdown(server_msg_socket,2);

        }else if (strcmp(send_buffer, "/file\n")==0) {

            clients[client_index].client_file_receiving_socket = accept_client(server_file_socket);
            pthread_create(&clients[client_index].file_receiving_thread, NULL, file_receiving_thread_func, (void *) (long) clients[client_index].client_file_receiving_socket);

        } else if (strcmp(send_buffer, "/filesrv\n") == 0) {
            printf("IN FILESRV\n");
            char* file_list = malloc(1); // malloc(1) because list_files reallocate the memory
            list_files(SERVER_DIR, &file_list);
            printf("LISTE DE FICHIERS : \n %s", file_list);
            send(client_socket,file_list,MAX_MSG_SIZE, 0); // envoi list des fichiers

            recv(client_socket, fileName, MAX_MSG_SIZE, 0); // reception du nom du fichier
            clients[client_index].client_file_send_socket = accept_client(server_send_file_socket);
            pthread_create(&clients[client_index].file_sending_thread, NULL, file_sending_thread_func, (void *) (long) clients[client_index].client_file_send_socket);

        } else if (strcmp(send_buffer, "/room\n") == 0) {

            clients[client_index].client_file_receiving_socket = accept_client(server_file_socket);
            int nbroom=get_room_count(rooms);
            printf("nb room %d\n",nbroom);
            if (get_room_count(rooms) == 0) {
                strcpy(send_buffer, "Pas de salon de disponible");
                printf("send buf%s\n", send_buffer );
                send(clients[client_index].client_file_receiving_socket, send_buffer, 150, 0);
            } 
            else {

                // sends the room list
                list_Rooms(rooms, send_buffer);
                send(clients[client_index].client_file_receiving_socket, send_buffer, MAX_MSG_SIZE, 0);
              
                // receives the room id
                printf("--Reception rooms--\n");
                int room_id;
                recv(clients[client_index].client_file_receiving_socket, &room_id, sizeof(int), 0);
                
                // receives the action id
                int action_id;
                recv(clients[client_index].client_file_receiving_socket, &action_id, sizeof(int), 0);

                switch (action_id) {
                    case 0: { // join
                        if (is_valable_id_room(room_id, rooms)==0){
                          strcpy(send_buffer, "Numero de room incorrect");
                        }
                        else if (is_in_room(client_index, room_id, clients) == 0) {
                            join_room(client_index, room_id, clients, rooms);
                            strcpy(send_buffer, "Vous avez rejoint le salon.");
                        } else {
                            strcpy(send_buffer, "Impossible de rejoindre le salon, vous êtes déjà dedans.");
                        }
                        break;
                    }
                    case 1: { // leave
                        if (is_valable_id_room(room_id, rooms)==0){
                          strcpy(send_buffer, "Numero de room incorrect");
                        }
                        else if (is_in_room(client_index, room_id, clients) == 1) {
                            leave_room(client_index, room_id, clients, rooms);
                            strcpy(send_buffer, "Vous avez quitté le salon.");
                        } else {
                            strcpy(send_buffer, "Impossible de quitter le salon, vous n'êtes pas dedans.");
                        }
                        break;
                    }
                    case 2: { // modify
                        printf("modify\n");
                        // Receives the modification action id
                        int modif_action_id;
                        recv(clients[client_index].client_file_receiving_socket, &modif_action_id, sizeof(int), 0);
                        printf("Modif action :%d\n", modif_action_id);

                        server_room_modification(clients[client_index].client_file_receiving_socket, modif_action_id,
                                                 room_id, rooms);
                        //save_rooms(rooms);
                        break;
                    }
                    case 3: { // delete
                        delete_room(room_id, clients, rooms);
                        strcpy(send_buffer, "Salon supprimé !");
                        break;
                    }
                    default: // bad command
                        strcpy(send_buffer, "Mauvaise commande...");
                        break;
                }
		printf("Buffer sent to client: %s\n", send_buffer);

                send(clients[client_index].client_file_receiving_socket, send_buffer, 150, 0);
            }

            // sends the response to the client
            //send(clients[client_index].client_file_receiving_socket, send_buffer, 150, 0);
            printf("%s\n",send_buffer);

        } else if (strcmp(send_buffer, "/room create\n") == 0) {

            clients[client_index].client_file_receiving_socket = accept_client(server_file_socket);
            server_room_creation(clients[client_index].client_file_receiving_socket, rooms);
            //save_rooms(rooms);

        }else if (strcmp(send_buffer, "/mute\n")==0){
            clients[client_index].mute=1;
        } 
        else if (strcmp(send_buffer, "/demute\n")==0){
            clients[client_index].mute=0;
        } 
        else { // the client wants to send a message

            int room_id = get_room_id_from_message(send_buffer);

            if (room_id != -1) { // sends the message in a room
                if (is_in_room(client_index,room_id, clients) == 1) {
                    broadcast_message_in_room(send_buffer, clients, rooms, room_id, client_index);
                }
            } else if (is_private_message(send_buffer, clients) == 1) { // sends the message to a user

                send_message_to(send_buffer, clients,client_socket);

            } else { // sends the message in the general chat
                broadcast_message(send_buffer, clients, client_index);
            }
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
    
    struct sockaddr_in server_msg_address;
    configure_listening_socket(atoi(argv[1]), &server_msg_socket, &server_msg_address);
    bind_and_listen_on(server_msg_socket, server_msg_address);

    // configures the server receiving file socket
    configure_listening_socket(atoi(argv[1]) + 1, &server_file_socket, &server_file_address);
    bind_and_listen_on(server_file_socket, server_file_address);

    //configures the server sending file socket on port + 2
    configure_listening_socket(atoi(argv[1]) + 2, &server_send_file_socket, &server_send_file_address);
    bind_and_listen_on(server_send_file_socket, server_send_file_address);

    //creation des rooms
    for (int w=0;w<NB_MAX_ROOM; w++) {
        char nom[20] = "SALON N°";
        char num[MAX_MSG_SIZE];
        sprintf(num, "%d", w); // writes the "w" value inside the num
        strcat(nom, num);
        strcpy(rooms[w].room_name, nom);
        rooms[w].nb_max_membre = 3;
        rooms[w].created=0;
    }
    //load_rooms(rooms);


    while (1) {

        // decrements the semaphore before accepting a new connection
        sem_getvalue(&semaphore, &sem_value);
        int sem_wait_res = sem_wait(&semaphore); // decrements the semaphore, waits if it is 0
        check_error(sem_wait_res, "Erreur lors du sem_wait.\n");

        int client_msg_socket = accept_client(server_msg_socket);
        printf("Client accepté\n");


        for (int k = 0; k < MAX_CLIENTS; k++) {
            if (clients[k].client_msg_socket == 0) {
                // we found a client not connected
                clients[k].mute=0;
                clients[k].client_msg_socket = client_msg_socket;
                pthread_create(&clients[k].messaging_thread, NULL, messaging_thread_func, (void *) (long) client_msg_socket);
                printf("Un client connecté de plus ! %d client(s)\n", get_client_count(semaphore));
                break;
            }
        }
    }
}
