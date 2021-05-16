clear
#lsof -nti:"$1" | xargs kill -9 # used to kill the process running on the $1 port
gcc ../server.c -o server -lpthread ../utils/server_utils.c ../utils/utils.c ../utils/file_utils.c ../utils/rooms_utils.c
./server "$1"
