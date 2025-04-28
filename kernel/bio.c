// Buffer cache.
//
// The buffer cache is a linked list of buf structures holding
// cached copies of disk block contents.  Caching disk blocks
// in memory reduces the number of disk reads and also provides
// a synchronization point for disk blocks used by multiple processes.
//
// Interface:
// * To get a buffer for a particular disk block, call bread.
// * After changing buffer data, call bwrite to write it to disk.
// * When done with the buffer, call brelse.
// * Do not use the buffer after calling brelse.
// * Only one process at a time can use a buffer,
//     so do not keep them longer than necessary.


#include "types.h"
#include "param.h"
#include "spinlock.h"
#include "sleeplock.h"
#include "riscv.h"
#include "defs.h"
#include "fs.h"
#include "buf.h"

#define NBUCKET 13

#define HASH(dev, blockno) ((((unsigned int)(dev)) << 16 | (blockno)) % NBUCKET)

struct bucket {
  struct spinlock lock;
  struct buf *head;
};

struct {
  struct spinlock lock;
  struct buf buf[NBUF];

  // Linked list of all buffers, through prev/next.
  // Sorted by how recently the buffer was used.
  // head.next is most recent, head.prev is least.
  struct bucket buckets[NBUCKET];
} bcache;

void
binit(void)
{
  for (int i = 0; i < NBUCKET; i++) {
    initlock(&bcache.buckets[i].lock, "bcache.bucket");  // 初始化每个桶的锁
    bcache.buckets[i].head = 0;  // 初始化每个桶的链表头为空
  }
  initlock(&bcache.lock, "bcache");  // 初始化全局锁
  for(int i = 0; i < NBUF; i++) {
    struct buf *b = &bcache.buf[i];
    int bucket = 0;
    b->next = bcache.buckets[bucket].head;
    b->prev = 0;
    if (bcache.buckets[bucket].head != 0) {
      bcache.buckets[bucket].head->prev = b;
    }
    bcache.buckets[bucket].head = b;
    initsleeplock(&b->lock, "buffer");
  }
}

// Look through buffer cache for block on device dev.
// If not found, allocate a buffer.
// In either case, return locked buffer.
static struct buf*
bget(uint dev, uint blockno)
{
  struct buf *b;

  int bucket = HASH(dev, blockno);  // 计算桶号
  acquire(&bcache.buckets[bucket].lock);  // 给目标桶加锁

  // 查看这个块是否被缓存到目标桶
  for(b = bcache.buckets[bucket].head; b; b = b->next){
    if(b->dev == dev && b->blockno == blockno){
      b->refcnt++;
      release(&bcache.buckets[bucket].lock);  // 解锁目标桶
      acquiresleep(&b->lock);  // 给目标缓存块加锁
      return b;
    }
  }

  // 缓存未命中，在目标桶中找一块干净的块
  for(b = bcache.buckets[bucket].head; b; b = b->next){
    if(b->refcnt == 0) {
      b->dev = dev;
      b->blockno = blockno;
      b->valid = 0;
      b->refcnt ++;
      release(&bcache.buckets[bucket].lock);  // 解锁目标桶
      acquiresleep(&b->lock);  // 给目标缓存块加锁
      return b;
    }
  }
  release(&bcache.buckets[bucket].lock);  // 解锁目标桶
  acquire(&bcache.lock);
  for (int i = 0; i < NBUCKET; i++) {
    if (i == bucket) continue;
    acquire(&bcache.buckets[i].lock); // 获取当前桶的锁
    for (b = bcache.buckets[i].head; b; b = b->next) {
      if (b->refcnt == 0) {
        // 从当前桶移除块
        if (b->prev) {
          b->prev->next = b->next;
        } else {
          bcache.buckets[i].head = b->next;
        }
        if (b->next) {
          b->next->prev = b->prev;
        }
        // 将块添加到目标桶头部
        b->prev = 0;
        b->next = bcache.buckets[bucket].head;
        if (bcache.buckets[bucket].head) {
          bcache.buckets[bucket].head->prev = b;
        }
        bcache.buckets[bucket].head = b;

        b->dev = dev;
        b->blockno = blockno;
        b->valid = 0;
        b->refcnt = 1;

        release(&bcache.buckets[i].lock);
        release(&bcache.lock);
        acquiresleep(&b->lock);
        return b;
      }
    }
    release(&bcache.buckets[i].lock); // 释放当前桶的锁
  }
  release(&bcache.lock);
  panic("bget: no buffers");
}

// Return a locked buf with the contents of the indicated block.
struct buf*
bread(uint dev, uint blockno)
{
  struct buf *b;

  b = bget(dev, blockno);
  if(!b->valid) {
    virtio_disk_rw(b, 0);
    b->valid = 1;
  }
  return b;
}

// Write b's contents to disk.  Must be locked.
void
bwrite(struct buf *b)
{
  if(!holdingsleep(&b->lock))
    panic("bwrite");
  virtio_disk_rw(b, 1);
}

// Release a locked buffer.
// Move to the head of the most-recently-used list.
void
brelse(struct buf *b)
{
  int bucket = HASH(b->dev, b->blockno);
  if(!holdingsleep(&b->lock))
    panic("brelse");

  releasesleep(&b->lock);
  acquire(&bcache.buckets[bucket].lock);
  b->refcnt--;
  release(&bcache.buckets[bucket].lock);
}

void
bpin(struct buf *b) {
  int bucket = HASH(b->dev, b->blockno);
  acquire(&bcache.buckets[bucket].lock);
  b->refcnt++;
  release(&bcache.buckets[bucket].lock);
}

void
bunpin(struct buf *b) {
  int bucket = HASH(b->dev, b->blockno);
  acquire(&bcache.buckets[bucket].lock);  
  if(b->refcnt > 0) b->refcnt--;
  release(&bcache.buckets[bucket].lock);
}