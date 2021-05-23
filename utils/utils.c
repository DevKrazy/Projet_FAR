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

// Retourne le dernier terminal
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

// Créé un nouveau terminal
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

int getCommandId(char* command) {
  if (strcmp(command, "/fin\n") == 0){
    return 1;
  }
  else if (strcmp(command, "/file\n") == 0){
    return 2;
  }
  else if (strcmp(command, "/filesrv\n") == 0){
    return 3;
  }
  else if (strcmp(command, "/room\n") == 0){
    return 4;
  }
  else{
    return 0;
  }
}