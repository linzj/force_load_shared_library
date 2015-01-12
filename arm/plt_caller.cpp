#include "plt_caller.h"
#include "ptracer.h"
#include "log.h"
#include "round.h"
#include <dlfcn.h>
#include <string.h>

plt_caller::plt_caller () {}
namespace _private
{

struct pt_regs
{
  unsigned long uregs[18];
};

#ifndef ARM_cpsr
#define ARM_cpsr uregs[16]
#define ARM_pc uregs[15]
#define ARM_lr uregs[14]
#define ARM_sp uregs[13]
#define ARM_ip uregs[12]
#define ARM_fp uregs[11]
#define ARM_r10 uregs[10]
#define ARM_r9 uregs[9]
#define ARM_r8 uregs[8]
#define ARM_r7 uregs[7]
#define ARM_r6 uregs[6]
#define ARM_r5 uregs[5]
#define ARM_r4 uregs[4]
#define ARM_r3 uregs[3]
#define ARM_r2 uregs[2]
#define ARM_r1 uregs[1]
#define ARM_r0 uregs[0]
#define ARM_ORIG_r0 uregs[17]
#endif
}
struct user_regs_struct : _private::pt_regs
{
};

bool
plt_caller::call (ptracer *ptracer, intptr_t target_location, intptr_t target,
                  const char *soname)
{
  user_regs_struct saved;
  if (!ptracer->get_regs (&saved))
    {
      LOGE ("plt_caller::call fails to saved regs\n");
      return false;
    }
  user_regs_struct working = saved;
  int soname_len = round_up (strlen (soname) + 1);
  char buf_so_name[soname_len];
  strcpy (buf_so_name, soname);

  unsigned long sp = working.ARM_sp;
  sp -= soname_len;
  if (!ptracer->write_memory (buf_so_name, soname_len, sp))
    {
      LOGE ("plt_caller::fails to write memory\n");
      return false;
    }
  int is_thumb = (target & 1);
  target &= ~1;
  working.ARM_ip = target_location;
  working.ARM_pc = target;

  // the file name.
  working.ARM_r0 = sp;
  // the flags.
  working.ARM_r1 = RTLD_NOW;
  working.ARM_cpsr ^= (working.ARM_cpsr & (1 << 5)) ^ (is_thumb << 5);

  // now the parameter has been setup. we still need to
  // setup the stack so that it align to 8 bytes.
  sp &= ~(8 - 1);
  working.ARM_lr = -1;

  working.ARM_sp = sp;
  if (!ptracer->set_regs (&working))
    {
      LOGE ("plt_caller::call fails to set regs\n");
      return false;
    }
  LOGI ("ptracer->continue_and_wait trying\n");
  ptracer->continue_and_wait ();
#if 0
  siginfo_t info;
  ptracer->get_siginfo (&info);
  LOGI ("siginfo si_signo = %d, si_code = %d, si_addr = %08lx\n", info.si_signo, info.si_code, info.si_addr);
  user_regs_struct r;
  ptracer->get_regs (&r);
  LOGI ("r0 = %08lx, r1 = %08lx, r2 = %08lx, r3 = %08lx, sp = %08lx, ip = %08lx, pc = %08lx, cpsr = %08lx\n",
        r.ARM_r0, r.ARM_r1, r.ARM_r2, r.ARM_r3, r.ARM_sp, r.ARM_ip, r.ARM_pc, r.ARM_cpsr);
#endif
  LOGI ("ptracer->continue_and_wait end\n");
  if (!ptracer->set_regs (&saved))
    {
      LOGE ("plt_caller::call fails to restore regs\n");
      return false;
    }
  return true;
}
