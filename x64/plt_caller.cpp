#include "plt_caller.h"
#include "ptracer.h"
#include "log.h"
#include "round.h"
#include <dlfcn.h>
#include <string.h>

plt_caller::plt_caller () {}

bool
plt_caller::call (ptracer *ptracer, intptr_t target, const char *soname)
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

  unsigned long sp = working.rsp;
  sp -= soname_len;
  if (!ptracer->write_memory (buf_so_name, soname_len, sp))
    {
      LOGE ("plt_caller::fails to write memory\n");
      return false;
    }
  // FIXME:I dont' know why add 2.
  working.rip = target + 2;
  // the file name.
  working.rdi = sp;
  // the flags.
  working.rsi = RTLD_NOW;

  // now the parameter has been setup. we still need to
  // setup the stack so that it align to 16 bytes.
  sp &= ~(16 - 1);
  unsigned long return_address = -1;
  sp -= sizeof (return_address);
  if (!ptracer->write_memory (&return_address, sizeof (return_address), sp))
    {
      LOGE ("plt_caller::fails to write memory\n");
      return false;
    }
  working.rsp = sp;
  if (!ptracer->set_regs (&working))
    {
      LOGE ("plt_caller::call fails to set regs\n");
      return false;
    }
  LOGI ("ptracer->continue_and_wait trying\n");
  ptracer->continue_and_wait ();
  LOGI ("ptracer->continue_and_wait end\n");
  if (!ptracer->set_regs (&saved))
    {
      LOGE ("plt_caller::call fails to restore regs\n");
      return false;
    }
  return true;
}
