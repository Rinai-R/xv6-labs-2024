#include "kernel/types.h"
#include "user/user.h"

void primes(int p0[2]) __attribute__((noreturn));

int main(int argc, char *argv[]) {
    int p[2];
    pipe(p);
    int pid = fork();
    if (pid == 0) {
        //管道的关闭逻辑在primes函数中
        primes(p);
    } else {
        close(p[0]);
        for (int i = 2; i <= 280; i++) {
            write(p[1], &i, sizeof(i));
        }
        close(p[1]);
        wait(0);
    }
    exit(0);
}


void primes(int old_pipe[2]) {
    //及时释放管道
    close(0);
    dup(old_pipe[0]);
    close(old_pipe[0]);
    close(old_pipe[1]);


    int prime;
    if (read(0, &prime, sizeof(prime)) == 0) {
        close(0);
        exit(0);
    }
    printf("prime %d\n", prime);
    //新建管道，并fork子进程
    int new_pipe[2];
    pipe(new_pipe);
    int pid = fork();
    if (pid == 0) {
        primes(new_pipe);
    } else {
        close(new_pipe[0]);
        int num;
        while (read(0, &num, sizeof(num))) {
            if (num % prime != 0) {
                write(new_pipe[1], &num, sizeof(num));
            }
        }
        close(0);
        close(new_pipe[1]);
        wait(0);
    }
    exit(0);
}