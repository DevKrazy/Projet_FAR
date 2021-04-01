#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main(int argc, char *argv[]) {

    int socket_descriptor = socket(PF_INET, SOCK_STREAM, 0); // creates a TCP socket

    //server address configuration
    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET; // address type
    inet_pton(AF_INET, argv[1], &(server_address.sin_addr)); //converts the address from the CLI to the correct format
    server_address.sin_port = htons(atoi(argv[2])); // address port (converted from the CLI)

    socklen_t server_address_len = sizeof(struct sockaddr_in);
    connect(socket_descriptor, (struct sockaddr*) &server_address, server_address_len); // opens the socket with the configured address

    int i = 0;
    while (i < 3) {

        // Envoi du message
        char send_buffer[32];
        printf("Mon message => ");
        scanf("%s", send_buffer);
        send(socket_descriptor, send_buffer, strlen(send_buffer) + 1, 0);

        // Reception du message
        char reception_buffer[32];
        recv(socket_descriptor, &reception_buffer, strlen(reception_buffer) + 1, 0) ;
        printf("Réponse : %s", reception_buffer) ;

        i++;
    }

    shutdown(socket_descriptor, 2);
}

