#ifndef PROJET_FAR_FILE_UTILS_H
#define PROJET_FAR_FILE_UTILS_H

#endif //PROJET_FAR_FILE_UTILS_H

void send_file(int socket, char* folder, char* filename);

void receive_file(int socket, char* folder);

void get_filename_to_send(char* dir, char** return_filename);