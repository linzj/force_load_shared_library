#include "plt_caller.h"
#include "ptracer.h"
#include "log.h"
#include "round.h"
#include <dlfcn.h>
#include <string.h>

plt_caller::plt_caller () {}

struct user_regs_struct : pt_regs
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
