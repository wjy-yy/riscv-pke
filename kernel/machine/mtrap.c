#include "kernel/riscv.h"
#include "kernel/process.h"
#include "spike_interface/spike_utils.h"
#include <string.h>

static void error_print()
{
  // sprint("dir:%s\n", current->dir[0]); 
  // sprint("file:%s\n", current->file[0].file); 
  // sprint("filedir:%ld\n", current->file[0].dir); 
  // sprint("lineaddr:%ld\n", current->line[0].addr); 
  // sprint("lineline:%ld\n", current->line[0].line); 
  // sprint("linefile:%ld\n", current->line[0].file); 
  uint64 mepc = read_csr(mepc);
  // sprint("mepc:%lx\n",mepc);
  int i;
  for(i=0;;++i)
  {
    // sprint("line number:%ld, addr number:%ld\n",current->line[i].line, current->line[i].addr);
    if(current->line[i].addr==mepc)
    {
      uint64 file_index=current->line[i].file;
      uint64 dir=current->file[file_index].dir;
      sprint("Runtime error at %s/%s:%ld\n",current->dir[dir],current->file[file_index].file,current->line[i].line);

      struct stat mystat;
      char path[200], code;
      int len=strlen(current->dir[dir]);
      strcpy(path, current->dir[dir]);
      strcpy(path+len+1, current->file[file_index].file);
      path[len]='/';
      path[len+1+strlen(current->file[file_index].file)]='\0';
      sprint("path:%s\n",path);
      spike_file_t *f = spike_file_open(path, O_RDONLY, 0);
      int cur,line=1;
      for(cur=0;;++cur)
      {
        spike_file_pread(f, &code, 1, cur);
        if(line==current->line[i].line)
        {
          sprint("%c",code);
          if(code=='\n')
            break;
        }
        if(code=='\n')
            ++line;
      }
      spike_file_close(f);
      break;
    }
  }
}

static void handle_instruction_access_fault() { 
  error_print(); panic("Instruction access fault!"); }

static void handle_load_access_fault() { 
  error_print(); panic("Load access fault!"); }

static void handle_store_access_fault() { error_print(); panic("Store/AMO access fault!"); }

static void handle_illegal_instruction() {
  error_print();
  panic("Illegal instruction!"); }

static void handle_misaligned_load() { error_print(); panic("Misaligned Load!"); }

static void handle_misaligned_store() { error_print(); panic("Misaligned AMO!"); }

// added @lab1_3
static void handle_timer() {
  int cpuid = 0;
  // setup the timer fired at next time (TIMER_INTERVAL from now)
  *(uint64*)CLINT_MTIMECMP(cpuid) = *(uint64*)CLINT_MTIMECMP(cpuid) + TIMER_INTERVAL;

  // setup a soft interrupt in sip (S-mode Interrupt Pending) to be handled in S-mode
  write_csr(sip, SIP_SSIP);
}

//
// handle_mtrap calls a handling function according to the type of a machine mode interrupt (trap).
//
void handle_mtrap() {
  uint64 mcause = read_csr(mcause);
  switch (mcause) {
    case CAUSE_MTIMER:
      handle_timer();
      break;
    case CAUSE_FETCH_ACCESS:
      handle_instruction_access_fault();
      break;
    case CAUSE_LOAD_ACCESS:
      handle_load_access_fault();
    case CAUSE_STORE_ACCESS:
      handle_store_access_fault();
      break;
    case CAUSE_ILLEGAL_INSTRUCTION:
      // TODO (lab1_2): call handle_illegal_instruction to implement illegal instruction
      // interception, and finish lab1_2.
      // panic( "call handle_illegal_instruction to accomplish illegal instruction interception for lab1_2.\n" );
      handle_illegal_instruction();

      break;
    case CAUSE_MISALIGNED_LOAD:
      handle_misaligned_load();
      break;
    case CAUSE_MISALIGNED_STORE:
      handle_misaligned_store();
      break;

    default:
      sprint("machine trap(): unexpected mscause %p\n", mcause);
      sprint("            mepc=%p mtval=%p\n", read_csr(mepc), read_csr(mtval));
      error_print();
      panic( "unexpected exception happened in M-mode.\n" );
      break;
  }
}
