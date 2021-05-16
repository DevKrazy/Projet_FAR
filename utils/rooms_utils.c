#include <stdio.h>
#include <string.h>
#include "headers/rooms_utils.h"
#include "headers/server_utils.h"


void create_room(int max_members, char room_name[20], int port, int index, Room rooms[]) {
    rooms[index].num_port = port;
    rooms[index].nb_max_membre = max_members;

    // socket creation
    configure_listening_socket(rooms[index].num_port, &rooms[index].socket_room_server, &rooms[index].room_address);
    bind_and_listen_on(rooms[index].socket_room_server, rooms[index].room_address);

    // name configuration
    char nom[20];
    if (index == 0) {
        strcpy(nom, "Général");
    } else {
        strcpy(nom, "Salon N°");
    }
    char num[MAX_MSG_SIZE];
    sprintf(num, "%d", index); // writes the "w" value inside the num
    strcat(nom, num);
    strcpy(rooms[index].room_name, nom);
}

void join_room(int client_id, int room_id, Client clients[], Room rooms[]) {
    clients[client_id].client_room_socket = accept_client(rooms[room_id].socket_room_server);
    clients[client_id].room_id = room_id;
    rooms[room_id].membres[client_id] = 1;
}

void leave_room(int client_id, Client clients[], Room rooms[]) {
    join_room(client_id, 0, clients, rooms); // joins the general room
}

