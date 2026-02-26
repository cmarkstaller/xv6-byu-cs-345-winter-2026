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

#ifdef LAB_PGTBL
#define NSUPERPAGES 2

struct {
  struct spinlock lock;
  void *base[NSUPERPAGES];
  int used[NSUPERPAGES];
} supermem;
#endif

void
kinit()
{
  initlock(&kmem.lock, "kmem");

  #ifdef LAB_PGTBL
  initlock(&supermem.lock, "supermem");

  char *p = (char*)PGROUNDUP((uint64)end);
  int nsuper = 0;

  for(; p + PGSIZE <= (char*)PHYSTOP; p += PGSIZE){
    // Is there still room to reserve more superpages?
    if(nsuper < NSUPERPAGES &&
       ((uint64)p % SUPERPGSIZE) == 0 &&
       p + SUPERPGSIZE <= (char*)PHYSTOP){
      supermem.base[nsuper] = p;
      supermem.used[nsuper] = 0;
      nsuper++;

      // Skip over the rest of this 2MB superpage region.
      p += SUPERPGSIZE - PGSIZE;
      continue;
    }

    kfree(p);
  }
#else
  freerange(end, (void*)PHYSTOP);
#endif
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

  acquire(&kmem.lock);
  r->next = kmem.freelist;
  kmem.freelist = r;
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
    kmem.freelist = r->next;
  release(&kmem.lock);

  if(r)
    memset((char*)r, 5, PGSIZE); // fill with junk
  return (void*)r;
}

#ifdef LAB_PGTBL
void *
superalloc(void)
{
  acquire(&supermem.lock);
  for(int i = 0; i < NSUPERPAGES; i++){
    if(supermem.base[i] && supermem.used[i] == 0){
      supermem.used[i] = 1;
      void *p = supermem.base[i];
      release(&supermem.lock);
      memset(p, 5, SUPERPGSIZE);
      return p;
    }
  }
  release(&supermem.lock);
  return 0;
}

void
superfree(void *pa)
{
  acquire(&supermem.lock);
  for(int i = 0; i < NSUPERPAGES; i++){
    if(supermem.base[i] == pa){
      supermem.used[i] = 0;
      break;
    }
  }
  release(&supermem.lock);
}
#endif
