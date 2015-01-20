#include "got_finder.h"
#include "ptracer.h"
#include "log.h"
#include <stdio.h>
#include <assert.h>
#include <dlfcn.h>
#include <errno.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <sys/prctl.h>

class test_got_finder_client : public got_finder_client
{
public:
  test_got_finder_client ();

  inline bool
  is_okay () const
  {
    return okay_;
  };

private:
  virtual bool found (ptracer *ptracer, intptr_t plt_got_location,
                      intptr_t target_location, intptr_t target);
  bool okay_;
};

test_got_finder_client::test_got_finder_client () : okay_ (false) {}

bool
test_got_finder_client::found (ptracer *ptracer, intptr_t plt_got_location,
                               intptr_t target_location, intptr_t target)
{
  LOGI ("test_got_finder_client::found, found dlopen at %08lx\n", target);
  okay_ = true;
  return false;
}

void
hold_dlopen ()
{
  dlopen ("libshit.so", RTLD_NOW);
}

static void
do_test ()
{
  ptracer ptracer (getppid ());
  got_finder finder;

  assert (ptracer.attach ());
  test_got_finder_client client;
  finder.find (&ptracer, "dlopen", getppid (), &client);
  ptracer.detach ();
  assert (client.is_okay ());
}

int
main ()
{
  pid_t forkret;
  int sv[2];

  if (-1 == socketpair (AF_UNIX, SOCK_STREAM, 0, sv))
    {
      LOGE ("pipe fails %s\n", strerror (errno));
      return 0;
    }
  forkret = fork ();

  if (forkret > 0)
    {
      int status;
      int num = 0;
      int readret;
#ifdef PR_SET_PTRACER
      int ptrctlret = prctl (PR_SET_PTRACER, forkret, 0, 0, 0);
      int writeret;
      do
        {
          writeret = write (sv[0], &ptrctlret, sizeof (ptrctlret));
        }
      while (writeret == -1 && errno == EINTR);
      if (ptrctlret == -1)
        {
          LOGE ("ptrctl fails %s\n", strerror (errno));
          close (sv[1]);
          goto fails;
        }
#endif
      close (sv[1]);

      do
        {
          readret = read (sv[0], &num, sizeof (num));
        }
      while (readret == -1 && errno == EINTR);
#ifdef PR_SET_PTRACER
    fails:
#endif
      close (sv[0]);
      waitpid (forkret, &status, 0);
      return num;
    }
  else if (forkret == 0)
    {
      int writeret;
#ifdef PR_SET_PTRACER
      int val;
      int readret;
      do
        {
          readret = read (sv[1], &val, sizeof (val));
        }
      while (readret == -1 && errno == EINTR);
      if (val == -1)
        {
          _exit (0);
        }
#endif
      close (sv[0]);
      do_test ();
      int num = 0;
      do
        {
          writeret = write (sv[1], &num, sizeof (num));
        }
      while (writeret == -1 && errno == EINTR);
      close (sv[1]);
      _exit (0);
    }
  else
    {
      LOGE ("fork fails %s\n", strerror (errno));
      return 0;
    }
  return 0;
}
