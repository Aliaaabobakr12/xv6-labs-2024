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

struct spinlock cnt_lock;
int refcnt[PHYSTOP >> 12];

void
kinit()
{
  initlock(&cnt_lock, "cow");
  initlock(&kmem.lock, "kmem");
  freerange(end, (void*)PHYSTOP);
}

void freerange(void *pa_start, void *pa_end)
{
  char *p;
  p = (char*)PGROUNDUP((uint64)pa_start);
  for (; p + PGSIZE <= (char*)pa_end; p += PGSIZE) {
    refcnt[(uint64)p >> 12] = 1;
    kfree(p);
  }
}

// Free the page of physical memory pointed at by pa,
// which normally should have been returned by a
// call to kalloc().  (The exception is when
// initializing the allocator; see kinit above.)
#include <stdint.h> // for uintptr_t

void kfree(void *pa)
{
  struct run *r;

  // pa validation checks
  if ((uintptr_t)pa % PGSIZE != 0 || (uintptr_t)pa < (uintptr_t)end || (uintptr_t)pa > (uintptr_t)PHYSTOP)
    panic("kfree: invalid pa");

  int pgidx = (uintptr_t)pa >> 12; // pa divide by PGSIZE (4k)

  // kfree will release the page if ref_cnt is 1, otherwise decrement ref_cnt
  acquire(&cnt_lock);

  // case 1: last reference, release the page
  if (--refcnt[pgidx] == 0) {
    release(&cnt_lock);

    memset(pa, 1 , PGSIZE);
    r = (struct run *) pa;

    acquire(&kmem.lock);
    r->next = kmem.freelist;
    kmem.freelist = r;
    release(&kmem.lock);

  // case 2: decrease the reference count only
  } else {
    release(&cnt_lock);
  }
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
   
  /* first make sure the page is available */
  if(r) {
    kmem.freelist = r->next;
   
    /* second, increase the ref_cnt of new page to 1 */
    acquire(&cnt_lock);
    refcnt[(uint64)r >> 12] = 1;
    release(&cnt_lock);
  }
   
  release(&kmem.lock);
   
  if(r)
    memset((char*)r, 5, PGSIZE); // fill with junk
   
  return (void*)r;
}
