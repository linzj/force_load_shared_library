#include "plt_caller.h"
#include "ptracer.h"
#include "log.h"
#include "round.h"
#include <dlfcn.h>
#include <string.h>

plt_caller::plt_caller () {}

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

  unsigned long sp = working.rsp;
  sp -= soname_len;
  if (!ptracer->write_memory (buf_so_name, soname_len, sp))
    {
      LOGE ("plt_caller::fails to write memory\n");
      return false;
    }
  // Add 2 to ip comes from these code.
  // So that is the syscall returning to user space
  // with -ERESTARTNOINTR or -ERESTARTNOINTR.
  // static void
  // handle_signal(struct ksignal *ksig, struct pt_regs *regs)
  // {
  // 	bool failed;
  // 	/* Are we from a system call? */
  // 	if (syscall_get_nr(current, regs) >= 0) {
  // 		/* If so, check system call restarting.. */
  // 		switch (syscall_get_error(current, regs)) {
  // 		case -ERESTART_RESTARTBLOCK:
  // 		case -ERESTARTNOHAND:
  // 			regs->ax = -EINTR;
  // 			break;
  //
  // 		case -ERESTARTSYS:
  // 			if (!(ksig->ka.sa.sa_flags & SA_RESTART)) {
  // 				regs->ax = -EINTR;
  // 				break;
  // 			}
  // 		/* fallthrough */
  // 		case -ERESTARTNOINTR:
  // 			regs->ax = regs->orig_ax;
  // 			regs->ip -= 2;
  // 			break;
  // 		}
  // 	}

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
