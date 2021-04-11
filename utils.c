#include <stdio.h>
#include <stdlib.h>
#include "headers/utils.h"

/**
 * Checks a given variable value and prints an error message if the variable value is -1.
 * @param check the variable to check
 * @param err_msg the message to print in case the variable is -1
 */
void check_error(int check, char *err_msg) {
    if (check == -1) {
        perror(err_msg);
        exit(-1);
    }
}

/**
 * Terminates the current program and prints a message.
 * @param code the return code
 */
void terminate_program(int code) {
    printf("Arrêt du programme (code : %d).\n", code);
    exit(code);
}

/**
 * Prints a message using a red color.
 * Current limitation: cannot print variables inside the message.
 * @param message the message to print
 */
void printf_red(char *message) {
    printf("\033[0;31m"); // red
    printf("%s", message);
    printf("\033[0m"); // white
}