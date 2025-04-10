// 内核上下文切换时保存的寄存器
struct context {
  uint64 ra;  // 返回地址寄存器，保存函数调用返回地址
  uint64 sp;  // 栈指针，保存当前栈的位置

  // 被调用者保存的寄存器
  uint64 s0;  // 保存s0寄存器的值
  uint64 s1;  // 保存s1寄存器的值
  uint64 s2;  // 保存s2寄存器的值
  uint64 s3;  // 保存s3寄存器的值
  uint64 s4;  // 保存s4寄存器的值
  uint64 s5;  // 保存s5寄存器的值
  uint64 s6;  // 保存s6寄存器的值
  uint64 s7;  // 保存s7寄存器的值
  uint64 s8;  // 保存s8寄存器的值
  uint64 s9;  // 保存s9寄存器的值
  uint64 s10; // 保存s10寄存器的值
  uint64 s11; // 保存s11寄存器的值
};

// 每个 CPU 的状态
struct cpu {
  struct proc *proc;        // 当前在此 CPU 上运行的进程，如果没有则为 null
  struct context context;   // 当前进程的上下文，切换到 scheduler() 时使用
  int noff;                 // push_off() 嵌套的深度
  int intena;               // 在 push_off() 前是否启用了中断
};

extern struct cpu cpus[NCPU];  // 定义每个 CPU 对应的结构体

// 每个进程的 trap 处理代码的数据结构，存储在 trampoline.S 中。
// 该结构体位于用户页表中，紧邻 trampoline 页，
// 在内核页表中没有特殊映射。
// uservec 在 trampoline.S 中保存用户的寄存器到 trapframe 中，
// 然后从 trapframe 初始化 kernel_sp, kernel_hartid, kernel_satp，
// 并跳转到 kernel_trap。
// usertrapret() 和 userret 在 trampoline.S 中设置 trapframe 中的 kernel_*，
// 恢复用户的寄存器，切换到用户页表，并进入用户空间。
// trapframe 包含了被调用者保存的用户寄存器（如 s0-s11），
// 因为通过 usertrapret() 返回用户时不通过整个内核调用栈。
struct trapframe {
  /*   0 */ uint64 kernel_satp;   // 内核页表地址
  /*   8 */ uint64 kernel_sp;     // 进程内核栈的顶部
  /*  16 */ uint64 kernel_trap;   // 跳转到的内核陷阱函数 usertrap()
  /*  24 */ uint64 epc;           // 保存的用户程序计数器（程序计数器）
  /*  32 */ uint64 kernel_hartid; // 保存的内核线程编号
  /*  40 */ uint64 ra;            // 返回地址寄存器
  /*  48 */ uint64 sp;            // 栈指针
  /*  56 */ uint64 gp;            // 全局指针
  /*  64 */ uint64 tp;            // 线程指针
  /*  72 */ uint64 t0;            // 临时寄存器 t0
  /*  80 */ uint64 t1;            // 临时寄存器 t1
  /*  88 */ uint64 t2;            // 临时寄存器 t2
  /*  96 */ uint64 s0;            // 保存的 s0 寄存器
  /* 104 */ uint64 s1;            // 保存的 s1 寄存器
  /* 112 */ uint64 a0;            // 参数寄存器 a0
  /* 120 */ uint64 a1;            // 参数寄存器 a1
  /* 128 */ uint64 a2;            // 参数寄存器 a2
  /* 136 */ uint64 a3;            // 参数寄存器 a3
  /* 144 */ uint64 a4;            // 参数寄存器 a4
  /* 152 */ uint64 a5;            // 参数寄存器 a5
  /* 160 */ uint64 a6;            // 参数寄存器 a6
  /* 168 */ uint64 a7;            // 参数寄存器 a7
  /* 176 */ uint64 s2;            // 保存的 s2 寄存器
  /* 184 */ uint64 s3;            // 保存的 s3 寄存器
  /* 192 */ uint64 s4;            // 保存的 s4 寄存器
  /* 200 */ uint64 s5;            // 保存的 s5 寄存器
  /* 208 */ uint64 s6;            // 保存的 s6 寄存器
  /* 216 */ uint64 s7;            // 保存的 s7 寄存器
  /* 224 */ uint64 s8;            // 保存的 s8 寄存器
  /* 232 */ uint64 s9;            // 保存的 s9 寄存器
  /* 240 */ uint64 s10;           // 保存的 s10 寄存器
  /* 248 */ uint64 s11;           // 保存的 s11 寄存器
  /* 256 */ uint64 t3;            // 临时寄存器 t3
  /* 264 */ uint64 t4;            // 临时寄存器 t4
  /* 272 */ uint64 t5;            // 临时寄存器 t5
  /* 280 */ uint64 t6;            // 临时寄存器 t6
};

enum procstate { UNUSED, USED, SLEEPING, RUNNABLE, RUNNING, ZOMBIE };

// 每个进程的状态
struct proc {
  struct spinlock lock;       // 保护进程状态的锁

  // 在使用以下字段时必须持有 p->lock 锁：
  enum procstate state;        // 进程的当前状态
  void *chan;                  // 如果非零，表示进程正在 chan 上睡眠
  int killed;                  // 如果非零，表示该进程已经被杀死
  int xstate;                  // 进程退出时返回给父进程的退出状态
  int pid;                     // 进程 ID
  int tracing;                 // 跟踪系统调用的掩码

  // 必须持有 wait_lock 锁才能使用以下字段：
  struct proc *parent;         // 父进程

  // 这些字段是进程私有的，因此不需要持有 p->lock 锁：
  uint64 kstack;               // 内核栈的虚拟地址
  uint64 sz;                   // 进程内存大小（字节数）
  pagetable_t pagetable;       // 用户页表
  struct trapframe *trapframe; // trampoline.S 中使用的数据页面
  struct context context;      // 切换到该进程时的上下文
  struct file *ofile[NOFILE];  // 打开的文件
  struct inode *cwd;           // 当前工作目录
  char name[16];               // 进程名称（用于调试）
};
