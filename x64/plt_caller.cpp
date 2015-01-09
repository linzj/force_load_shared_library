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
  // Use gdb way:
  // /* We must be careful with modifying the program counter.  If we
  //    just interrupted a system call, the kernel might try to restart
  //    it when we resume the inferior.  On restarting the system call,
  //    the kernel will try backing up the program counter even though it
  //    no longer points at the system call.  This typically results in a
  //    SIGSEGV or SIGILL.  We can prevent this by writing `-1' in the
  //    "orig_eax" pseudo-register.

  //    Note that "orig_eax" is saved when setting up a dummy call frame.
  //    This means that it is properly restored when that frame is
  //    popped, and that the interrupted system call will be restarted
  //    when we resume the inferior on return from a function call from
  //    within GDB.  In all other cases the system call will not be
  //    restarted.  */
  // regcache_cooked_write_unsigned (regcache, I386_LINUX_ORIG_EAX_REGNUM, -1);

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

  working.rip = target;
  working.orig_rax = -1;
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
#if 0
  siginfo_t info;
  ptracer->get_siginfo (&info);
  LOGI ("siginfo si_signo = %d, si_code = %d, si_addr = %08lx, working.rip = "
        "%08lx\n",
        info.si_signo, info.si_code, info.si_addr, working.rip);
  user_regs_struct r;
  ptracer->get_regs (&r);
  LOGI ("rax = %08lx, rbx = %08lx, rcx = %08lx, rdx = %08lx, rdi = %08lx, rsi "
        "= %08lx, rsp = %08lx, rbp = %08lx, rip = %08lx\n",
        r.rax, r.rbx, r.rcx, r.rdx, r.rdi, r.rsi, r.rsp, r.rbp, r.rip);
#endif
  LOGI ("ptracer->continue_and_wait end\n");
  if (!ptracer->set_regs (&saved))
    {
      LOGE ("plt_caller::call fails to restore regs\n");
      return false;
    }
  return true;
}
