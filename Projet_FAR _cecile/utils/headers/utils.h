#ifndef PROJET_FAR_UTILS_H
#define PROJET_FAR_UTILS_H
#endif

#define MAX_MSG_SIZE 10000
#define MAX_FILE_SIZE 10000
#define MAX_NAME_SIZE 12
#define END_WORD "fin\n"

void check_error(int check, char *err_msg);

void terminate_program(int code);

int get_last_tty();

FILE* new_tty();

//char* envoi_fichier();

void printf_red(char *message);