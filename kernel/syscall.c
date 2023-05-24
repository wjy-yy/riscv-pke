/*
 * contains the implementation of all syscalls.
 */

#include <stdint.h>
#include <errno.h>

#include "util/types.h"
#include "syscall.h"
#include "string.h"
#include "process.h"
#include "util/functions.h"
#include "pmm.h"
#include "vmm.h"
#include "spike_interface/spike_utils.h"

//
// implement the SYS_user_print syscall
//
ssize_t sys_user_print(const char* buf, size_t n) {
  // buf is now an address in user space of the given app's user stack,
  // so we have to transfer it into phisical address (kernel is running in direct mapping).
  assert( current );
  char* pa = (char*)user_va_to_pa((pagetable_t)(current->pagetable), (void*)buf);
  sprint(pa);
  return 0;
}

//
// implement the SYS_user_exit syscall
//
ssize_t sys_user_exit(uint64 code) {
  sprint("User exit with code:%d.\n", code);
  // in lab1, PKE considers only one app (one process). 
  // therefore, shutdown the system when the app calls exit()
  shutdown(code);
}

//
// maybe, the simplest implementation of malloc in the world ... added @lab2_2
//
typedef struct used_t {
  uint64 page_num, page_begin, page_end;
  //page_num is va's 12~38bits
}used;
used heap[512];
uint64 tot=0;
uint64 sys_user_allocate_page(uint64 width) {
  // heap[0].page_num=0;
  // heap[0].page_off=0; left_bound
  int i;
  for(i=0;i<tot;++i)
  {
    // sprint("allocate:%d\n",i);
    // sprint("va:%lx, begin:%lx, end:%lx\n",heap[i].page_num,heap[i].page_begin,heap[i].page_end);
    uint64 remain=0;
    // if(heap[i+1].page_num!=heap[i].page_num)
    //   remain=4096-heap[i].page_off;
    // else
    //   remain=heap[i+1].page_off-heap[i].page_off;
    // insert before current page
    if(i==0)
      remain=heap[0].page_begin;//special
    else if(heap[i-1].page_num!=heap[i].page_num)
      remain=4096-heap[i-1].page_end;
    else
      remain=heap[i].page_begin-heap[i-1].page_end;
    // sprint("remain:%ld\n",remain);
    if(remain>=width)
    {
      int j;
      for(j=tot+1;j>i;--j)
        heap[j]=heap[j-1];
      // if(heap[i-1].page_num!=heap[i].page_num)//must be [0,0]
      // {
        heap[i].page_num=heap[i-1].page_num;
        heap[i].page_begin=heap[i-1].page_end;
        heap[i].page_end=heap[i].page_begin+width;
      // }
      // else
      // {
      //   heap[i].page_num=heap[i-1].page_num;
      //   heap[i].page_begin=heap[i-1].page_end;
      //   heap[i].page_end=heap[i].page_begin+width;
      // }
      ++tot;
      return heap[i].page_num|heap[i].page_begin;
    }
  }
  //for the tail
  if(tot){
  uint64 remain=4096-heap[tot-1].page_end;
  if(remain>=width)
  {
    int j;
    for(j=tot+1;j>i;--j)
      heap[j]=heap[j-1];
    // if(heap[i-1].page_num!=heap[i].page_num)//must be [0,0]
    // {
      heap[i].page_num=heap[i-1].page_num;
      heap[i].page_begin=heap[i-1].page_end;
      heap[i].page_end=heap[i].page_begin+width;
    // }
    // else
    // {
    //   heap[i].page_num=heap[i-1].page_num;
    //   heap[i].page_begin=heap[i-1].page_end;
    //   heap[i].page_end=heap[i].page_begin+width;
    // }
    ++tot;
    return heap[i].page_num|heap[i].page_begin;
  }}
  //
  void* pa = alloc_page();
  uint64 va = g_ufree_page;
  g_ufree_page += PGSIZE;
  user_vm_map((pagetable_t)current->pagetable, va, PGSIZE, (uint64)pa,
         prot_to_type(PROT_WRITE | PROT_READ, 1));
  
  heap[tot].page_num=va;
  heap[tot].page_begin=0;
  heap[tot].page_end=0;
  ++tot;
  heap[tot].page_num=va;
  heap[tot].page_begin=0;
  heap[tot].page_end=width;
  ++tot;
  // sprint("va:%lx, off:%lx\n",va,width);
  // sprint("tot:%d\n",tot);
  return va;
}

//
// reclaim a page, indicated by "va". added @lab2_2
//
uint64 sys_user_free_page(uint64 va) {
  int i;
  for(i=0;i<tot;++i)
  {
    // sprint("free:%d\n",i);
    if((heap[i].page_num>>12)==(va>>12))
    {
      if(va>=heap[i].page_num+heap[i].page_begin&&va<heap[i].page_num+heap[i].page_end)
      {
        // if((i==0||heap[i].page_num!=heap[i-1].page_num)&&heap[i].page_num!=heap[i+1].page_num)
        if(heap[i].page_end==0&&heap[i].page_num!=heap[i+1].page_num)
        {
          user_vm_unmap((pagetable_t)current->pagetable, heap[i].page_num, PGSIZE, 1);
          int j;
          for(j=i;j<tot;++j)
            heap[j]=heap[j+1];
          --tot;
          --i;
        }
        int j;
        for(j=i;j<tot;++j)
          heap[j]=heap[j+1];
        --tot;
        return 0;
      }
    }
  }
  // user_vm_unmap((pagetable_t)current->pagetable, va, PGSIZE, 1);
  return 0;
}

//
// [a0]: the syscall number; [a1] ... [a7]: arguments to the syscalls.
// returns the code of success, (e.g., 0 means success, fail for otherwise)
//
long do_syscall(long a0, long a1, long a2, long a3, long a4, long a5, long a6, long a7) {
  switch (a0) {
    case SYS_user_print:
      return sys_user_print((const char*)a1, a2);
    case SYS_user_exit:
      return sys_user_exit(a1);
    // added @lab2_2
    case SYS_user_allocate_page:
      return sys_user_allocate_page(a1);
    case SYS_user_free_page:
      return sys_user_free_page(a1);
    default:
      panic("Unknown syscall %ld \n", a0);
  }
}
