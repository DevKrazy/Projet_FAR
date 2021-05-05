#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <dirent.h>
#include "utils/headers/utils.h"
#include "utils/headers/server_utils.h"

pthread_t msg_thread;
pthread_t file_thread;
sem_t file_semaphore;

char file_content[MAX_FILE_SIZE];
char file_name[MAX_MSG_SIZE];

/**
 * Configures the server's socket and updates the socket_return and addr_return values with the
 * created socket and the created address.
 * @param address the IPV4 address we want to use
 * @param port the port we want to use
 * @param socket_return the pointer where the created socket will be stored at
 * @param addr_return the address where the created sockaddr_in will be stored at
 * @return 0 if everything was successful; -1 if there was an error during socket creation
 */
int configure_server_socket(char* address, int port, int* socket_return, struct sockaddr_in *addr_return) {

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
    inet_pton(AF_INET, address, &(server_address.sin_addr)); //converts the address from the CLI to the correct format
    server_address.sin_port = htons(port); // address port (converted from the CLI)
    printf("Adresse du serveur configurée avec succès ! (%s:%d)\n", address, port);

    *socket_return = server_socket;
    *addr_return = server_address;
    return 0;
}

/**
 * Connects the given address to the given socket.
 * @param socket the server's socket
 * @param address the server's address (ip + port)
 * @return
 */
int connect_on(int socket, struct sockaddr_in address) {
    // connection to the server (message socket)
    socklen_t server_address_len = sizeof(struct sockaddr_in);
    int connect_res = connect(socket, (struct sockaddr*) &address, server_address_len); // opens the socket with the configured address
    if (connect_res == -1) {
        perror("Erreur lors de la connexion au serveur.\n");
        return -1;
    }
    printf("En attente de l'acceptation du serveur...\n");
    return 0;
}

/**
 * Asks the user which file he wishes to send to the server and stores it in the file_content global variable.
 */
void get_file_to_send() {

    DIR *dir_stream;
    struct dirent *dir_entry;
    dir_stream = opendir("./");

    // displays the files list to the client
    if (dir_stream != NULL) {
        printf("Voilà la liste de fichiers :\n");
        while (dir_entry = readdir (dir_stream)) {
            if(strcmp(dir_entry->d_name, ".") != 0 && strcmp(dir_entry->d_name, "..") != 0) {
                // does not display the . and .. files
                printf(" - %s\n", dir_entry->d_name);
            }
        }
        (void) closedir(dir_stream);
    } else {
        perror ("Ne peux pas ouvrir le répertoire");
    }

    // asks the client to select a file
    printf("Indiquer le nom du fichier : ");
    fgets(file_name, sizeof(file_name), stdin);
    file_name[strlen(file_name) - 1] = '\0';
    FILE *file_to_send = fopen(file_name, "r");
    if (file_to_send == NULL){
        printf("Impossible d'ouvrir le fichier : %s\n", file_name);
    } else {
        char str[1000];
        // stores the file content chunk by chunk in a global variable
        while (fgets(str, 1000, file_to_send) != NULL) {
            strcat(file_content, str);
        }
        printf("= = = = = = = Contenu fichier = = = = = = \n%s\n", file_content );
        printf("= = = = = = = = = = = = = = = = = = = = =\n");
    }
    fclose(file_to_send); // closes the file whatever happens
}

/**
 * The client's messaging thread.
 * @param socket the server's socket
 */
void* message_transfer_thread(void* socket) {
    printf("Thread de messages créé !\n");
    char send_buffer[MAX_MSG_SIZE];
    int server_socket = (int) (long) socket;

    // gets and sends the client's name to the server
    printf("Entrez votre pseudo (max %d lettres) : ", MAX_NAME_SIZE);
    fgets(send_buffer, MAX_MSG_SIZE, stdin);
    send_buffer[strcspn(send_buffer, "\n")] = 0; // removes the \n at the end
    send(server_socket, send_buffer, MAX_MSG_SIZE, 0);
    printf("Bienvenue %s !\n", send_buffer);

    while (1) {
        printf("> ");
        fgets(send_buffer, MAX_MSG_SIZE, stdin);

        if (strcmp(send_buffer, "file\n") == 0) {
            // the client wants to send a file
            get_file_to_send();
            sem_post(&file_semaphore);

        } else {
            // the client wants to send a message
            send(server_socket, send_buffer, MAX_MSG_SIZE, 0);
            printf("[Vous] : %s", send_buffer);
        }
    }
}

/**
 * The client's file SENDING thread.
 * @param socket the server's socket
 */
void* client_file_sending_thread(void* socket) {
    printf("Thread d'envoi de fichiers créé !\n");
    int server_socket = (int) (long) socket;
    while (1) {
        sem_wait(&file_semaphore);
        printf("After sem_wait\n");
        //connect_on(server_socket, server_file_address);
        int len = (int) strlen(file_content); //TODO changer la manière de calculer la tialle
        printf("Envoi du fichier...\n");
        send(server_socket, file_name, MAX_MSG_SIZE, 0); // sends the file name
        printf("Sent the file name\n");
        send(server_socket, &len, sizeof(int), 0); // sends the file size
        printf("Sent the file len\n");
        send(server_socket, file_content, len, 0); // sends the file content
        printf("Sent the file content\n");
        printf("Fichier envoyé !\n");
        bzero(file_content, len); // clears the array
        //shutdown(server_socket, 2); // disconnects from the server
    }
}

// TODO: void* file_sending_thread(void* socket) {}

// TODO: void* message_receiving_thread(void* socket) {}

int main(int argc, char* argv[]) {

    // checks for the correct args number
    if (argc != 3) {
        printf("Nombre d'arguments incorrect. Utilisation :\n");
        printf("%s <adresse_ip_serveur> <port>\n", argv[0]);
        exit(0);
    } else {
        printf("Lancement du client...\n");
    }


    // configures the server's file sending socket and connects to it
    int server_file_socket;
    struct sockaddr_in server_file_address;
    configure_server_socket(argv[1], atoi(argv[2]) + 1, &server_file_socket, &server_file_address);
    connect_on(server_file_socket, server_file_address);
    // starts the file sending thread
    pthread_create(&file_thread, NULL, client_file_sending_thread, (void *) (long) server_file_socket);


    // configures the server's message sending socket and connects to it
    int server_msg_socket;
    struct sockaddr_in server_msg_address;
    configure_server_socket(argv[1], atoi(argv[2]), &server_msg_socket, &server_msg_address);
    connect_on(server_msg_socket, server_msg_address);
    // starts the message sending thread
    pthread_create(&msg_thread, NULL, message_transfer_thread, (void *) (long) server_msg_socket);

    printf("All thread created\n");

    // starts the file receiving thread
    // TODO code the thread


    // starts the message sending thread
    // TODO: move the message receving code into a thread


    // = = = = = = on pourrait mettre ce code dans un thread dédié à la reception de message = = = = = //

    // receives the connection confirmation message from the server
    char recv_buffer[MAX_MSG_SIZE];
    recv(server_msg_socket, recv_buffer, MAX_MSG_SIZE, 0);
    printf("%s",recv_buffer );

    sem_init(&file_semaphore, PTHREAD_PROCESS_SHARED, 0);

    while (1) {
        // receiving the messages from the server
        int recv_res = recv(server_msg_socket, recv_buffer, MAX_MSG_SIZE, 0);
        printf("[Serveur] : %s\n", recv_buffer);
        if (recv_res == 0) {
            // the server closed the connection
            terminate_program(0);
        }
    }

    // = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = //

}