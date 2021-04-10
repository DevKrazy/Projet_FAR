# This script compiles and launches the server on the given port (first arg)
clear
gcc -o client_new client_new.c utils.c
gcc -o serveur_new serveur_new.c utils.c
./serveur_new $1