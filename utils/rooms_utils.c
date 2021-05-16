#include "headers/rooms_utils.h"
#include "headers/server_utils.h"

void join_room(int client_id, int room_id, Client clients[], Room rooms[]) {
    clients[client_id].client_room_socket = accept_client(rooms[room_id].socket_room_server);

}

