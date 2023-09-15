#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h> // strerror.
#include <errno.h>  // errno.

int main(int argc, char *argv[]) {
    int retval = EXIT_SUCCESS;
    if (argc != 2) {
        printf("One argument required only\n");
        retval = 1;
    } else {
        if (access(argv[1],F_OK) == 0) {
            printf("Filename: %s \n",argv[1]);
            retval = 0;
        } else {
            printf("File does not exist");
            retval = 1;
        }
    }
    return retval;
}
