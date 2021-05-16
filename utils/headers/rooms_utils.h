#include "server_utils.h"

#ifndef PROJET_FAR_ROOMS_UTILS_H
#define PROJET_FAR_ROOMS_UTILS_H
#endif

void create_room(int max_members, char room_name[20], int port, int index, Room rooms[]);

void join_room(int client_id, int room_id, Client clients[], Room rooms[]);

void leave_room(int client_id, Client clients[], Room rooms[]);
