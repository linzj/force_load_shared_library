#ifndef PLT_CALLER_H
#define PLT_CALLER_H
#include <memory>
#include <stdint.h>

class ptracer;

class plt_caller
{
public:
  plt_caller ();
  bool call (ptracer *ptracer, intptr_t plt_got_location,
             intptr_t target_location, intptr_t target, const char *soname);

private:
};
#endif /* PLT_CALLER_H */
