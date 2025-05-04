// Physical memory allocator, for user processes,
// kernel stacks, page-table pages,
// and pipe buffers. Allocates whole 4096-byte pages.

#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "riscv.h"
#include "defs.h"
#include "proc.h"

void freerange(void *pa_start, void *pa_end);

extern char end[]; // first address after kernel.
                   // defined by kernel.ld.
struct run {
  struct run *next;
};

struct {
  struct spinlock lock;
  struct run *freelist;
} kmem[NCPU];

struct spinlock kmem_lock;

char* kmem_lock_name[NCPU] = {
  "kmem1", "kmem2", "kmem3", "kmem4", "kmem5", "kmem6", "kmem7", "kmem8"
};

void
kinit()
{
  initlock(&kmem_lock, "kmem_lock");
  for(int i = 0; i < NCPU; i++) {
    initlock(&kmem[i].lock, kmem_lock_name[i]);
  }
  freerange(end, (void*)PHYSTOP);
}

void
freerange(void *pa_start, void *pa_end)
{
  char *p;
  p = (char*)PGROUNDUP((uint64)pa_start);
  for(; p + PGSIZE <= (char*)pa_end; p += PGSIZE)
    kfree(p);
}

// Free the page of physical memory pointed at by pa,
// which normally should have been returned by a
// call to kalloc().  (The exception is when
// initializing the allocator; see kinit above.)
void
kfree(void *pa)
{
  struct run *r;

  if(((uint64)pa % PGSIZE) != 0 || (char*)pa < end || (uint64)pa >= PHYSTOP)
    panic("kfree");

  // Fill with junk to catch dangling refs.
  memset(pa, 1, PGSIZE);

  r = (struct run*)pa;

  push_off();
  int cpu = cpuid();
  pop_off();
  acquire(&kmem[cpu].lock);
  r->next = kmem[cpu].freelist;
  kmem[cpu].freelist = r;
  release(&kmem[cpu].lock);
}

// Allocate one 4096-byte page of physical memory.
// Returns a pointer that the kernel can use.
// Returns 0 if the memory cannot be allocated.
void *
kalloc(void)
{
  struct run *r;
  push_off();
  int cpu = cpuid();
  pop_off();
  acquire(&kmem[cpu].lock);

  r = kmem[cpu].freelist;

  if(!r) {
    int steal_pages = 0;
    for(int i = 0; i < NCPU; i++) {
      if (i == cpu) continue;
      acquire(&kmem[i].lock);
      while (kmem[i].freelist) {
        // 处理被窃取的链表
        struct run* newpage = kmem[i].freelist;
        kmem[i].freelist = newpage->next; 
        release(&kmem[i].lock);
        // 处理窃取的链表
        // acquire(&kmem[cpu].lock);
        newpage->next = kmem[cpu].freelist;
        kmem[cpu].freelist = newpage;
        steal_pages ++;
        // release(&kmem[cpu].lock);
        // 窃取了 32 个页面，足够了，就不再偷了。
        if(steal_pages == 128) {
          goto done;
        }
        acquire(&kmem[i].lock);
      }
      release(&kmem[i].lock);
    }
    // 如果为 0，说明没有别的 CPU 有空闲页面，需要 panic。
    if(steal_pages == 0) {
      release(&kmem[cpu].lock);
      return 0;
    }
  }
done:
  // acquire(&kmem[cpu].lock);
  r = kmem[cpu].freelist;

  if(r)
    kmem[cpu].freelist = r->next;


  release(&kmem[cpu].lock);

  if(r)
    memset((char*)r, 5, PGSIZE); // fill with junk
  return (void*)r;
}
