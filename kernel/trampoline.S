        #
        # low-level code to handle traps from user space into
        # the kernel, and returns from kernel to user.
        #
        # the kernel maps the page holding this code
        # at the same virtual address (TRAMPOLINE)
        # in user and kernel space so that it continues
        # to work when it switches page tables.
        # kernel.ld causes this code to start at 
        # a page boundary.
        #

#include "riscv.h"
#include "memlayout.h"

.section trampsec
.globl trampoline
.globl usertrap
trampoline:
.align 4
.globl uservec
uservec:    
        #
        # trap.c sets stvec to point here, so
        # traps from user space start here,
        # in supervisor mode, but with a
        # user page table.
        #

        # 保存用户空间传入的 a0 寄存器值到 sscratch 寄存器，
        # 因为 a0 会用于获取 TRAPFRAME 地址。
        csrw sscratch, a0

        # 设置 a0 为 TRAPFRAME 的地址，
        # 每个进程都有一个单独的 trapframe 内存区域，
        # 但是在每个进程的用户页表中，该区域被映射到相同的虚拟地址。
        li a0, TRAPFRAME
        
        # 将用户程序的寄存器保存到 TRAPFRAME 中，
        # 这些寄存器会在陷阱发生时被修改，返回时需要恢复。
        sd ra, 40(a0)  # 保存 ra 寄存器
        sd sp, 48(a0)  # 保存 sp 寄存器
        sd gp, 56(a0)  # 保存 gp 寄存器
        sd tp, 64(a0)  # 保存 tp 寄存器
        sd t0, 72(a0)  # 保存 t0 寄存器
        sd t1, 80(a0)  # 保存 t1 寄存器
        sd t2, 88(a0)  # 保存 t2 寄存器
        sd s0, 96(a0)  # 保存 s0 寄存器
        sd s1, 104(a0) # 保存 s1 寄存器
        sd a1, 120(a0) # 保存 a1 寄存器
        sd a2, 128(a0) # 保存 a2 寄存器
        sd a3, 136(a0) # 保存 a3 寄存器
        sd a4, 144(a0) # 保存 a4 寄存器
        sd a5, 152(a0) # 保存 a5 寄存器
        sd a6, 160(a0) # 保存 a6 寄存器
        sd a7, 168(a0) # 保存 a7 寄存器
        sd s2, 176(a0) # 保存 s2 寄存器
        sd s3, 184(a0) # 保存 s3 寄存器
        sd s4, 192(a0) # 保存 s4 寄存器
        sd s5, 200(a0) # 保存 s5 寄存器
        sd s6, 208(a0) # 保存 s6 寄存器
        sd s7, 216(a0) # 保存 s7 寄存器
        sd s8, 224(a0) # 保存 s8 寄存器
        sd s9, 232(a0) # 保存 s9 寄存器
        sd s10, 240(a0) # 保存 s10 寄存器
        sd s11, 248(a0) # 保存 s11 寄存器
        sd t3, 256(a0) # 保存 t3 寄存器
        sd t4, 264(a0) # 保存 t4 寄存器
        sd t5, 272(a0) # 保存 t5 寄存器
        sd t6, 280(a0) # 保存 t6 寄存器

        # 将之前保存的用户空间的 a0 值（即 TRAPFRAME 的地址）重新保存到 p->trapframe->a0。
        csrr t0, sscratch
        sd t0, 112(a0)

        # 从 p->trapframe->kernel_sp 加载内核栈指针，并设置 sp。
        ld sp, 8(a0)

        # 从 p->trapframe->kernel_hartid 加载当前的 hartid，设置 tp。
        ld tp, 32(a0)

        # 从 p->trapframe->kernel_trap 加载 usertrap 函数的地址，并跳转到 usertrap。
        ld t0, 16(a0)

        # 从 p->trapframe->kernel_satp 获取内核页表地址，设置 satp 寄存器。
        ld t1, 0(a0)

        # 等待先前的内存操作完成，以便后续的操作使用用户页表。
        sfence.vma zero, zero

        # 安装内核页表（切换到内核页表）。
        csrw satp, t1

        # 刷新 TLB 缓存，移除过时的用户页表条目。
        sfence.vma zero, zero

        # 跳转到 usertrap 函数，该函数不会返回。
        jr t0

.globl userret
userret:
        # userret(pagetable)
        # usertrapret() 函数调用时会切换回用户模式
        # a0: 用户页表，用于 satp。

        # 切换到用户页表。
        sfence.vma zero, zero
        csrw satp, a0
        sfence.vma zero, zero

        li a0, TRAPFRAME

        # 从 TRAPFRAME 中恢复用户寄存器，恢复所有寄存器状态（除了 a0）。
        ld ra, 40(a0)  # 恢复 ra 寄存器
        ld sp, 48(a0)  # 恢复 sp 寄存器
        ld gp, 56(a0)  # 恢复 gp 寄存器
        ld tp, 64(a0)  # 恢复 tp 寄存器
        ld t0, 72(a0)  # 恢复 t0 寄存器
        ld t1, 80(a0)  # 恢复 t1 寄存器
        ld t2, 88(a0)  # 恢复 t2 寄存器
        ld s0, 96(a0)  # 恢复 s0 寄存器
        ld s1, 104(a0) # 恢复 s1 寄存器
        ld a1, 120(a0) # 恢复 a1 寄存器
        ld a2, 128(a0) # 恢复 a2 寄存器
        ld a3, 136(a0) # 恢复 a3 寄存器
        ld a4, 144(a0) # 恢复 a4 寄存器
        ld a5, 152(a0) # 恢复 a5 寄存器
        ld a6, 160(a0) # 恢复 a6 寄存器
        ld a7, 168(a0) # 恢复 a7 寄存器
        ld s2, 176(a0) # 恢复 s2 寄存器
        ld s3, 184(a0) # 恢复 s3 寄存器
        ld s4, 192(a0) # 恢复 s4 寄存器
        ld s5, 200(a0) # 恢复 s5 寄存器
        ld s6, 208(a0) # 恢复 s6 寄存器
        ld s7, 216(a0) # 恢复 s7 寄存器
        ld s8, 224(a0) # 恢复 s8 寄存器
        ld s9, 232(a0) # 恢复 s9 寄存器
        ld s10, 240(a0) # 恢复 s10 寄存器
        ld s11, 248(a0) # 恢复 s11 寄存器
        ld t3, 256(a0) # 恢复 t3 寄存器
        ld t4, 264(a0) # 恢复 t4 寄存器
        ld t5, 272(a0) # 恢复 t5 寄存器
        ld t6, 280(a0) # 恢复 t6 寄存器

        # 恢复用户的 a0 寄存器值。
        ld a0, 112(a0)
        
        # 返回到用户模式，并且返回到用户程序计数器（pc）。
        # usertrapret() 会设置好 sstatus 和 sepc。
        sret  # 执行返回到用户模式的操作。
