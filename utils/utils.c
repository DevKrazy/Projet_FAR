#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>
#include <string.h>
#include <pthread.h>
#include <dirent.h>

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
 *
 * @param val
 * @param arr
 * @return
 */
int value_in_array(char* val, char** arr) {
    int i;
    printf("%s", arr[0]);
    for(i = 0; i < sizeof(arr) / sizeof(arr[0]); i++) {
        if(strcmp(val, arr[i]) == 0) {
            return 1;
        }
    }
    return 0;
}

