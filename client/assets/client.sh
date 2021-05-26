clear
gcc ../client.c -o client -lpthread ../client_utils.c ../../common/utils.c ../../common/file_utils.c
./client $1 $2