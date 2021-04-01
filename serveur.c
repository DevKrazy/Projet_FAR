#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main(int argc, char *argv[]) {

    int socket_descriptor = socket(PF_INET, SOCK_STREAM, 0);

    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET; // address type
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons(atoi(argv[1])); // address port (converted from the CLI)

    bind(socket_descriptor, (struct sockaddr*)&server_address, sizeof(server_address)); // binds the configured address to the socket descriptor
    listen(socket_descriptor, 7); // listens for incoming connections

    // client1 address config
    struct sockaddr_in client1_address;
    socklen_t client1_address_len = sizeof(struct sockaddr_in);
    // if accept() is successful, creates a new socket descriptor already connected
    int client1_socket_descriptor = accept(socket_descriptor, (struct sockaddr*) &client1_address, &client1_address_len);

    int clientid1=1;
    send(client1_socket_descriptor, &clientid1, 4, 0);
    printf("id1 envoyé au client 1 : \n");

    // client2 address config
    struct sockaddr_in client2_address;
    socklen_t client2_address_len = sizeof(struct sockaddr_in);
    // if accept() is successful, creates a new socket descriptor already connected
    int client2_socket_descriptor = accept(socket_descriptor, (struct sockaddr*) &client2_address, &client2_address_len);

    int clientid2=2;
    send(client2_socket_descriptor, &clientid2, 4, 0);
    printf("id2 envoyé au client 2 : \n");


    // Réception du client1


    char reception_buffer[32]; //buffer reception

    sleep(1);
    recv(client1_socket_descriptor, reception_buffer, 32, 0); //reception message client 1
    printf("Reçu du client1 : %s \n", reception_buffer);

    // Envoi au client2
    int nb = (int)send(client2_socket_descriptor, reception_buffer, 32, 0);
    printf("envoyé du client1 : %s %d \n", reception_buffer,nb);

    //printf("Envoyé au client2 : %s \n", reception_buffer);

    recv(client2_socket_descriptor, reception_buffer, 32, 0);//reception message 2e client
    printf("Reçu de client2: %s \n", reception_buffer) ;
    int nb2 = (int)send(client1_socket_descriptor, reception_buffer, 32, 0) ; //send to client 1
    printf("envoye du client2 : %s %d \n", reception_buffer,nb2);

    recv(client1_socket_descriptor, reception_buffer, 32, 0); //reception message client 1
    printf("Reçu du client1 : %s \n", reception_buffer);

    // Envoi au client2
    send(client2_socket_descriptor, reception_buffer, 32, 0);
    printf("envoyé du client1 : %s %d \n", reception_buffer,nb);

    //printf("Envoyé au client2 : %s \n", reception_buffer);

    recv(client2_socket_descriptor, reception_buffer, 32, 0);//reception message 2e client
    printf("Reçu de client2: %s \n", reception_buffer) ;
    send(client1_socket_descriptor, reception_buffer, 32, 0) ; //send to client 1
    printf("envoye du client2 : %s %d \n", reception_buffer,nb2);


    shutdown(client2_socket_descriptor, 2) ;
    shutdown(client1_socket_descriptor, 2) ;
    shutdown(socket_descriptor, 2) ;
}