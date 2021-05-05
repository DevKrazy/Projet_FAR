#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>
#include "utils/headers/utils.h"
#include "utils/headers/server_utils.h"

// TODO: quand on reçoit un mp : remplacer "server" par le pseudo
// TODO: utiliser des puts plutot que des printf
// TODO: corriger le retour a la ligne en trop quand on reçoit un message (ça vient du strtok surement)

sem_t semaphore;

Client clients[MAX_CLIENTS];

/**
 * The server's messaging thread. Receives and sends the messages to the correct client(s).
 * @param socket the server's socket
 * @return
 */
void *message_transfer_thread(void *socket) {
    printf("Thread de transfert de messages client créé !\n");
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
            shutdown(clients[client_index].msg_socket, 2);
            shutdown(clients[client_index].file_socket, 2);
            clients[client_index].msg_socket = 0;
            clients[client_index].file_socket = 0;
            sem_post(&semaphore);
            pthread_cancel(clients[client_index].file_thread);
            pthread_exit(0);
        }

        //sends the message to the concerned clients
        if (is_private_message(send_buffer, clients) == 1) {
            send_message_to(send_buffer,clients);
        } else {
            broadcast_message(send_buffer, clients, client_index);
        }
    }
}

void* server_file_receiving_thread(void* socket) {
    printf("Thread de réception de fichiers créé !\n");

    int client_socket = (int) (long) socket;
    FILE *fp;

    // buffers
    char file_name[MAX_MSG_SIZE];
    int len;
    char* file_content = NULL;

    while (1) {
        // TODO: transmettre le fichier morceau par morceau, ne pas tout stocker dans la RAM
        printf(" = = = Entered while\n");
        recv(client_socket, file_name, MAX_MSG_SIZE, 0);
        printf("Received file name\n");
        recv(client_socket, &len, sizeof(int), 0);
        printf("Received file len\n");
        file_content = malloc(len);
        printf("Received file content\n");
        recv(client_socket, file_content, len, 0);
        printf(" = = = Received everything\n");

        // saves the file
        char folder[MAX_MSG_SIZE] = "./recv/";
        printf("Before strcata\n");
        fp = fopen(strcat(folder, file_name), "w");
        printf("After strcata\n");
        fputs(file_content, fp);
        printf("%s\n", file_content);
        fclose(fp);
        free(file_content);
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
int configure_server_socket(int port, int* socket_return, struct sockaddr_in *addr_return) {

    // creates a socket in the IPV4 domain using TCP protocol
    int server_socket = socket(PF_INET, SOCK_STREAM, 0);
    if (server_socket == -1) {
        perror("Erreur lors de la création de la socket serveur pour les messages.\n");
        return -1;
    }
    //printf("Socket serveur créée avec succès.\n");

    // server address configuration
    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET; // address type
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons(port); // address port (converted from the CLI)
    printf("Adresse du serveur configurée avec succès ! (port : %d)\n", port);

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
    //printf("Bind réussi !\n");

    // listen
    int listen_res = listen(socket, MAX_CLIENTS); // listens for incoming connections (maximum 2 waiting connections)
    if (listen_res == -1) {
        perror("Erreur lors du listen\\n");
        return -1;
    }
    //printf("Le serveur écoute !\n");
    return 0;
}

/**
 * Accepts a connection on the given socket and sends a confirmation message.
 * @param server_socket the socket on which we accept the connection
 * @return a socket ready for communication
 */
int accept_client_with_confirmation(int server_socket) {

    // clients address initialization
    struct sockaddr_in client_address;
    socklen_t client_address_len = sizeof(struct sockaddr_in);

    int client_socket = accept(server_socket, (struct sockaddr *) &client_address, &client_address_len);
    check_error(client_socket, "Erreur lors de l'acceptation du client.\n");
    printf("Connexion acceptée.\n");
    send(client_socket, "Connexion acceptée\n", MAX_MSG_SIZE, 0);
    return client_socket;
}

/**
 * Accepts a connection on the given socket.
 * @param server_socket the socket on which we accept the connection
 * @return a socket ready for communication
 */
int accept_client(int server_socket) {
    // clients address initialization
    struct sockaddr_in client_address;
    socklen_t client_address_len = sizeof(struct sockaddr_in);

    int client_socket = accept(server_socket, (struct sockaddr *) &client_address, &client_address_len);
    check_error(client_socket, "Erreur lors de l'acceptation du client.\n");
    printf("Connexion sans confirmation acceptée.\n");
    return client_socket;
}

int main(int argc, char *argv[]) {

    // semaphore initialisation
    sem_init(&semaphore, PTHREAD_PROCESS_SHARED, MAX_CLIENTS);

    // checks for the correct args number
    if (argc != 2) {
        printf("Nombre d'arguments incorrect. Utilisation :\n");
        printf("%s <num_port>\n", argv[0]);
        exit(0);
    } else {
        printf("Lancement du serveur...\n");
    }

    // configures the server messaging socket
    int server_msg_socket;
    struct sockaddr_in server_msg_address;
    configure_server_socket(atoi(argv[1]), &server_msg_socket, &server_msg_address);
    bind_and_listen_on(server_msg_socket, server_msg_address);

    // configures the server file transfer socket
    int server_file_socket;
    struct sockaddr_in server_file_address;
    configure_server_socket(atoi(argv[1]) + 1, &server_file_socket, &server_file_address);
    bind_and_listen_on(server_file_socket, server_file_address);


    while (1) {

        // decrements the semaphore before accepting a new connection
        int sem_wait_res = sem_wait(&semaphore); // decrements the semaphore, waits if it is 0
        check_error(sem_wait_res, "Erreur lors du sem_wait.\n");

        int client_file_socket = accept_client(server_file_socket);
        int client_msg_socket = accept_client_with_confirmation(server_msg_socket);

        // finds the first empty slot in the clients array
        for (int k = 0; k < MAX_CLIENTS; k++) {
            if (clients[k].msg_socket == 0) {
                // we found an empty slot
                clients[k].msg_socket = client_msg_socket;
                clients[k].file_socket = client_file_socket;
                pthread_create(&clients[k].msg_thread, NULL, message_transfer_thread, (void *) (long) client_msg_socket);
                pthread_create(&clients[k].file_thread, NULL, server_file_receiving_thread,(void *) (long) client_file_socket);
                printf("Un clients connecté de plus ! %d client(s)\n", get_client_count(semaphore));
                break;
            }
        }
    }
}