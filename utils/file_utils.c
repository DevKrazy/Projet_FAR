#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include "headers/utils.h"
#include "headers/file_utils.h"


/**
 * Opens, reads and sends a file to a socket.
 * @param socket the socket we want to send the file to
 * @param folder the folder where the file is stored (must end with a /)
 * @param filename the name of the file we want to send
 */
// TODO read and send chunk by chunk
void send_file(int socket, char* folder, char* filename) {
    char* path = malloc(strlen(folder) + strlen(filename) + 1); // path to the file

    // formats the path
    strcat(path, folder); // adds the folder to the path
    strcat(path, filename); // adds the filename to the path

    int file = open(path, O_RDONLY);

    if (file == -1) {
        printf("Erreur lors de l'ouverture du fichier %s\n", path);
        perror("Erreur : ");
    } else {

        // reads the file
        char file_content[MAX_FILE_SIZE];
        int name_size = strlen(filename);
        printf("Name_size: %d\n", name_size);
        read(file, file_content, MAX_FILE_SIZE);

        // sends the file
        send(socket, &name_size, sizeof(int), 0); // sends the filename size
        send(socket, filename, name_size, 0); // sends the filename
        send(socket, file_content, MAX_FILE_SIZE, 0); // sends the file content

        close(file);
        printf("Le fichier %s a été envoyé au socket %d\n", path, socket);
    }
    free(path);
}

/**
 * Receives, opens and writes a file from a socket.
 * @param socket the socket we receive the file from
 * @param folder the folder in which we will store the received file
 */
// TODO receive and write chunk by chunk
void receive_file(int socket, char* folder) {

    char filename[MAX_MSG_SIZE];
    int name_size;
    char file_content[MAX_FILE_SIZE];

    // receives the file
    recv(socket, &name_size, sizeof(int), 0); // receives the filename size
    printf("Received name size: %d\n", name_size);
    recv(socket, filename, name_size, 0); // receives the filename
    recv(socket, file_content, MAX_FILE_SIZE, 0); // receives the file content

    // formats the path
    char* path = malloc(strlen(folder) + strlen(filename) + 1); // path to the file
    strcat(path, folder);
    strcat(path, filename);

    int file = open(path,  O_WRONLY | O_CREAT, S_IRWXU);

    if (file == -1) {
        printf("Erreur lors de l'ouverture du fichier %s\n", path);
        perror("Erreur : ");
    } else {
        // writes the file
        write(file, file_content, MAX_FILE_SIZE);
        close(file);
        printf("Le fichier %s a été reçu du socket %d et sauvegardé.\n", path, socket);
    }
    free(path);
}

// TODO: move list_files into this file