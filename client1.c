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

    int i =0;
    //while(i<3){
    printf("mon msg =>");
    scanf("%s",msg1);
    send(socket_descriptor, msg1, strlen(msg1)+1, 0) ;
    sleep(1);
    char r[32];
    recv(socket_descriptor, &r, strlen(r) + 1, 0) ;
    sleep(1);
    printf("Client2 : %s", r) ;
    i++;
    //}

    shutdown(socket_descriptor,2) ;
}

