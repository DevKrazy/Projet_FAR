clear
gcc ../client.c -o client -lpthread ../utils/utils.c
./client 127.0.0.1 $1