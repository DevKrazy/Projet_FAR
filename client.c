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
#include <fcntl.h>

// Threads
pthread_t message_sending_thread;
pthread_t file_sending_thread;
pthread_t file_recv_thread;

// Semaphores
//sem_t file_semaphore;
int fake_semaphore = 0;
int fake_recv_semaphore = 0;

// File transfer buffers
char file_content[MAX_FILE_SIZE];
char fileName[MAX_MSG_SIZE];
int file_size;

void get_file_to_send(int* size_file) {
    // Demander à l'utilisateur quel fichier afficher

    char* file_list = malloc(1); // malloc(1) because list_files reallocate the memory
    list_files("./", &file_list);
    printf("LISTE DE FICHIERS : \n %s", file_list);

    printf("Indiquer le nom du fichier : \n");
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
        printf("size %d\n",size);
        //write(1,file_content,size);

    }
    *size_file=size;
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

    while (1) {

        // gets what the client wrote in the CLI
        fgets(send_buffer, MAX_MSG_SIZE, stdin);

        if (strcmp(send_buffer, "file\n") == 0) {
            // the client wants to send a file
            get_file_to_send(&file_size);
            //sem_post(&file_semaphore);
            fake_semaphore += 1;
            printf("Fichier envoyé.\n");
        } else if (strcmp(send_buffer, "filesrv\n") == 0) {
            send(server_socket, send_buffer, MAX_MSG_SIZE, 0);
            // recevoir la liste
            printf("Entrez un fichier à recevoir: \n");
            fgets(fileName, MAX_MSG_SIZE, stdin);
            fake_recv_semaphore+=1;
        } else {
            // the client wants to send a message
            send(server_socket, send_buffer, MAX_MSG_SIZE, 0);
            printf("[Vous] : %s", send_buffer);
        }
    }
}

void* file_sending_thread_func(void *socket) {
    int server_socket = (int) (long) socket;
    while (1) {
        //sem_wait(&file_semaphore);
        while (fake_semaphore == 0) {}
        fake_semaphore -= 1;
        int len = (int) strlen(file_content);
        send(server_socket, fileName, MAX_MSG_SIZE, 0); // sends file name
        send(server_socket, &file_size, sizeof(int), 0); // sends file size
        send(server_socket, file_content, len, 0); // sends file content
        bzero(file_content, len);
    }
}

void* file_receiving_thread_func(void *socket) {
    int server_socket = (int) (long) socket;
    while (1) {
        //sem_wait(&file_semaphore);
        while (fake_recv_semaphore == 0) {}
        fake_recv_semaphore -= 1;
        //int len = (int) strlen(file_content);
        send(server_socket, fileName, MAX_MSG_SIZE, 0); // envoi du nom du fichier
        recv(server_socket, &file_size, sizeof(int), 0); // envoi taille fichier
        char* file_recv_cont = NULL;
        file_recv_cont = malloc(file_size);
        recv(server_socket, file_recv_cont, file_size, 0); // envoi du contenu du fichier
        free(file_recv_cont);
    }
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

    int server_file_sending_socket;
    struct sockaddr_in server_file_sending_address;
    configure_connecting_socket(argv[1], atoi(argv[2]) + 1, &server_file_sending_socket, &server_file_sending_address);
    connect_on(server_file_sending_socket, server_file_sending_address);
    pthread_create(&file_sending_thread, NULL, file_sending_thread_func, (void *) (long) server_file_sending_socket);

    int server_file_receiving_socket;
    struct sockaddr_in server_file_receiving_address;
    configure_connecting_socket(argv[1], atoi(argv[2]) + 1, &server_file_receiving_socket, &server_file_receiving_address);
    connect_on(server_file_receiving_socket, server_file_receiving_address);
    pthread_create(&file_recv_thread, NULL, file_receiving_thread_func, (void *) (long) server_file_receiving_socket);

    int server_messaging_socket;
    struct sockaddr_in server_message_sending_address;
    configure_connecting_socket(argv[1], atoi(argv[2]), &server_messaging_socket, &server_message_sending_address);
    connect_on(server_messaging_socket, server_message_sending_address);
    pthread_create(&message_sending_thread, NULL, message_sending_thread_func, (void *) (long) server_messaging_socket);


    char recv_buffer[MAX_MSG_SIZE];

    while (1) {
        recv(server_messaging_socket, recv_buffer, MAX_MSG_SIZE, 0);
        char affichage[MAX_MSG_SIZE+14];
        strcat(affichage,"[");
        strcat(affichage,recv_buffer);
        strcat(affichage,"] : ");
        int recv_res = recv(server_messaging_socket, recv_buffer, MAX_MSG_SIZE, 0);
        strcat(affichage,recv_buffer);
        printf("%s\n", affichage);
        if (recv_res == 0) {
            // the server closed the connection
            terminate_program(0);
        }
    }
}