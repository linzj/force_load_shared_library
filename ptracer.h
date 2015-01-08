#ifndef PTRACER_H
#define PTRACER_H
#include <unistd.h>
#include <ptrace.h>
#ifndef __ANDROID__
#include <sys/user.h>
#else
#include <asm/user.h>
#endif

class ptracer
{
public:
  ptracer (pid_t tid);
  ~ptracer ();

  bool attach ();
  void detach ();
  void continue_and_wait ();
  bool read_memory (void *buffer, size_t s, intptr_t dest);
  bool write_memory (void *buffer, size_t s, intptr_t dest);
  bool get_regs (pt_regs *);
  bool set_regs (pt_regs *);

private:
  pid_t tid_;
  bool attached_;
};

#endif /* PTRACER_H */
