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

// TODO: quand on reçoit un mp : remplacer "server" par le pseudo
// TODO: utiliser des puts plutot que des printf
// TODO: corriger le retour a la ligne en trop quand on reçoit un message (ça vient du strtok surement)

sem_t semaphore;
//sem_t file_semaphore;
int fake_semaphore = 0;
int fake_send_semaphore = 0;
Client clients[MAX_CLIENTS];
char file_content[MAX_FILE_SIZE];
char fileName[MAX_MSG_SIZE];
int file_size;

/**
 * The server's messaging thread.
 * @param socket the server's socket
 * @return
 */
void *messaging_thread(void *socket) {
    printf("Thread de messaging client créé !\n");
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
            clients[client_index].client_socket = 0; // client_socket reset
            sem_post(&semaphore);
            shutdown(client_socket, 2);
            pthread_exit(0);
        }

      
       if (strcmp(send_buffer, "filesrv\n") == 0) {
        printf("On est dans le fileSSRV\n");
        recv(client_socket, fileName, MAX_MSG_SIZE, 0); // reception du nom du fichier
        printf("Received Filename: %s\n", fileName );
         fake_send_semaphore += 1;
         printf("Fake_send_semaphore: %d\n", fake_send_semaphore);


       } else if (is_private_message(send_buffer, clients) == 1) {
            send_message_to(send_buffer, clients,client_socket);
        }
        else {
            broadcast_message(send_buffer, clients, client_index);
        }
        
    }
}

void* file_receiving_thread(void* socket) {
    printf("Lancement du thread de réception de fichiers.\n");
    int client_socket = (int) (long) socket;
    char fileName[MAX_MSG_SIZE];
    int size;
    char* file_content = NULL;

    while (1) {
        printf("Hello file\n");
        int recv1 = recv(client_socket, fileName, MAX_MSG_SIZE, 0);
        printf("fileName: %d\n",recv1 );
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
        //fp = fopen("test.txt", "w");
        printf("%s\n", file_content);
        //fputs(file_content, fp);
        write(fp,file_content,size);
        close(fp);
        free(file_content);
    }
}

void* file_sending_thread(void* socket) {
      printf("Lancement du thread d'envoi de fichiers.\n");
      int client_socket = (int) (long) socket;
      while (1) {
        //sem_wait(&file_semaphore);
        while (fake_send_semaphore == 0) {}
            printf("After filesending fake_send_semaphore\n");
        fake_send_semaphore -= 1;
        int len = (int) strlen(file_content);
        
        
        int fp = open(fileName, O_RDONLY);
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
            close(fp);

        
        send(client_socket, &file_size, sizeof(int), 0); // envoi taille fichier
        send(client_socket, file_content, len, 0); // envoi du contenu du fichier
        printf("file_content: %s\n",file_content);
        printf("Fichier envoyé !\n");
        bzero(file_content, len);
    }
}

/**
 * Configures the server's socket and updates the socket_return and addr_return values with the
 * created socket and the created address.
 * @param port the port we want to use
 * @param socket_return the pointer where the created socket will be stored at
 * @param addr_return the address where the created sockaddr_in will be stored at
 * @return 0 if everything was successful; -1 if there was an error during socket creation
 */
int configure_server_socket(char* port, int* socket_return, struct sockaddr_in *addr_return) {

    // creates a socket in the IPV4 domain using TCP protocol
    int server_socket = socket(PF_INET, SOCK_STREAM, 0);
    if (server_socket == -1) {
        perror("Erreur lors de la création de la socket serveur pour les messages.\n");
        return -1;
    }
    printf("Socket serveur créée avec succès.\n");

    // server address configuration
    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET; // address type
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons(atoi(port)); // address port (converted from the CLI)
    printf("Adresse du serveur configurée avec succès ! (port : %s)\n", port);

    *socket_return = server_socket;
    *addr_return = server_address;

    return 0;
}

/**
 * Binds the given sockaddr_in to the given socket.
 * @param socket the socket we want to bind the address to
 * @param address the address we want to bind
 * @return 0 if successful; -1 if bind or listen failed (with errno positioned)
 */
