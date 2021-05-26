clear
#lsof -nti:"$1" | xargs kill -9 # used to kill the process running on the $1 port
gcc ../server.c -o server -lpthread ../server_utils.c ../../common/utils.c ../../common/file_utils.c
./server "$1"
