        #
        # interrupts and exceptions while in supervisor
        # mode come here.
        #
        # 当前栈是内核栈，发生中断或异常时将跳转到这里。
        # 保存寄存器，调用内核的 trap 处理函数 kerneltrap()。
        # 当 kerneltrap() 返回时，恢复寄存器状态并返回。
        #

.globl kerneltrap      # 声明 kerneltrap 为全局符号，方便 C 语言代码调用。
.globl kernelvec       # 声明 kernelvec 为全局符号，表示中断处理向量。
.align 4               # 保证代码按 4 字节对齐。

kernelvec:             # 中断/异常处理向量，跳转到此处时开始执行

        # make room to save registers.
        # 为保存寄存器做空间，保存必要的寄存器内容。
        addi sp, sp, -256      # 调整栈指针，向栈中压入 256 字节空间，用于保存寄存器。

        # save caller-saved registers.
        # 保存调用者保存寄存器（即被中断或异常可能改变的寄存器）。
        sd ra, 0(sp)           # 保存返回地址寄存器 ra（返回地址）。
        sd sp, 8(sp)           # 保存当前栈指针 sp。
        sd gp, 16(sp)          # 保存全局指针 gp。
        sd tp, 24(sp)          # 保存线程指针 tp（用于标识当前 hart）。
        sd t0, 32(sp)          # 保存临时寄存器 t0。
        sd t1, 40(sp)          # 保存临时寄存器 t1。
        sd t2, 48(sp)          # 保存临时寄存器 t2。
        sd a0, 72(sp)          # 保存参数寄存器 a0。
        sd a1, 80(sp)          # 保存参数寄存器 a1。
        sd a2, 88(sp)          # 保存参数寄存器 a2。
        sd a3, 96(sp)          # 保存参数寄存器 a3。
        sd a4, 104(sp)         # 保存参数寄存器 a4。
        sd a5, 112(sp)         # 保存参数寄存器 a5。
        sd a6, 120(sp)         # 保存参数寄存器 a6。
        sd a7, 128(sp)         # 保存参数寄存器 a7。
        sd t3, 216(sp)         # 保存临时寄存器 t3。
        sd t4, 224(sp)         # 保存临时寄存器 t4。
        sd t5, 232(sp)         # 保存临时寄存器 t5。
        sd t6, 240(sp)         # 保存临时寄存器 t6。

        # call the C trap handler in trap.c
        # 调用 C 语言的中断处理函数 kerneltrap()，该函数处理具体的中断/异常。
        call kerneltrap

        # restore registers.
        # 从栈中恢复寄存器的值，恢复到中断之前的状态。

        ld ra, 0(sp)           # 恢复返回地址寄存器 ra。
        ld sp, 8(sp)           # 恢复栈指针寄存器 sp。
        ld gp, 16(sp)          # 恢复全局指针寄存器 gp。
        # tp 寄存器包含 hartid，可能由于 CPU 切换而需要手动设置，所以不恢复 tp。
        ld t0, 32(sp)          # 恢复临时寄存器 t0。
        ld t1, 40(sp)          # 恢复临时寄存器 t1。
        ld t2, 48(sp)          # 恢复临时寄存器 t2。
        ld a0, 72(sp)          # 恢复参数寄存器 a0。
        ld a1, 80(sp)          # 恢复参数寄存器 a1。
        ld a2, 88(sp)          # 恢复参数寄存器 a2。
        ld a3, 96(sp)          # 恢复参数寄存器 a3。
        ld a4, 104(sp)         # 恢复参数寄存器 a4。
        ld a5, 112(sp)         # 恢复参数寄存器 a5。
        ld a6, 120(sp)         # 恢复参数寄存器 a6。
        ld a7, 128(sp)         # 恢复参数寄存器 a7。
        ld t3, 216(sp)         # 恢复临时寄存器 t3。
        ld t4, 224(sp)         # 恢复临时寄存器 t4。
        ld t5, 232(sp)         # 恢复临时寄存器 t5。
        ld t6, 240(sp)         # 恢复临时寄存器 t6。

        addi sp, sp, 256       # 恢复栈指针 sp，弹出栈空间，恢复栈状态。

        # return to whatever we were doing in the kernel.
        # 返回到内核之前的执行状态，继续执行内核任务。
        sret                    # 执行 `sret` 指令返回到上次的模式（在此情况下为 Supervisor 模式）。