int bind_and_listen_on(int socket, struct sockaddr_in address) {
    // bind
    int bind_res = bind(socket, (struct sockaddr*) &address, sizeof(address)); // binds address to server socket
    if (bind_res == -1) {
        perror("Erreur lors du bind\\n");
        return -1;
    }
    printf("Bind réussi !\n");

    // listen
    int listen_res = listen(socket, MAX_CLIENTS); // listens for incoming connections (maximum 2 waiting connections)
    if (listen_res == -1) {
        perror("Erreur lors du listen\\n");
        return -1;
    }
    printf("Le serveur écoute !\n");
    return 0;
}

/**
 * Configures the server and returns the server's socket.
 * @param port the listening port
 * @return the server socket
 */
int configure_server(int port) {

    // creates a socket in the IPV4 domain using TCP protocol
    int server_socket = socket(PF_INET, SOCK_STREAM, 0);
    check_error(server_socket, "Erreur lors de la création de la socket serveur.\n");
    printf("Socket serveur créée avec succès.\n");

    // server address configuration
    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET; // address type
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons(port); // address port (converted from the CLI)
    //server_address.sin_port = htons(atoi(argv[1])); // address port (converted from the CLI)

    // bind
    int bind_res = bind(server_socket, (struct sockaddr*) &server_address, sizeof(server_address)); // binds address to server socket
    check_error(bind_res, "Erreur lors du bind\n");
    printf("Bind réussi !\n");

    // listen
    int listen_res = listen(server_socket, MAX_CLIENTS); // listens for incoming connections (maximum 2 waiting connections)
    check_error(listen_res, "Erreur lors du listen\n");
    printf("Le serveur écoute sur le port %d\n", port);
    //printf("Le serveur écoute sur le port %s.\n", argv[1]);

    return server_socket;
}

int accept_client(int server_socket) {

    // clients address initialization
    struct sockaddr_in client_address;
    socklen_t client_address_len = sizeof(struct sockaddr_in);

    int client_socket = accept(server_socket, (struct sockaddr *) &client_address, &client_address_len);
    check_error(client_socket, "Erreur lors de l'acceptation du client.\n");
    send(client_socket, "Connexion acceptée\n", MAX_MSG_SIZE, 0);
    return client_socket;
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
    //check_error(-1, "Erreur lors de l'initialisation du semaphore.\n");
    int server_msg_socket = configure_server(atoi(argv[1]));
    int server_file_socket = configure_server(atoi(argv[1])+1);

    /*
    // configures the server messaging socket
    int server_msg_socket;
    struct sockaddr_in server_msg_address;
    configure_server_socket(argv[1], &server_msg_socket, &server_msg_address);
    bind_and_listen_on(server_msg_socket, server_msg_address);

    // configures the server file sending socket
    int server_file_socket;
    struct sockaddr_in server_file_address;
    configure_server_socket(argv[2], &server_file_socket, &server_file_address);
    bind_and_listen_on(server_file_socket, server_file_address);
     */

    while (1) {

        // clients address initialization
        /*
        struct sockaddr_in client_msg_address;
        socklen_t client_address_len = sizeof(struct sockaddr_in);
         */

        // decrements the semaphore before accepting a new connection
        sem_getvalue(&semaphore, &sem_value);
        int sem_wait_res = sem_wait(&semaphore); // decrements the semaphore, waits if it is 0
        check_error(sem_wait_res, "Erreur lors du sem_wait.\n");

        /*
        int client_msg_socket = accept(server_msg_socket, (struct sockaddr *) &client_msg_address, &client_address_len);
        check_error(client_msg_socket, "Erreur lors de l'acceptation du client.\n");
        send(client_msg_socket, "Connexion acceptée\n", MAX_MSG_SIZE, 0);
         */


        int client_file_socket = accept_client(server_file_socket);
        int client_file_sending_socket = accept_client(server_file_socket);
        int client_msg_socket = accept_client(server_msg_socket);


        for (int k = 0; k < MAX_CLIENTS; k++) {
            if (clients[k].client_socket == 0) {
                // we found a client not connected
                clients[k].client_socket = client_msg_socket;
                pthread_create(&clients[k].msg_thread, NULL, messaging_thread, (void *) (long) client_msg_socket);
                pthread_create(&clients[k].file_thread, NULL, file_receiving_thread, (void *) (long) client_file_socket);
                pthread_create(&clients[k].file_send_thread, NULL, file_sending_thread, (void *) (long) client_file_sending_socket);
                printf("Un clients connecté de plus ! %d client(s)\n", get_client_count(semaphore));
                break;
            }
        }
    }
}