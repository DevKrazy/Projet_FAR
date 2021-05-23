cmake_minimum_required(VERSION 3.17)
project(Projet_FAR C)

set(CMAKE_C_STANDARD 99)

add_executable(Projet_FAR server.c client.c utils/utils.c utils/headers/utils.h utils/server_utils.c utils/headers/server_utils.h utils/client_utils.c utils/headers/client_utils.h utils/file_utils.c utils/headers/file_utils.h utils/headers/rooms_utils.h)