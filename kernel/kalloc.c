// Physical memory allocator, for user processes,
// kernel stacks, page-table pages,
// and pipe buffers. Allocates whole 4096-byte pages.

#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "riscv.h"
#include "defs.h"

void freerange(void *pa_start, void *pa_end);

extern char end[]; // first address after kernel.
                   // defined by kernel.ld.

struct run {
  struct run *next;
};

struct {
  struct spinlock lock;
  struct run *freelist;
} kmem;

void
kinit()
{
  initlock(&kmem.lock, "kmem");
  freerange(end, (void*)PHYSTOP);
}



void
freerange(void *pa_start, void *pa_end)
{
  char *p;
  p = (char*)PGROUNDUP((uint64)pa_start);
  for(; p + PGSIZE <= (char*)pa_end; p += PGSIZE) {
    kfree(p);
  }
}

// Free the page of physical memory pointed at by pa,
// which normally should have been returned by a
// call to kalloc().  (The exception is when
// initializing the allocator; see kinit above.)
void kfree(void *pa)
{
  struct run *r;

  // 检查传入的地址是否有效：
  // 1. 检查地址是否为页对齐，PGSIZE通常是4KB。
  // 2. 检查地址是否在有效的物理内存范围内，即大于等于内核的结束地址`end`，小于物理内存的上限`PHYSTOP`。
  if(((uint64)pa % PGSIZE) != 0 || (char*)pa < end || (uint64)pa >= PHYSTOP)
    panic("kfree");  // 如果条件不满足，则触发内存错误，终止程序。

#ifndef LAB_SYSCALL
  // 如果没有定义`LAB_SYSCALL`，填充释放的内存页为1（调试用）。
  // 这样可以帮助检测是否有程序继续访问已释放的内存。
  memset(pa, 1, PGSIZE);
#endif
  
  // 将释放的内存块转换为`struct run*`，用于操作空闲链表。
  r = (struct run*)pa;

  // 获取内存锁，以确保访问空闲链表时是线程安全的。
  acquire(&kmem.lock);

  // 将当前内存块（r）插入到空闲链表的头部。
  r->next = kmem.freelist;
  kmem.freelist = r;

  // 释放内存锁，其他线程可以继续操作空闲链表。
  release(&kmem.lock);
}



// Allocate one 4096-byte page of physical memory.
// Returns a pointer that the kernel can use.
// Returns 0 if the memory cannot be allocated.
void *
kalloc(void)
{
  struct run *r;

  acquire(&kmem.lock);
  r = kmem.freelist;
  if(r) {
    kmem.freelist = r->next;
  }
  release(&kmem.lock);
#ifndef LAB_SYSCALL
  if(r)
    memset((char*)r, 5, PGSIZE); // fill with junk
#endif
  return (void*)r;
}

