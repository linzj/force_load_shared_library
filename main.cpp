#include "plt_caller.h"
#include "ptracer.h"
#include "got_finder.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

class my_got_finder_client : public got_finder_client
{
public:
  my_got_finder_client (const char *);

private:
  virtual bool found (const found_info &);

  const char *so_name_;
};

my_got_finder_client::my_got_finder_client (const char *so_name)
    : so_name_ (so_name)
{
}

bool
my_got_finder_client::found (const found_info &info)
{
  plt_caller caller;
  caller.call (info, so_name_);
  return true;
}

int
main (int argc, char **argv)
{
  if (argc != 3)
    {
      fprintf (stderr, "<pid> <so lib path>");
      exit (1);
    }
  errno = 0;
  pid_t pid = strtoul (argv[1], NULL, 10);
  if (errno)
    {
      perror ("pid strtoul");
      exit (1);
    }

  ptracer ptracer (pid);
  got_finder finder;

  if (!ptracer.attach ())
    {
      fprintf (stderr, "attaching to pid %d fails.\n", pid);
      exit (1);
    }
  my_got_finder_client client (argv[2]);
  finder.find (&ptracer, "dlopen", pid, &client);
  ptracer.detach ();
  return 0;
}
