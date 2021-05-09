clear
gcc ../client.c -o client -lpthread ../utils/server_utils.c ../utils/utils.c
./client 127.0.0.1 $1