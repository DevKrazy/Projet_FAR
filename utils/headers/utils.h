#ifndef PROJET_FAR_UTILS_H
#define PROJET_FAR_UTILS_H
#endif

#define MAX_MSG_SIZE 10000
#define MAX_FILE_SIZE 10000
#define MAX_NAME_SIZE 12
#define END_WORD "fin\n"

void check_error(int check, char *err_msg);

void terminate_program(int code);

void printf_red(char *message);

int value_in_array(char* val, char** arr);

