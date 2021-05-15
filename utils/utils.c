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


/**
 * Gets the files of a given directory and stores the list inside the given buffer.
 * @param dir_name the directory d_name
 * @param buffer the buffer where we'll store the file list
 */
void list_files(char* dir_name, char** buffer) {
    DIR *dir_stream;
    struct dirent *dir_entry;
    dir_stream = opendir(dir_name);
    int realloc_size = 0;
    if (dir_stream != NULL) {
        while (dir_entry = readdir (dir_stream)) {
            if(strcmp(dir_entry->d_name, ".") != 0 && strcmp(dir_entry->d_name, "..") != 0) {
                // does not display the . and .. files
                realloc_size += strlen(dir_entry->d_name) + 2; // +2 for the \0 and the \n
                *buffer = realloc(*buffer, realloc_size);
                strcat(*buffer, dir_entry->d_name);
                strcat(*buffer, "\n");
            }
        }
        (void) closedir(dir_stream);
    } else {
        perror ("Ne peux pas ouvrir le répertoire");
    }
}