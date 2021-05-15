#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <dirent.h>
#include <unistd.h>
#include <semaphore.h>
#include "utils/headers/utils.h"
#include "utils/headers/client_utils.h"
#include "utils/headers/file_utils.h"
#include <fcntl.h>

// Threads
pthread_t message_sending_thread;
pthread_t file_sending_thread;
pthread_t file_recv_thread;

//to send file to server
int server_file_sending_socket;
struct sockaddr_in server_file_sending_address;
char* send_filename;

//to recv file from server
int recv_file_socket;
struct sockaddr_in recv_file_address;

char argv1[15];
int argv2;

// File transfer buffers
char file_content[MAX_FILE_SIZE];
char fileName[MAX_MSG_SIZE];
int file_size;
void* file_sending_thread_func(void *socket);
void* file_receiving_thread_func(void *socket);

void* message_sending_thread_func(void *socket) {
    char send_buffer[MAX_MSG_SIZE];
    int server_socket = (int) (long) socket;

    // gets and sends the client's name to the server
    printf("Entrez votre pseudo (max 10 lettres) : \n");
    fgets(send_buffer, MAX_MSG_SIZE, stdin);
    send_buffer[strcspn(send_buffer, "\n")] = 0; // removes the \n at the end
    send(server_socket, send_buffer, MAX_MSG_SIZE, 0);
    printf("Bienvenue %s !\n", send_buffer);

    while (1) {

        // gets what the client wrote in the CLI
        fgets(send_buffer, MAX_MSG_SIZE, stdin);

        if (strcmp(send_buffer, "file\n") == 0) {

            get_filename_to_send(CLIENT_DIR, &send_filename);
            send(server_socket, send_buffer, MAX_MSG_SIZE, 0);
            configure_connecting_socket(argv1, argv2 + 1, &server_file_sending_socket, &server_file_sending_address);
            connect_on(server_file_sending_socket, server_file_sending_address);
            pthread_create(&file_sending_thread, NULL, file_sending_thread_func, (void *) (long) server_file_sending_socket);


        } else if (strcmp(send_buffer, "filesrv\n") == 0) {

            send(server_socket, send_buffer, MAX_MSG_SIZE, 0);
            fgets(fileName, MAX_MSG_SIZE, stdin);
            fileName[strcspn(fileName, "\n")] = 0;

            send(server_socket, fileName, MAX_MSG_SIZE, 0); // sends the name of the file
            configure_connecting_socket(argv1, argv2 + 2, &recv_file_socket, &recv_file_address);
            connect_on(recv_file_socket, recv_file_address);
            pthread_create(&file_recv_thread, NULL, file_receiving_thread_func, (void *) (long) recv_file_socket);
        } else {
            // the client wants to send a message
            send(server_socket, send_buffer, MAX_MSG_SIZE, 0);
            printf("[Vous] : %s", send_buffer);
        }
    }
}

void* file_sending_thread_func(void *socket) {
    int server_socket = (int) (long) socket;

    // sends the file
    send_file(server_socket, CLIENT_DIR, send_filename);

    shutdown(server_socket,2);
    pthread_exit(0);
}


void* file_receiving_thread_func(void *socket) {
    int server_socket = (int) (long) socket;


    recv(server_socket, &file_size, sizeof(int), 0); // reception taille fichier
    printf("RECEIVED SIZE FROM SERVER\n");

    char* file_recv_cont = NULL;
    file_recv_cont = malloc(file_size);
    recv(server_socket, file_recv_cont,file_size, 0); // reception du contenu du fichier
    printf("RECEIVED FILE CONTENT FROM SERVER\n");
    printf("%s\n", file_recv_cont);

    char folder[200] = CLIENT_DIR;
    strcat(folder, fileName);
    int fp = open(folder,  O_WRONLY | O_CREAT, S_IRWXU);
    write(fp, file_recv_cont, file_size);
    close(fp);
    free(file_recv_cont);
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