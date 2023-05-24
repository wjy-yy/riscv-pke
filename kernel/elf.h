#ifndef _ELF_H_
#define _ELF_H_

#include "util/types.h"
#include "process.h"

#define MAX_CMDLINE_ARGS 64

// elf header structure
typedef struct elf_header_t {
  uint32 magic;
  uint8 elf[12];
  uint16 type;      /* Object file type */
  uint16 machine;   /* Architecture */
  uint32 version;   /* Object file version */
  uint64 entry;     /* Entry point virtual address */
  uint64 phoff;     /* Program header table file offset */
  uint64 shoff;     /* Section header table file offset */
  uint32 flags;     /* Processor-specific flags */
  uint16 ehsize;    /* ELF header size in bytes */
  uint16 phentsize; /* Program header table entry size */
  uint16 phnum;     /* Program header table entry count */
  uint16 shentsize; /* Section header table entry size */
  uint16 shnum;     /* Section header table entry count */
  uint16 shstrndx;  /* Section header string table index */
} elf_header;

// Program segment header.
typedef struct elf_prog_header_t {
  uint32 type;   /* Segment type */
  uint32 flags;  /* Segment flags */
  uint64 off;    /* Segment file offset */
  uint64 vaddr;  /* Segment virtual address */
  uint64 paddr;  /* Segment physical address */
  uint64 filesz; /* Segment size in file */
  uint64 memsz;  /* Segment size in memory */
  uint64 align;  /* Segment alignment */
} elf_prog_header;

#define ELF_MAGIC 0x464C457FU  // "\x7FELF" in little endian
#define ELF_PROG_LOAD 1

typedef enum elf_status_t {
  EL_OK = 0,

  EL_EIO,
  EL_ENOMEM,
  EL_NOTELF,
  EL_ERR,

} elf_status;

typedef struct elf_ctx_t {
  void *info;
  elf_header ehdr;
} elf_ctx;

typedef struct elf_shdr_t {
  uint32 name; //存储关于sh str ndx值所在节的地址的偏移量
  uint32 type;  
  uint64 flags;  
  uint64 addr; /*the first byte of the section.*/  
  uint64 offset;/*此成员的取值给出节区的第一个字节与文件头之间的偏移*/  
  uint64 size;  
  uint32 link;  
  uint32 info;  
  uint64 addralign;  
  uint64 entsize;
}elf_shdr;

typedef struct elf_sym_t {  
  uint32 st_name; // symbol name, the index of string table  
  uint64 st_value; // symbol value, the virtual address  
  uint32 st_size;  
  uint8 st_info;  
  uint8 st_other;  
  uint16 st_shndx; // 符号相关节的节索引  
} elf_sym;

elf_status elf_init(elf_ctx *ctx, void *info);
elf_status elf_load(elf_ctx *ctx);

void load_bincode_from_host_elf(process *p);

elf_status elf_load_symbol(elf_ctx *ctx, char namelist[], elf_sym sym[], int* symnum);

void get_function_name(process *p, char namelist[], elf_sym sym[], int* symnum);



// char name_list[1024];
// elf_sym sym_[1024];

#endif
