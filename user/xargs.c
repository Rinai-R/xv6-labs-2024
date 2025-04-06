#include "kernel/types.h"
#include "user/user.h"
#include "kernel/param.h"

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(2, "Usage: xargs command [args...]\n");
        exit(1);
    }
    char *cmd = argv[1];
    char *args[MAXARG];
    int i, n = 0;

    // 复制参数
    for (i = 1; i < argc && n < MAXARG - 1; i++) {
        args[n++] = argv[i];
    }
    int end = n;
    //方便重置索引
    char buf[512];
    int m = 0;
    while (read(0, &buf[m], 1) == 1) {
        if (buf[m] == '\n') {
            buf[m] = 0;
            args[n++] = &buf[0];

            // 参数必须以 NULL 结尾
            args[n] = 0;

            int fd = fork();
            if (fd == 0) {
                exec(cmd, args);
                fprintf(2, "xargs: exec failed\n");
                exit(1);
            }
            wait(0);

            // 索引重置
            m = 0;
            n = end;
        } else {
            m++;
        }
    }
    exit(0);
}