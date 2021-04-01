#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int MAX_SIZE = 1000;

int main(int argc, char *argv[]) {

    int socket_descriptor = socket(PF_INET, SOCK_STREAM, 0); // creates a TCP socket

    //server address configuration
    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET; // address type
    inet_pton(AF_INET, argv[1], &(server_address.sin_addr)); //converts the address from the CLI to the correct format
    server_address.sin_port = htons(atoi(argv[2])); // address port (converted from the CLI)

    socklen_t server_address_len = sizeof(struct sockaddr_in);
    connect(socket_descriptor, (struct sockaddr*) &server_address, server_address_len); // opens the socket with the configured address
    // receives the client id from the server
    int client_id;
    recv(socket_descriptor, &client_id, 4, 0) ;
    printf("ClientID: %d\n", client_id);

    //int i =0;
    //while (i < 3) {

      if (client_id == 1) {

        // Sends a message to the server
        char send_buffer[MAX_SIZE];
        printf("Mon message => ");
        fgets(send_buffer, MAX_SIZE, stdin);
        send(socket_descriptor, send_buffer, strlen(send_buffer) + 1, 0);
        //free(send_buffer);

        // Receives a message from the server
        char reception_buffer[MAX_SIZE];
        printf("r_buffer : %ld\n", strlen(reception_buffer));
        recv(socket_descriptor, &reception_buffer, strlen(reception_buffer) + 1, 0);
        printf("Réponse : %s\n", reception_buffer);


      }
      else if (client_id == 2) {

        // Receives a message from the server
        char reception_buffer[MAX_SIZE];
        printf("r_buffer : %ld\n", strlen(reception_buffer));
        recv(socket_descriptor, &reception_buffer, strlen(reception_buffer) + 1, 0);
        printf("Réponse : %s\n", reception_buffer);

        // Sends a message to the server
         char send_buffer[MAX_SIZE];
        printf("Mon message => ");
        fgets(send_buffer, MAX_SIZE, stdin);
        send(socket_descriptor, send_buffer, strlen(send_buffer) + 1, 0);

      } else {
        printf("Mauvais id de client : %d\n", client_id);
      }
      //i++;

    //}

    shutdown(socket_descriptor, 2);
}

