#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(1, "Usage: sleep seconds\n");
        exit(1);
    }   
    int pid = fork();
    if (pid == 0) {
        unsigned int seconds = atoi(argv[1]);
        sleep(seconds * 10);
        exit(0);
    } else {
        wait(0);
    }
    exit(0);
}