#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main(int argc, char *argv[]) {

    char msg1[32];

    int socket_descriptor = socket(PF_INET, SOCK_STREAM, 0); // creates a TCP socket

    //server address configuration
    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    inet_pton(AF_INET, argv[1], &(server_address.sin_addr)); //converts the address from the CLI to the correct format
    server_address.sin_port = htons(atoi(argv[2]));

    socklen_t lgA = sizeof(struct sockaddr_in) ;
    connect(socket_descriptor, (struct sockaddr*) &server_address, lgA); // opens the socket with the configured address

    for (int i = 0; i < 3; i++) {
        //while (1) {
        char buffer[32];

        // Reception du 1er client
        recv(socket_descriptor, &buffer, strlen(buffer) + 1, 0) ;
        printf("Réponse : %s", buffer) ;
        sleep(1);

        // Puis envoi
        printf("Mon message => ");
        scanf("%s",msg1);
        send(socket_descriptor, msg1, strlen(msg1) + 1, 0);

        sleep(1);
    }

    shutdown(socket_descriptor, 2);
}

