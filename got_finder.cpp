#include "ptracer.h"
#include "got_finder.h"
#include "log.h"
#include "round.h"
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <string>
#include <set>

got_finder_client::~got_finder_client () {}
typedef uint32_t mask_type;

struct got_finder::got_finder_impl
{
  intptr_t symtab_start_;
  intptr_t strtab_start_;
  intptr_t strtab_size_;
  intptr_t jmprel_start_;
  size_t plt_got_size_;
  int plt_got_rel_type_;
  mask_type filled_mask_;
  bool fatal_occured_;

  got_finder_impl ();

  template <class elf_relocation_type, class elf_header_type>
  bool deal_with_elf_relocation (unsigned long start, unsigned long end,
                                 ptracer *ptracer, const char *name,
                                 got_finder_client *client,
                                 std::string &strtab);
};

static mask_type symtab_start_bit = (1 << 0);
static mask_type strtab_start_bit = (1 << 1);
static mask_type strtab_size_bit = (1 << 2);
static mask_type jmprel_start_bit = (1 << 3);
static mask_type plt_got_size_bit = (1 << 4);
static mask_type plt_got_rel_type_bit = (1 << 5);
static mask_type mandatory_mask = (1 << 6) - 1;
static mask_type mandatory_value = (1 << 6) - 1;

#if defined(__x86_64__) && (__x86_64__ == 1)
#define R_X86_64_JUMP_SLOT 7
#define DESIGNATIVE_R_TYPE R_X86_64_JUMP_SLOT
#elif defined(__arm__) && (__arm__ == 1)
#define R_ARM_JUMP_SLOT 22
#define DESIGNATIVE_R_TYPE R_ARM_JUMP_SLOT
#else
#error unsupported arch
#endif

inline static bool
is_mandatory_set (mask_type filled)
{
  return (filled & mandatory_mask) == mandatory_value;
}

got_finder::got_finder () : impl_ (new got_finder_impl) {}
bool
got_finder::find (ptracer *ptracer, const char *name, pid_t tid,
                  got_finder_client *client)
{
  // first lookup from /proc/tid/maps
  char buf[1024];
  snprintf (buf, 1024, "/proc/%d/maps", tid);
  FILE *filep;
  typedef std::set<std::string> str_set;
  str_set _str_set;

  filep = fopen (buf, "r");
  if (!filep)
    return false;
  char *str;
  bool _deal_with = false;

  while ((str = fgets (buf, 1024, filep)))
    {
      int len = strlen (str);
      if (len == 0)
        continue;
      // strip the left.
      while (true)
        {
          if (str[len - 1] == '\n')
            {
              str[--len] = '\0';
            }
          else
            {
              break;
            }
        }
      char *slash = strrchr (str, '/');
      if (slash == NULL)
        continue;
      std::pair<str_set::iterator, bool> ret = _str_set.insert (slash);
      if (!ret.second)
        continue;
      unsigned long start, end;
      errno = 0;
      char *end_of_first_match;
      start = strtoul (str, &end_of_first_match, 16);
      if (errno)
        {
          LOGE ("got_finder::find: strtoul fails %d:%s\n", __LINE__,
                strerror (errno));
          break;
        }
      end_of_first_match++; // skip -
      end = strtoul (end_of_first_match, NULL, 16);
      if (errno)
        {
          LOGE ("got_finder::find: strtoul fails %d:%s\n", __LINE__,
                strerror (errno));
          break;
        }
      if (deal_with (start, end, ptracer, name, client))
        {
          _deal_with = true;
          break;
        }
      // Will not continue if fatal occured.
      if (impl_->fatal_occured_)
        {
          break;
        }
    }
  fclose (filep);
  return _deal_with;
}

