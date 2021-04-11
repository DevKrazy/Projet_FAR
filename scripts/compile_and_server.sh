# This script compiles and launches the server on the given port (first arg)
clear
#lsof -nti:"$1" | xargs kill -9 # used to kill the process running on the $1 port
gcc ../client.c -o client -lpthread ../utils.c
gcc ../server.c -o server -lpthread ../utils.c
./server "$1"
