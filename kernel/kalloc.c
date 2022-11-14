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
  char ref_cnt[PANUM];
} kmem;


uint64
pageindex(void* pa)
{
    return ((uint64)pa - (uint64)KERNBASE) / PGSIZE;
}

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
  for (int i = 0 ; i < PANUM; i++)
    kmem.ref_cnt[i] = 1;

  for(; p + PGSIZE <= (char*)pa_end; p += PGSIZE)
    kfree(p);
}

// Free the page of physical memory pointed at by v,
// which normally should have been returned by a
// call to kalloc().  (The exception is when
// initializing the allocator; see kinit above.)
void
kfree(void *pa)
{
  struct run *r;

  if(((uint64)pa % PGSIZE) != 0 || (char*)pa < end || (uint64)pa >= PHYSTOP)
    panic("kfree");

  acquire(&kmem.lock);
  uint index = pageindex(pa); 
  char ref_count = kmem.ref_cnt[index];

  if (ref_count == 0) {
        panic("re_free: ref_cnt is 0");
  }
  ref_count = ref_count -1;

  if (ref_count == 0) { // 0 for initstatus, 1 for only one user

      memset(pa, 1, PGSIZE);

      r = (struct run*)pa;

      r->next = kmem.freelist;
      kmem.freelist = r;
  }

  kmem.ref_cnt[index] = ref_count;

  // Fill with junk to catch dangling refs.
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
  if(r)
  {
    uint index = pageindex(r);
    if (kmem.ref_cnt[index] != 0) {
        panic("realloc, ref > 1");
    }
    kmem.ref_cnt[index] = 1;

    kmem.freelist = r->next;
  }
  release(&kmem.lock);

  if(r)
    memset((char*)r, 5, PGSIZE); // fill with junk
  return (void*)r;
}

void
krefcalloc(void * pa)
{
    acquire(&kmem.lock);
    uint index = pageindex(pa);
    char ref_count = kmem.ref_cnt[index];
    kmem.ref_cnt[index] = ref_count + 1;
    release(&kmem.lock);
}

void
krefcfree(void *pa)
{
    acquire(&kmem.lock);
    char account = kmem.ref_cnt[pageindex(pa)];
    if (account >=2 )
        kmem.ref_cnt[pageindex(pa)] --;
    release(&kmem.lock);
}
