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

/*
 * CODE GIVEN BY THE PROFESSOR
 */
int get_last_tty() {
    FILE *fp;
    char path[1035];
    fp = popen("/bin/ls /dev/pts", "r");
    if (fp == NULL) {
        printf("Impossible d'exécuter la commande\n" );
        exit(1);
    }
    int i = INT_MIN;
    while (fgets(path, sizeof(path)-1, fp) != NULL) {
        if(strcmp(path,"ptmx")!=0){
            int tty = atoi(path);
            if(tty > i) i = tty;
        }
    }

    pclose(fp);
    return i;
}

/*
 * CODE GIVEN BY THE PROFESSOR
 */
FILE* new_tty() {
    pthread_mutex_t the_mutex;
    pthread_mutex_init(&the_mutex,0);
    pthread_mutex_lock(&the_mutex);
    system("gnome-terminal");
    sleep(1);
    char *tty_name = ttyname(STDIN_FILENO);
    int ltty = get_last_tty();
    char str[2];
    sprintf(str,"%d",ltty);
    int i;
    for(i = strlen(tty_name)-1; i >= 0; i--) {
        if(tty_name[i] == '/') break;
    }
    tty_name[i+1] = '\0';
    strcat(tty_name,str);
    FILE *fp = fopen(tty_name,"wb+");
    pthread_mutex_unlock(&the_mutex);
    pthread_mutex_destroy(&the_mutex);
    return fp;
}
