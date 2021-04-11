#ifndef PROJET_FAR_UTILS_H
#define PROJET_FAR_UTILS_H
#endif

#define MAX_SIZE 10000
#define END_WORD "fin\n"
#define MAX_THREADS 10
#define MAX_CLIENTS 3

void check_error(int check, char *err_msg);

void terminate_program(int code);

void printf_red(char *message);
