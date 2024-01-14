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
  int cpuid;
  struct run *next;
};

struct {
  struct spinlock lock;
  struct run *freelist;
} kmem[NCPU];

void
kinit()
{
  int i;
  for(i = 0; i < NCPU; i++){
    initlock(&kmem[i].lock, "kmem");
    kmem[i].freelist = 0;
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
  int cur_cpu_id;
  push_off();
  cur_cpu_id = cpuid();

  if(((uint64)pa % PGSIZE) != 0 || (char*)pa < end || (uint64)pa >= PHYSTOP)
    panic("kfree");

  // Fill with junk to catch dangling refs.
  memset(pa, 1, PGSIZE);

  r = (struct run*)pa;
  r->cpuid = cur_cpu_id;
  acquire(&kmem[cur_cpu_id].lock);
  r->next = kmem[cur_cpu_id].freelist;
  kmem[cur_cpu_id].freelist = r;
  release(&kmem[cur_cpu_id].lock);
  pop_off();
}

// Allocate one 4096-byte page of physical memory.
// Returns a pointer that the kernel can use.
// Returns 0 if the memory cannot be allocated.
void *
kalloc(void)
{
  struct run *r;
  int i;
  int cur_cpu_id;
  push_off();
  cur_cpu_id = cpuid();

  acquire(&kmem[cur_cpu_id].lock);
  r = kmem[cur_cpu_id].freelist;
  if(r)
    kmem[cur_cpu_id].freelist = r->next;
  release(&kmem[cur_cpu_id].lock);

  if(!r){
    for (i = 0; i < NCPU && !r; i++){
      if(i == cur_cpu_id) continue;
      acquire(&kmem[i].lock);
      if(kmem[i].freelist){
        r = kmem[i].freelist;
        kmem[i].freelist = r->next;
      }
      release(&kmem[i].lock);
    }
  }


  if(r)
    memset((char*)r, 5, PGSIZE); // fill with junk
  pop_off();
  return (void*)r;
}
