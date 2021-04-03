#include <stdio.h>
#include <string.h>

int main() {
    char buffer[32];
    printf("buffer size %ld\n", sizeof(buffer));
    printf("buffer strlen %ld\n", strlen(buffer));
    return 0;
}
