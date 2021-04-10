cmake_minimum_required(VERSION 3.17)
project(Projet_FAR C)

set(CMAKE_C_STANDARD 99)

add_executable(Projet_FAR main.c serveur.c client.c comm_utils.c comm_utils.h remake/serveur_new.c remake/client_new.c remake/utils.c remake/utils.h threads/client_thread.c threads/serveur_thread.c)