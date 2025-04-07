        # qemu -kernel loads the kernel at 0x80000000
        # and causes each hart (i.e. CPU) to jump there.
        # kernel.ld causes the following code to
        # be placed at 0x80000000.

.section .text           # 指定代码段
.global _entry           # 将 _entry 定义为全局符号，表示程序的入口

_entry:                  # 程序入口，qemu 启动时首先执行的地方

        # 设置栈空间，栈的初始化为每个 CPU（或 hart）提供一个独立的栈
        # stack0 在 start.c 文件中声明，每个 CPU 使用 4096 字节的栈空间
        # 计算当前 CPU 的栈指针：sp = stack0 + (hartid * 4096)

        la sp, stack0      # 加载 stack0 的地址到栈指针寄存器 sp 中
        li a0, 1024*4      # 将 4096 字节（1 页内存）存入 a0 寄存器，用于计算栈偏移量
        csrr a1, mhartid   # 读取当前 hart（CPU）的 ID，存入 a1 寄存器
        addi a1, a1, 1     # hartid 加 1，确保每个 CPU 有独立的栈
        mul a0, a0, a1     # 计算当前 CPU 的栈偏移量：4096 * hartid，存入 a0
        add sp, sp, a0     # 将栈指针 sp 偏移到当前 CPU 应该使用的栈空间

        # 跳转到 C 语言中的 start 函数，开始操作系统的初始化
        call start          # 跳转到 start 函数，开始操作系统的初始化过程

spin:                      # 如果 start 函数意外结束（不会发生），进入无限循环
        j spin             # 无限跳转，防止程序继续执行不正确的代码
