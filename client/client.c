#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <dirent.h>
#include <unistd.h>
#include <semaphore.h>
#include "../common/headers/utils.h"
#include "headers/client_utils.h"
#include <fcntl.h>
#include <ctype.h>

// TODO : vérifier qu'un client est dans une room avant d'envoyer le emssage
// TODO : ne pas envoyer le emssage de room au client lui-même
// TODO : voir les rooms dans lesquelles on est

// Threads
pthread_t message_sending_thread;
pthread_t file_sending_thread;
pthread_t file_recv_thread;


//to send file to server
int server_file_sending_socket;
struct sockaddr_in server_file_sending_address;

//to recv file from server
int recv_file_socket;
struct sockaddr_in recv_file_address;

//to join room
int server_room_socket;
struct sockaddr_in server_room_address;

char argv1[15];
int argv2;

// File transfer buffers
char file_content[MAX_FILE_SIZE];
char fileName[MAX_MSG_SIZE];
int file_size;

// TODO bouger ces fonctions dans un header
void* file_sending_thread_func(void *socket);
void* file_receiving_thread_func(void *socket);


void get_file_to_send(int* size_file) {
    // Demander à l'utilisateur quel fichier afficher

    char* file_list = malloc(1); // malloc(1) because list_files reallocate the memory
    list_files(CLIENT_DIR, &file_list);
    printf("Liste des fichiers :\n%s", file_list);

    printf("Indiquer le nom du fichier à envoyer au serveur : \n");
    fgets(fileName, sizeof(fileName), stdin);
    fileName[strlen(fileName) - 1] = '\0';

    int fp = open(fileName,O_RDONLY);
    int size=0;
    if (fp == -1){
        printf("Ne peux pas ouvrir le fichier suivant : %s\n", fileName);
    } else {
        char str[MAX_FILE_SIZE];
        // Stores the file content
        size = read(fp,file_content,MAX_FILE_SIZE);
        file_content[size]=0;

    }
    //bzero(nomTot, 40);
    *size_file=size;
    free(file_list);
    close(fp);
}

void* message_sending_thread_func(void *socket) {
    char send_buffer[MAX_MSG_SIZE];
    int server_socket = (int) (long) socket;

    // gets and sends the client's name to the server
    printf("Entrez votre pseudo (max 10 lettres) : \n");
    fgets(send_buffer, MAX_MSG_SIZE, stdin);
    send_buffer[strcspn(send_buffer, "\n")] = 0; // removes the \n at the end
    send(server_socket, send_buffer, MAX_MSG_SIZE, 0);
    printf("Bienvenue %s !\n", send_buffer);
    printf("Entrez /man pour avoir de l'aide\n");

    while (1) {

        // gets what the client wrote in the CLI
        fgets(send_buffer, MAX_MSG_SIZE, stdin);

        if (strcmp(send_buffer, "/file\n") == 0) {

            print_title("Envoi de fichier");
            get_file_to_send(&file_size);
            send(server_socket, send_buffer, MAX_MSG_SIZE, 0); // sends the command to the server
            configure_connecting_socket(argv1, argv2 + 1, &server_file_sending_socket, &server_file_sending_address);
            connect_on(server_file_sending_socket, server_file_sending_address);
            pthread_create(&file_sending_thread, NULL, file_sending_thread_func, (void *) (long) server_file_sending_socket);

        } else if (strcmp(send_buffer, "/man\n") == 0) {
            print_man();
        } else if (strcmp(send_buffer, "/room\n") == 0) {

            send(server_socket, send_buffer, MAX_MSG_SIZE, 0); // sends the command to the server
            configure_connecting_socket(argv1, argv2 + 1, &server_room_socket, &server_room_address);
            connect_on(server_room_socket, server_room_address);
            recv(server_room_socket, send_buffer, 150, 0); // receives either the room list
            
            if (strcmp(send_buffer, "Pas de salon de disponible") == 0) {
                printf("Il n'y a aucun salon, utilisez /room create pour en créer.\n");
            } 
            else {
                print_title("Salons");
                printf("%s\n", send_buffer);

                // Asks the user for the room id
                printf("Avec quel salon souhaitez-vous interagir ? \n");
                fgets(send_buffer, MAX_MSG_SIZE, stdin);
                while (is_string_a_number(send_buffer)==0 ){
                  printf("Vous n'avez pas entré un nombre, réessayez\n");
                  fgets(send_buffer, MAX_MSG_SIZE, stdin);
                }
                int room_id = atoi(send_buffer);
                send(server_room_socket, &room_id, sizeof(int), 0); // sends the room id
                

                // Asks the user what he wants to do with the selected room
                print_room_actions();
                printf("Que souhaitez-vous faire ?\n");
                fgets(send_buffer, MAX_MSG_SIZE, stdin);
                int action_id = get_action_id(send_buffer);
                send(server_room_socket, &action_id, sizeof(int), 0); // sends the action id

                switch (action_id) {
                    case 2: { // modify room

                        // Asks the user what he wants to modify
                        print_room_modification_actions();
                        fgets(send_buffer, sizeof(int), stdin);
                        int modif_action_id = atoi(send_buffer);
                        send(server_room_socket, &modif_action_id, sizeof(int), 0); // sends the modification action id

                        client_room_modification(server_room_socket, modif_action_id);
                        break;
                    }
                    default: { // all other cases (just need the server's response)
                        //printf("Inside default clause.\n");
                        recv(server_room_socket, send_buffer, MAX_MSG_SIZE, 0); // receives the server's response
                        //printf("Response buffer: %s\n", send_buffer);
                        break;
                    }
                }
                recv(server_room_socket, send_buffer, MAX_MSG_SIZE,0);
                printf("%s\n",send_buffer);
                print_separator(strlen("Salons"));
            }

            sleep(1);
            shutdown(server_room_socket,2);

        } else if (strcmp(send_buffer, "/room create\n") == 0) {

            send(server_socket, send_buffer, MAX_MSG_SIZE, 0); // sends the command to the server
            configure_connecting_socket(argv1, argv2 + 1, &server_room_socket, &server_room_address);
            connect_on(server_room_socket, server_room_address);

            client_room_creation(server_room_socket);

        } else if (strcmp(send_buffer, "/filesrv\n") == 0) {

            print_title("Réception de fichier");
            send(server_socket, send_buffer, MAX_MSG_SIZE, 0); // sends the command to the server

            // Asks the user which file he wants to send
            printf("Indiquer le nom fu fichier à recevoir du serveur :\n");
            fgets(fileName, MAX_MSG_SIZE, stdin);
            fileName[strcspn(fileName, "\n")] = 0;
            send(server_socket, fileName, MAX_MSG_SIZE, 0); // sends the name of the file

            // Configures the socket and starts the file receiving thread
            configure_connecting_socket(argv1, argv2 + 2, &recv_file_socket, &recv_file_address);
            connect_on(recv_file_socket, recv_file_address);
            pthread_create(&file_recv_thread, NULL, file_receiving_thread_func, (void *) (long) recv_file_socket);


        }else if (strcmp(send_buffer, "/mute\n")==0){
            send(server_socket, send_buffer, MAX_MSG_SIZE, 0); // sends the command to the server
            printf("Vous etes mute\n");
        } 
        else if (strcmp(send_buffer, "/demute\n")==0){
            send(server_socket, send_buffer, MAX_MSG_SIZE, 0); // sends the command to the server
            printf("Vous etes demute \n");
        } 
         else {
            // the client wants to send a message
            send(server_socket, send_buffer, MAX_MSG_SIZE, 0);
        }
        // TODO: free send_buffer ? (vérifier que ça casse rien)
    }
}