typedef uint64_t __u64;
typedef uint32_t __u32;
typedef uint16_t __u16;
typedef int64_t __s64;
typedef int32_t __s32;
typedef int16_t __s16;
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
typedef __u32 Elf32_Addr;
typedef __u16 Elf32_Half;
typedef __u32 Elf32_Off;
typedef __s32 Elf32_Sword;
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
typedef __u32 Elf32_Word;
typedef __u64 Elf64_Addr;
typedef __u16 Elf64_Half;
typedef __s16 Elf64_SHalf;
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
typedef __u64 Elf64_Off;
typedef __s32 Elf64_Sword;
typedef __u32 Elf64_Word;
typedef __u64 Elf64_Xword;
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
typedef __s64 Elf64_Sxword;
#define PT_NULL 0
#define PT_LOAD 1
#define PT_DYNAMIC 2
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define PT_INTERP 3
#define PT_NOTE 4
#define PT_SHLIB 5
#define PT_PHDR 6
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define PT_TLS 7
#define PT_LOOS 0x60000000
#define PT_HIOS 0x6fffffff
#define PT_LOPROC 0x70000000
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define PT_HIPROC 0x7fffffff
#define PT_GNU_EH_FRAME 0x6474e550
#define PT_GNU_STACK (PT_LOOS + 0x474e551)
#define PN_XNUM 0xffff
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define ET_NONE 0
#define ET_REL 1
#define ET_EXEC 2
#define ET_DYN 3
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define ET_CORE 4
#define ET_LOPROC 0xff00
#define ET_HIPROC 0xffff
#define DT_NULL 0
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define DT_NEEDED 1
#define DT_PLTRELSZ 2
#define DT_PLTGOT 3
#define DT_HASH 4
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define DT_STRTAB 5
#define DT_SYMTAB 6
#define DT_RELA 7
#define DT_RELASZ 8
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define DT_RELAENT 9
#define DT_STRSZ 10
#define DT_SYMENT 11
#define DT_INIT 12
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define DT_FINI 13
#define DT_SONAME 14
#define DT_RPATH 15
#define DT_SYMBOLIC 16
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define DT_REL 17
#define DT_RELSZ 18
#define DT_RELENT 19
#define DT_PLTREL 20
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define DT_DEBUG 21
#define DT_TEXTREL 22
#define DT_JMPREL 23
#define DT_ENCODING 32
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define OLD_DT_LOOS 0x60000000
#define DT_LOOS 0x6000000d
#define DT_HIOS 0x6ffff000
#define DT_VALRNGLO 0x6ffffd00
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define DT_VALRNGHI 0x6ffffdff
#define DT_ADDRRNGLO 0x6ffffe00
#define DT_ADDRRNGHI 0x6ffffeff
#define DT_VERSYM 0x6ffffff0
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define DT_RELACOUNT 0x6ffffff9
#define DT_RELCOUNT 0x6ffffffa
#define DT_FLAGS_1 0x6ffffffb
#define DT_VERDEF 0x6ffffffc
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define DT_VERDEFNUM 0x6ffffffd
#define DT_VERNEED 0x6ffffffe
#define DT_VERNEEDNUM 0x6fffffff
#define OLD_DT_HIOS 0x6fffffff
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define DT_LOPROC 0x70000000
#define DT_HIPROC 0x7fffffff
#define STB_LOCAL 0
#define STB_GLOBAL 1
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define STB_WEAK 2
#define STT_NOTYPE 0
#define STT_OBJECT 1
#define STT_FUNC 2
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define STT_SECTION 3
#define STT_FILE 4
#define STT_COMMON 5
#define STT_TLS 6
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define ELF_ST_BIND(x) ((x) >> 4)
#define ELF_ST_TYPE(x) (((unsigned int)x) & 0xf)
#define ELF32_ST_BIND(x) ELF_ST_BIND (x)
#define ELF32_ST_TYPE(x) ELF_ST_TYPE (x)
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define ELF64_ST_BIND(x) ELF_ST_BIND (x)
#define ELF64_ST_TYPE(x) ELF_ST_TYPE (x)
typedef struct dynamic
{
  Elf32_Sword d_tag;
  /* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
  union
  {
    Elf32_Sword d_val;
    Elf32_Addr d_ptr;
  } d_un;
  /* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
} Elf32_Dyn;
typedef struct
{
  Elf64_Sxword d_tag;
  union
  {
    /* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
    Elf64_Xword d_val;
    Elf64_Addr d_ptr;
  } d_un;
} Elf64_Dyn;
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define ELF32_R_SYM(x) ((x) >> 8)
#define ELF32_R_TYPE(x) ((x)&0xff)
#define ELF64_R_SYM(i) ((i) >> 32)
#define ELF64_R_TYPE(i) ((i)&0xffffffff)
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
typedef struct elf32_rel
{
  Elf32_Addr r_offset;
  Elf32_Word r_info;
} Elf32_Rel;
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
typedef struct elf64_rel
{
  Elf64_Addr r_offset;
  Elf64_Xword r_info;
} Elf64_Rel;
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
typedef struct elf32_rela
{
  Elf32_Addr r_offset;
  Elf32_Word r_info;
  Elf32_Sword r_addend;
  /* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
} Elf32_Rela;
typedef struct elf64_rela
{
  Elf64_Addr r_offset;
  Elf64_Xword r_info;
  /* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
  Elf64_Sxword r_addend;
} Elf64_Rela;
typedef struct elf32_sym
{
  Elf32_Word st_name;
  /* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
  Elf32_Addr st_value;
  Elf32_Word st_size;
  unsigned char st_info;
  unsigned char st_other;
  /* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
  Elf32_Half st_shndx;
} Elf32_Sym;
typedef struct elf64_sym
{
  Elf64_Word st_name;
  /* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
  unsigned char st_info;
  unsigned char st_other;
  Elf64_Half st_shndx;
  Elf64_Addr st_value;
  /* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
  Elf64_Xword st_size;
} Elf64_Sym;
#define EI_NIDENT 16
typedef struct elf32_hdr
{
  /* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
  unsigned char e_ident[EI_NIDENT];
  Elf32_Half e_type;
  Elf32_Half e_machine;
  Elf32_Word e_version;
  /* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
  Elf32_Addr e_entry;
  Elf32_Off e_phoff;
  Elf32_Off e_shoff;
  Elf32_Word e_flags;
  /* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
  Elf32_Half e_ehsize;
  Elf32_Half e_phentsize;
  Elf32_Half e_phnum;
  Elf32_Half e_shentsize;
  /* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
  Elf32_Half e_shnum;
  Elf32_Half e_shstrndx;
} Elf32_Ehdr;
typedef struct elf64_hdr
{
  /* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
  unsigned char e_ident[EI_NIDENT];
  Elf64_Half e_type;
  Elf64_Half e_machine;
  Elf64_Word e_version;
  /* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
  Elf64_Addr e_entry;
  Elf64_Off e_phoff;
  Elf64_Off e_shoff;
  Elf64_Word e_flags;
  /* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
  Elf64_Half e_ehsize;
  Elf64_Half e_phentsize;
  Elf64_Half e_phnum;
  Elf64_Half e_shentsize;
  /* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
  Elf64_Half e_shnum;
  Elf64_Half e_shstrndx;
} Elf64_Ehdr;
#define PF_R 0x4
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define PF_W 0x2
#define PF_X 0x1
typedef struct elf32_phdr
{
  Elf32_Word p_type;
  /* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
  Elf32_Off p_offset;
  Elf32_Addr p_vaddr;
  Elf32_Addr p_paddr;
  Elf32_Word p_filesz;
  /* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
  Elf32_Word p_memsz;
  Elf32_Word p_flags;
  Elf32_Word p_align;
} Elf32_Phdr;
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
typedef struct elf64_phdr
{
  Elf64_Word p_type;
  Elf64_Word p_flags;
  Elf64_Off p_offset;
  /* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
  Elf64_Addr p_vaddr;
  Elf64_Addr p_paddr;
  Elf64_Xword p_filesz;
  Elf64_Xword p_memsz;
  /* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
  Elf64_Xword p_align;
} Elf64_Phdr;
#define SHT_NULL 0
#define SHT_PROGBITS 1
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define SHT_SYMTAB 2
#define SHT_STRTAB 3
#define SHT_RELA 4
#define SHT_HASH 5
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define SHT_DYNAMIC 6
#define SHT_NOTE 7
#define SHT_NOBITS 8
#define SHT_REL 9
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define SHT_SHLIB 10
#define SHT_DYNSYM 11
#define SHT_NUM 12
#define SHT_LOPROC 0x70000000
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define SHT_HIPROC 0x7fffffff
#define SHT_LOUSER 0x80000000
#define SHT_HIUSER 0xffffffff
#define SHF_WRITE 0x1
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define SHF_ALLOC 0x2
#define SHF_EXECINSTR 0x4
#define SHF_MASKPROC 0xf0000000
#define SHN_UNDEF 0
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define SHN_LORESERVE 0xff00
#define SHN_LOPROC 0xff00
#define SHN_HIPROC 0xff1f
#define SHN_ABS 0xfff1
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define SHN_COMMON 0xfff2
#define SHN_HIRESERVE 0xffff
typedef struct elf32_shdr
{
  Elf32_Word sh_name;
  /* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
  Elf32_Word sh_type;
  Elf32_Word sh_flags;
  Elf32_Addr sh_addr;
  Elf32_Off sh_offset;
  /* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
  Elf32_Word sh_size;
  Elf32_Word sh_link;
  Elf32_Word sh_info;
  Elf32_Word sh_addralign;
  /* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
  Elf32_Word sh_entsize;
} Elf32_Shdr;
typedef struct elf64_shdr
{
  Elf64_Word sh_name;
  /* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
  Elf64_Word sh_type;
  Elf64_Xword sh_flags;
  Elf64_Addr sh_addr;
  Elf64_Off sh_offset;
  /* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
  Elf64_Xword sh_size;
  Elf64_Word sh_link;
  Elf64_Word sh_info;
  Elf64_Xword sh_addralign;
  /* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
  Elf64_Xword sh_entsize;
} Elf64_Shdr;
#define EI_MAG0 0
#define EI_MAG1 1
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define EI_MAG2 2
#define EI_MAG3 3
#define EI_CLASS 4
#define EI_DATA 5
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define EI_VERSION 6
#define EI_OSABI 7
#define EI_PAD 8
#define ELFMAG0 0x7f
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define ELFMAG1 'E'
#define ELFMAG2 'L'
#define ELFMAG3 'F'
#define ELFMAG "\177ELF"
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define SELFMAG 4
#define ELFCLASSNONE 0
#define ELFCLASS32 1
#define ELFCLASS64 2
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define ELFCLASSNUM 3
#define ELFDATANONE 0
#define ELFDATA2LSB 1
#define ELFDATA2MSB 2
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define EV_NONE 0
#define EV_CURRENT 1
#define EV_NUM 2
#define ELFOSABI_NONE 0
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define ELFOSABI_LINUX 3
#ifndef ELF_OSABI
#define ELF_OSABI ELFOSABI_NONE
#endif
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define NT_PRSTATUS 1
#define NT_PRFPREG 2
#define NT_PRPSINFO 3
#define NT_TASKSTRUCT 4
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define NT_AUXV 6
#define NT_SIGINFO 0x53494749
#define NT_FILE 0x46494c45
#define NT_PRXFPREG 0x46e62b7f
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define NT_PPC_VMX 0x100
#define NT_PPC_SPE 0x101
#define NT_PPC_VSX 0x102
#define NT_386_TLS 0x200
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define NT_386_IOPERM 0x201
#define NT_X86_XSTATE 0x202
#define NT_S390_HIGH_GPRS 0x300
#define NT_S390_TIMER 0x301
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define NT_S390_TODCMP 0x302
#define NT_S390_TODPREG 0x303
#define NT_S390_CTRS 0x304
#define NT_S390_PREFIX 0x305
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define NT_S390_LAST_BREAK 0x306
#define NT_S390_SYSTEM_CALL 0x307
#define NT_S390_TDB 0x308
#define NT_ARM_VFP 0x400
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define NT_ARM_TLS 0x401
#define NT_ARM_HW_BREAK 0x402
#define NT_ARM_HW_WATCH 0x403
#define NT_METAG_CBUF 0x500
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define NT_METAG_RPIPE 0x501
#define NT_METAG_TLS 0x502
typedef struct elf32_note
{
  Elf32_Word n_namesz;
  /* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
  Elf32_Word n_descsz;
  Elf32_Word n_type;
} Elf32_Nhdr;
typedef struct elf64_note
{
  /* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
  Elf64_Word n_namesz;
  Elf64_Word n_descsz;
  Elf64_Word n_type;
} Elf64_Nhdr;
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */

#if __LP64__
#define ElfW(type) Elf64_##type
#else
#define ElfW(type) Elf32_##type
#endif

#ifndef offsetof
#define offsetof(TYPE, MEMBER) ((size_t) & ((TYPE *)0)->MEMBER)
#endif

bool
got_finder::check_elf (unsigned long start, ptracer *ptracer, bool *is_elf32)
{
  unsigned char e_ident[EI_NIDENT];
  if (!ptracer->read_memory (e_ident, sizeof (e_ident), start))
    {
      impl_->fatal_occured_ = true;
      LOGE ("got_finder::check_elf: fails to read memory.\n");
      return false;
    }
  if (memcmp (e_ident, ELFMAG, SELFMAG))
    {
      return false;
    }
  *is_elf32 = (e_ident[EI_CLASS] != ELFCLASS64);
  return true;
}

bool
got_finder::deal_with (unsigned long start, unsigned long end,
                       ptracer *ptracer, const char *name,
                       got_finder_client *client)
{
  assert (end - start >= PAGE_SIZE);
  bool is_elf32;
  if (!check_elf (start, ptracer, &is_elf32))
    {
      return false;
    }
  if (is_elf32)
    return deal_with_elf<Elf32_Ehdr> (start, end, ptracer, name, client);
  else
    return deal_with_elf<Elf64_Ehdr> (start, end, ptracer, name, client);
}

template <class _elf_header_type> struct elf_deducer
{
  typedef _elf_header_type elf_header_type;
};

template <> struct elf_deducer<Elf32_Ehdr>
{
  typedef Elf32_Ehdr elf_header_type;
  typedef Elf32_Phdr elf_phdr;
  typedef Elf32_Rela elf_rela;
  typedef Elf32_Rel elf_rel;
  typedef Elf32_Dyn elf_dynamic;
  typedef Elf32_Sym elf_sym;
};

template <> struct elf_deducer<Elf64_Ehdr>
{
  typedef Elf64_Ehdr elf_header_type;
  typedef Elf64_Phdr elf_phdr;
  typedef Elf64_Rela elf_rela;
  typedef Elf64_Rel elf_rel;
  typedef Elf64_Dyn elf_dynamic;
  typedef Elf64_Sym elf_sym;
};

template <class dynamic_type>
bool
got_finder::fill_impl_with_dyn (unsigned long start, unsigned long end,
                                ptracer *ptracer)
{
  char buf[end - start];
  if (!ptracer->read_memory (buf, end - start, start))
    {
      return false;
    }
  dynamic_type *pdyn = reinterpret_cast<dynamic_type *> (buf);
  dynamic_type *pdyn_end
      = reinterpret_cast<dynamic_type *> (buf + (end - start));
  impl_->filled_mask_ = 0;
  for (; pdyn < pdyn_end; ++pdyn)
    {
      switch (pdyn->d_tag)
        {
        case DT_SYMTAB:
          impl_->symtab_start_ = pdyn->d_un.d_ptr;
          impl_->filled_mask_ |= symtab_start_bit;
          break;
        case DT_STRTAB:
          impl_->strtab_start_ = pdyn->d_un.d_ptr;
          impl_->filled_mask_ |= strtab_start_bit;
          break;
        case DT_STRSZ:
          impl_->strtab_size_ = pdyn->d_un.d_val;
          impl_->filled_mask_ |= strtab_size_bit;
          break;
        case DT_JMPREL:
          impl_->jmprel_start_ = pdyn->d_un.d_ptr;
          impl_->filled_mask_ |= jmprel_start_bit;
          break;
        case DT_PLTRELSZ:
          impl_->plt_got_size_ = pdyn->d_un.d_val;
          impl_->filled_mask_ |= plt_got_size_bit;
          break;
        case DT_PLTREL:
          impl_->plt_got_rel_type_ = pdyn->d_un.d_val;
          impl_->filled_mask_ |= plt_got_rel_type_bit;
          break;
        }
    }
  if (!is_mandatory_set (impl_->filled_mask_))
    {
      LOGE ("got_finder::fill_impl_with_dyn: fails to fill all mandatory "
            "values\n");
      return false;
    }
  return true;
}

template <class elf_header_type>
bool
got_finder::deal_with_elf (unsigned long start, unsigned long end,
                           ptracer *ptracer, const char *name,
                           got_finder_client *client)
{
  elf_header_type hdr;
  typedef typename ::elf_deducer<elf_header_type>::elf_phdr elf_phdr;
  typedef typename ::elf_deducer<elf_header_type>::elf_rela elf_rela;
  typedef typename ::elf_deducer<elf_header_type>::elf_rel elf_rel;
  typedef typename ::elf_deducer<elf_header_type>::elf_dynamic elf_dynamic;
  assert ((sizeof (hdr) & (sizeof (intptr_t) - 1)) == 0);
  if (!ptracer->read_memory (&hdr, sizeof (hdr), start))
    {
      impl_->fatal_occured_ = true;
      return false;
    }
  elf_phdr phdr;
  assert ((sizeof (phdr) & (sizeof (intptr_t) - 1)) == 0);
  intptr_t phdr_indexer = start + hdr.e_phoff;
  for (int i = 0; i < hdr.e_phnum; ++i, phdr_indexer += hdr.e_phentsize)
    {
      if (!ptracer->read_memory (&phdr, sizeof (phdr), phdr_indexer))
        {
          impl_->fatal_occured_ = true;
          return false;
        }
      if (phdr.p_type == PT_DYNAMIC)
        break;
    }
  if (phdr.p_type != PT_DYNAMIC)
    {
      LOGE ("got_finder::deal_with_elf fails to find dynamic program "
            "header.\n");
      return false;
    }
  unsigned long start_of_dyn = phdr.p_vaddr + start;
  unsigned long end_of_dyn = start_of_dyn + phdr.p_memsz;
  if (!fill_impl_with_dyn<elf_dynamic> (start_of_dyn, end_of_dyn, ptracer))
    {
      LOGE ("got_finder::deal_with_elf fill_impl_with_dyn fails\n");
      return false;
    }
  // fill str table
  std::string strtab;
  size_t rounded_strtab_size = round_up (impl_->strtab_size_);
  strtab.reserve (rounded_strtab_size);
  intptr_t strtab_start = impl_->strtab_start_;
#if defined(ANDROID)
  strtab_start += start;
#endif
  if (!ptracer->read_memory (const_cast<char *> (strtab.data ()),
                             rounded_strtab_size, strtab_start))
    {
      LOGE ("got_finder::deal_with_elf fill strtab fails, dest %lx\n",
            strtab_start);
      impl_->fatal_occured_ = true;
      return false;
    }
  switch (impl_->plt_got_rel_type_)
    {
    case DT_RELA:
      return impl_->deal_with_elf_relocation<elf_rela, elf_header_type> (
          start, end, ptracer, name, client, strtab);
    case DT_REL:
      return impl_->deal_with_elf_relocation<elf_rel, elf_header_type> (
          start, end, ptracer, name, client, strtab);
    default:
      LOGE ("got_finder::deal_with_elf: unexpected plt got rel type.\n");
      impl_->fatal_occured_ = true;
      assert (false);
    }
}

template <class relocation_type> struct relocation_deducer
{
};

struct relocation_deducer_base_32
{
  static inline Elf32_Word
  sym_index (Elf32_Word rinfo)
  {
    return ELF32_R_SYM (rinfo);
  }

  static inline Elf32_Word
  type (Elf32_Word rinfo)
  {
    return ELF32_R_TYPE (rinfo);
  }
};

struct relocation_deducer_base_64
{
  static inline Elf64_Xword
  sym_index (Elf64_Xword rinfo)
  {
    return ELF64_R_SYM (rinfo);
  }

  static inline Elf64_Xword
  type (Elf64_Xword rinfo)
  {
    return ELF64_R_TYPE (rinfo);
  }
};

template <> struct relocation_deducer<Elf32_Rel> : relocation_deducer_base_32
{
  static const bool is_rela_ = false;
};

template <> struct relocation_deducer<Elf64_Rel> : relocation_deducer_base_64
{
  static const bool is_rela_ = false;
};

template <> struct relocation_deducer<Elf32_Rela> : relocation_deducer_base_32
{
  static const bool is_rela_ = true;
};

template <> struct relocation_deducer<Elf64_Rela> : relocation_deducer_base_64
{
  static const bool is_rela_ = true;
};

got_finder::got_finder_impl::got_finder_impl () : fatal_occured_ (false) {}

template <class elf_relocation_type, class elf_header_type>
bool
got_finder::got_finder_impl::deal_with_elf_relocation (
    unsigned long start, unsigned long end, ptracer *ptracer, const char *name,
    got_finder_client *client, std::string &strtab)
{
  size_t rounded_plt_got_size = round_up (plt_got_size_);
  char buf[rounded_plt_got_size];
  intptr_t jmprel_start = jmprel_start_;
#if defined(ANDROID)
  jmprel_start += start;
#endif
  if (!ptracer->read_memory (buf, rounded_plt_got_size, jmprel_start))
    {
      LOGE ("got_finder::got_finder_impl::deal_with_elf_relocation:read "
            "memory fails, %d.\n",
            __LINE__);
      fatal_occured_ = true;
      return false;
    }

  typedef relocation_deducer<elf_relocation_type> myreldeducer;
  typedef elf_deducer<elf_header_type> myelfdeducer;

  for (
      elf_relocation_type *r = reinterpret_cast<elf_relocation_type *> (buf);
      r < reinterpret_cast<elf_relocation_type *> (buf + rounded_plt_got_size);
      ++r)
    {
      if (myreldeducer::type (r->r_info) != DESIGNATIVE_R_TYPE)
        {
          continue;
        }
      size_t rounded_sym_size
          = round_up (sizeof (typename myelfdeducer::elf_sym));
      char buf2[rounded_sym_size];
      int sym_index = myreldeducer::sym_index (r->r_info);
      intptr_t symtab_start = symtab_start_;
#if defined(ANDROID)
      symtab_start += start;
#endif
      if (!ptracer->read_memory (buf2, rounded_sym_size,
                                 sizeof (typename myelfdeducer::elf_sym)
                                         * sym_index
                                     + symtab_start))
        {
          LOGE ("got_finder::got_finder_impl::deal_with_elf_relocation:read "
                "memory fails, %d.\n",
                __LINE__);
          fatal_occured_ = true;
          return false;
        }
      typename myelfdeducer::elf_sym *sym
          = reinterpret_cast<typename myelfdeducer::elf_sym *> (buf2);
      if (strcmp (strtab.data () + sym->st_name, name) != 0)
        {
          continue;
        }
      // a match.
      // read back the address first.
      intptr_t addr;
      if (!ptracer->read_memory (&addr, sizeof (addr), r->r_offset + start))
        {
          fatal_occured_ = true;
          return false;
        }
      return client->found (ptracer, r->r_offset + start, addr);
    }
  return false;
}
