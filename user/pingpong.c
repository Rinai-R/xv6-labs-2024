#include "kernel/types.h"
#include "user/user.h"

int main(int argc, char *argv[]) {
    int p1[2];
    int p2[2];
    pipe(p1);
    pipe(p2);
    int pid1 = fork();
    if (pid1 == 0) {
        close(p1[1]);
        close(p2[0]);
        char buf[1];
        read(p1[0], buf, 1);
        close(p1[0]);
        printf("%d: received ping\n", getpid());
        write(p2[1], "x", 1);
        close(p2[1]);
        exit(0);
    }
    int pid2 = fork();
    if (pid2 == 0) {
        close(p1[0]);
        close(p2[1]);
        write(p1[1], "x", 1);
        close(p1[1]);
        char buf[1];
        read(p2[0], buf, 1);
        close(p2[0]);
        printf("%d: received pong\n", getpid());
        write(p1[1], "x", 1);
        close(p1[1]);
        exit(0);
    }
}