void* file_sending_thread_func(void *socket) {
    int server_socket = (int) (long) socket;
    send(server_socket, fileName, MAX_MSG_SIZE, 0); // sends file name
    send(server_socket, &file_size, sizeof(int), 0); // sends file size
    send(server_socket, file_content, file_size, 0); // sends file content
    printf("Fichier envoyé.\n");
    print_separator(strlen("Envoi de fichier"));
    bzero(file_content, file_size);
    shutdown(server_socket,2);
    pthread_exit(0);
}

void* file_receiving_thread_func(void *socket) {
    int server_socket = (int) (long) socket;


    recv(server_socket, &file_size, sizeof(int), 0); // reception taille fichier

    char* file_recv_cont = NULL;
    file_recv_cont = malloc(file_size);
    recv(server_socket, file_recv_cont,file_size, 0); // reception du contenu du fichier

    char folder[200] = CLIENT_DIR;
    strcat(folder, fileName);
    int fp = open(folder,  O_WRONLY | O_CREAT, S_IRWXU);
    write(fp, file_recv_cont, file_size);
    close(fp);
    free(file_recv_cont);
    printf("Fichier %s reçu et enregistré.\n", fileName);
    print_separator(strlen("Réception de fichier"));
    shutdown(server_socket,2);
    pthread_exit(0);
}

int main(int argc, char *argv[]) {

    // checks for the correct args number
    if (argc != 3) {
        printf("Nombre d'arguments incorrect. Utilisation :\n");
        printf("%s <adresse_ip_serveur> <port>\n", argv[0]);
        exit(0);
    } else {
        printf("Lancement du client...\n");
    }

    strcpy(argv1,argv[1]);
    argv2=atoi(argv[2]);


    int server_messaging_socket;
    struct sockaddr_in server_message_sending_address;
    configure_connecting_socket(argv[1], atoi(argv[2]), &server_messaging_socket, &server_message_sending_address);
    connect_on(server_messaging_socket, server_message_sending_address);
    pthread_create(&message_sending_thread, NULL, message_sending_thread_func, (void *) (long) server_messaging_socket);


    char recv_buffer[MAX_MSG_SIZE];

    while (1) {
        int recv_res = recv(server_messaging_socket, recv_buffer, MAX_MSG_SIZE, 0);
        if (recv_res == 0) {
            // the server closed the connection
            terminate_program(0);
        }
        else{
            printf("%s\n", recv_buffer);
        }
    }
}
