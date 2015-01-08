#include "ptracer.h"
#include "log.h"
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdint.h>

ptracer::ptracer (pid_t tid) : tid_ (tid), attached_ (false) {}
ptracer::~ptracer () { detach (); }

bool
ptracer::attach ()
{
  if (attached_)
    return true;
  int ret = ptrace (PTRACE_ATTACH, tid_, 0, 0);
  if (ret == -1)
    {
      LOGE ("ptracer::attach:fails to attach : %s\n", strerror (errno));
      return false;
    }
  int status;
  ret = waitpid (tid_, &status, 0);
  if (ret == -1)
    {
      LOGE ("ptracer::attach:fails to wait: %s\n", strerror (errno));
      ptrace (PTRACE_DETACH, tid_, NULL, NULL);
      return false;
    }
  attached_ = true;
  return true;
}

void
ptracer::detach ()
{
  if (attached_)
    ptrace (PTRACE_DETACH, tid_, NULL, NULL);
  attached_ = false;
}

void
ptracer::continue_and_wait ()
{
  if (!attached_)
    return;
  ptrace (PTRACE_CONT, tid_, NULL, NULL);
  int status;
  ret = waitpid (tid_, &status, 0);
  if (ret == -1)
    {
      LOGE ("ptracer::continue_and_wait:fails to wait: %s\n",
            strerror (errno));
    }
}

template <class T>
inline static bool
check_align (T t)
{
  intptr_t i = reinterpret_cast<intptr_t> (t);
  if (i & (sizeof (intptr_t) - 1))
    return false;
  return true;
}

bool
ptracer::read_memory (void *buffer, size_t s, intptr_t dest)
{
  if (!attached_)
    return false;
  if (!check_align (buffer))
    {
      LOGE ("ptracer::read_memory: buffer not align to word size of the "
            "machine.\n");
      return false;
    }
  if (!check_align (s))
    {
      LOGE (
          "ptracer::read_memory: s not align to word size of the machine.\n");
      return false;
    }
  size_t count = s / sizeof (intptr_t);
  intptr_t *_buffer = static_cast<intptr_t *> (buffer);
  for (size_t i = 0; i < count; ++i, dest += sizeof (intptr_t))
    {
      long v = ptrace (PTRACE_PEEKDATA, tid_, dest, 0);
      if (errno)
        {
          LOGE ("ptracer::read_memory: peek data fail %s.\n",
                strerror (errno));
          return false;
        }
      _buffer[i] = v;
    }
  return true;
}

bool
ptracer::write_memory (void *buffer, size_t s, intptr_t dest)
{
  if (!attached_)
    return false;
  if (!check_align (buffer))
    {
      LOGE ("ptracer::read_memory: buffer not align to word size of the "
            "machine.\n");
      return false;
    }
  if (!check_align (s))
    {
      LOGE (
          "ptracer::read_memory: s not align to word size of the machine.\n");
      return false;
    }
  size_t count = s / sizeof (intptr_t);
  intptr_t *_buffer = static_cast<intptr_t *> (buffer);
  for (size_t i = 0; i < count; ++i, dest += sizeof (intptr_t))
    {
      long r = ptrace (PTRACE_POKEDATA, tid_, dest,
                       reinterpret_cast<void *> (_buffer[i]));
      if (r == -1)
        {
          LOGE ("ptracer::read_memory: poke data fail %s.\n",
                strerror (errno));
          return false;
        }
    }
  return true;
}

bool
ptracer::get_regs (pt_regs *regs)
{
  if (!attached_)
    return false;
  int ret = ptrace (PTRACE_GETREGS, tid_, regs, NULL);
  if (ret == -1)
    {
      LOGE ("ptracer::get_regs: %s\n", strerror (errno));
      return false;
    }
  return true;
}

bool
ptracer::set_regs (pt_regs *regs)
{
  if (!attached_)
    return false;
  int ret = ptrace (PTRACE_SETREGS, tid_, regs, NULL);
  if (ret == -1)
    {
      LOGE ("ptracer::set_regs : %s\n", strerror (errno));
      return false;
    }
  return true;
}
