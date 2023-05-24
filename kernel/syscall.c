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

#include "spike_interface/spike_utils.h"

#include "elf.h"

//
// implement the SYS_user_print syscall
//
ssize_t sys_user_print(const char* buf, size_t n) {
  sprint(buf);
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

ssize_t sys_user_backtrace(long a1) {
  uint64 fp=current->trapframe->regs.s0;
  uint64 sp=current->trapframe->kernel_sp;
  // sprint("fp=%lx, sp=%lx\n",fp,sp);
  // load the elf header
  // elf_ctx *ctx;
  // if (elf_fpread(ctx, &ctx->ehdr, sizeof(ctx->ehdr), 0) != sizeof(ctx->ehdr)) return EL_EIO;
  // return print_tab();
  //use the "user kernel" stack (whose pointer stored in p->trapframe->kernel_sp)
  // char namelist[1024];
  // elf_sym sym[1024];
  // elf_load_symbol(current,namelist,sym);
  char namelist[1024];
  elf_sym sym[1024];
  int i, symnum;
  get_function_name(current, namelist, sym, &symnum);
  // sprint("main_symnum=%ld\n",symnum);
  fp=*((uint64*)fp-1);
  while(a1--)
  {
    // sprint("T:%ld\n",a1);
    uint64 ra=*((uint64*)fp-1);
    // sprint("ra=%lx\n",ra);
    //return address
    if(!ra) break;
    for(i=0;i<symnum;++i)
    {
      // sprint("operating on %d\n",i);
      if(sym[i].st_value<ra&&sym[i].st_value>ra-sym[i].st_size)
        sprint("%s\n",&namelist[sym[i].st_name]);
        // sprint("%s\n",namelist[sym[i].st_name]);
      // sprint("%lx\n",sym[i].st_name);
      // sprint("operating on %d\n",i);
      // // sprint("%s\n",namelist[sym[i].st_name]);
      // sprint("%s\n",sym[i].st_name);
      // sprint("operating on %d\n",i);
    }
      
    fp=*((uint64*)fp-2);
    // uint64 ra=newfp-8;
  }
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
    case SYS_user_backtrace:
      return sys_user_backtrace(a1);
    default:
      panic("Unknown syscall %ld \n", a0);
  }
}
