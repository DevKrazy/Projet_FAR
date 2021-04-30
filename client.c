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
char** files = NULL;

/**
 * Lists the files of the current directory.
 */
void list_files() {

    DIR* dir_stream; // directory stream
    struct dirent* dir_entry;
    dir_stream = opendir ("./");

    if (dir_stream != NULL) {
        dir_entry = readdir(dir_stream);
        int file_counter = 0;
        files = NULL;

        printf("Voilà la liste de fichiers :\n");
        while (dir_entry != NULL) { // readdir moves the dir_stream pointer forward
            if(strcmp(dir_entry->d_name, ".") != 0 && strcmp(dir_entry->d_name, "..") != 0) {
                // does not print the . and .. files
                printf("Trying to allocate %lu \n", (file_counter + 1) * sizeof(*files));
                files = realloc(files, (file_counter + 1) * sizeof(*files)); // reallocate memory for the newly found file
                files[file_counter] = dir_entry->d_name; // adds the filename to the files array
                printf(" - %s\n", files[file_counter]);
                file_counter += 1;
                //printf(" - %s\n", dir_entry->d_name);
            }
            dir_entry = readdir(dir_stream); // reads the next file from the directory stream
        }
        (void) closedir(dir_stream);
    }
    else {
        perror ("Erreur lors de l'ouverture du répertoire.");
    }
}

/**
 * The client's messaging thread.
 * @param socket the server's socket
 */
void* messaging_thread(void *socket) {
    char send_buffer[MAX_MSG_SIZE];
    int server_socket = (int) (long) socket;

    // gets and sends the client's name to the server
    printf("Entrez votre pseudo (max 10 lettres) : ");
    fgets(send_buffer, MAX_MSG_SIZE, stdin);
    send_buffer[strcspn(send_buffer, "\n")] = 0; // removes the \n at the end
    send(server_socket, send_buffer, MAX_MSG_SIZE, 0);
    printf("Bienvenue %s !\n", send_buffer);

    while (1) {
        fgets(send_buffer, MAX_MSG_SIZE, stdin);

        if (strcmp(send_buffer, "file\n") == 0) {
            // the client entered the "file" keyword
            list_files();
            printf("Choisissez un fichier\n");
            char filename[1023];
            scanf("%s", filename);
            printf("%s", filename);

            if (value_in_array(filename, files) == 1) { // CA NE FONCTIONNE PAS LE CHECK :'(
                printf("Envoi du fichier %s au serveur.\n", filename);
                // TODO: envoyer le fichier au serveur
            } else {
                printf("Le fichier %s n'existe pas...\n", filename);
            }

            free(files);
        } else {
            send(server_socket, send_buffer, MAX_MSG_SIZE, 0);
            printf("[Vous] : %s", send_buffer);
        }

        // if is file
            // recup liste des fichiers du répertoire
            // affichier liste des fichiers (ou demander un index)
            // demander nom fichier (dans variable globale)
                // si fichier existe dans la liste (faire fonction)
                    // sem_post
                    // print "envoi du fichier"
                // sinon
                    // print "fichier invalide"
        // else
            // on envoi le message reçu
    }
}

void* file_sending_thread(void *socket) {
    while (1) {
        sem_wait(&file_semaphore);
        // sem_wait
        // connect
        // send file
        // shutdown
    }
}

 /**
  * Configures the server's socket and updates the socket_return and addr_return values with the
  * created socket and the created address.
  * @param address the IPV4 address we want to use
  * @param port the port we want to use
  * @param socket_return the pointer where the created socket will be stored at
  * @param addr_return the address where the created sockaddr_in will be stored at
  * @return 0 if everything was successful; -1 if there was an error during socket creation
  */
int configure_server_socket(char* address, char* port, int* socket_return, struct sockaddr_in *addr_return) {

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
    server_address.sin_port = htons(atoi(port)); // address port (converted from the CLI)
    printf("Adresse du serveur configurée avec succès ! (%s:%s)\n", address, port);

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

int main(int argc, char *argv[]) {

    // checks for the correct args number
    if (argc != 4) {
        printf("Nombre d'arguments incorrect. Utilisation :\n");
        printf("%s <adresse_ip_serveur> <port_msg> <port_fichier>\n", argv[0]);
        exit(0);
    } else {
        printf("Lancement du client...\n");
    }

    // configures the server's message sending socket and connects to it
    int server_msg_socket;
    struct sockaddr_in server_msg_address;
    configure_server_socket(argv[1], argv[2], &server_msg_socket, &server_msg_address);
    connect_on(server_msg_socket, server_msg_address);

    // configures the server's file sending socket and connects to it
    int server_file_socket;
    struct sockaddr_in server_file_address;
    configure_server_socket(argv[1], argv[3], &server_file_socket, &server_file_address);
    connect_on(server_file_socket, server_file_address);


    // receives the connection confirmation message from the server
    char recv_buffer[MAX_MSG_SIZE];
    recv(server_msg_socket, recv_buffer, MAX_MSG_SIZE, 0);
    printf("%s",recv_buffer );

    // starts the messaging thread
    pthread_create(&msg_thread, NULL, messaging_thread, (void *) (long) server_msg_socket);

    // starts the file sending thread
    pthread_create(&file_thread, NULL, file_sending_thread, (void *) (long) server_file_socket);
    sem_init(&file_semaphore, PTHREAD_PROCESS_SHARED, 0);


    while (1) {
        int recv_res = recv(server_msg_socket, recv_buffer, MAX_MSG_SIZE, 0);
        printf("[Serveur] : %s\n", recv_buffer);
        if (recv_res == 0) {
            // the server closed the connection
            terminate_program(0);
        }
    }
}