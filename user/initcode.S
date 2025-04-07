# 初始进程，它执行 /init。
# 这段代码在用户空间运行。

#include "syscall.h"

# 执行 exec(init, argv)，启动 /init 进程
.globl start
start:
        # 将字符串 "/init" 的地址加载到 a0 寄存器。
        # a0 是 exec 系统调用的第一个参数，用于传递程序的路径。
        la a0, init

        # 将 argv 数组的地址加载到 a1 寄存器。
        # a1 是 exec 系统调用的第二个参数，用于传递程序的参数。
        la a1, argv

        # 将系统调用号 SYS_exec 加载到 a7 寄存器。
        # a7 是系统调用号，指示要执行的系统调用类型，这里表示 exec 系统调用。
        li a7, SYS_exec

        # 执行系统调用，a0, a1 和 a7 现在已经包含 exec 所需的参数。
        ecall

# 如果 exec 调用失败，进入死循环退出进程。
exit:
        # 将系统调用号 SYS_exit 加载到 a7 寄存器。
        # a7 表示系统调用号，这里表示退出进程。
        li a7, SYS_exit

        # 执行退出系统调用，结束当前进程。
        ecall

        # 如果退出失败，则进入死循环保持退出状态。
        # 永远不会返回。
        jal exit

# 定义字符串 "/init\0" 并将其作为 init 变量的值。
init:
  .string "/init\0"  # 以 null 字符结尾的字符串 "/init"

# 定义参数 argv 数组，指向字符串 init，最后一个元素为 0（数组结束符）。
.p2align 2  # 对齐到 2 字节边界，确保内存地址对齐
argv:
  .quad init  # 将 init 的地址存储为 argv 数组的第一个元素。
  .quad 0     # argv 数组的最后一个元素为 NULL，表示参数结束。
