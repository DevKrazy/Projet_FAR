cmake_minimum_required(VERSION 3.17)
project(Projet_FAR C)

set(CMAKE_C_STANDARD 99)

add_executable(Projet_FAR server/server.c client/client.c common/utils.c common/headers/utils.h server/server_utils.c server/headers/server_utils.h client/client_utils.c client/headers/client_utils.h common/file_utils.c common/headers/file_utils.h)