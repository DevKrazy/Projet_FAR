#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main(int argc, char *argv[]) {

    int dS = socket(PF_INET, SOCK_STREAM, 0);

    struct sockaddr_in ad;
    ad.sin_family = AF_INET;
    ad.sin_addr.s_addr = INADDR_ANY ;
    ad.sin_port = htons(atoi(argv[1])) ;

    bind(dS, (struct sockaddr*)&ad, sizeof(ad)) ;
    listen(dS, 7) ;

    //client 1
    struct sockaddr_in aC ;
    socklen_t lg = sizeof(struct sockaddr_in) ;
    int dSC= accept(dS, (struct sockaddr*) &aC,&lg) ;
    //client 2
    struct sockaddr_in aC1 ;
    socklen_t lg1 = sizeof(struct sockaddr_in) ;
    int dSC1 = accept(dS, (struct sockaddr*) &aC1,&lg1) ;

    // Réception
    char msg [32] ; // buffer reception
    sleep(2);
    ssize_t res = recv(dSC, msg, 32, 0) ;//reception message client 1
    printf("recu : %s \n", msg) ;
    send(dSC1, msg, 32, 0) ; // Envoi du message au 2e client
    ssize_t res1 = recv(dSC1, msg, 32, 0) ;//reception message 2e client
    printf("recu : %s \n", msg) ;
    send(dSC, msg, 32, 0) ; //envoie au client 1
    shutdown(dSC1, 2) ;
    shutdown(dSC, 2) ;
    shutdown(dS, 2) ;


}