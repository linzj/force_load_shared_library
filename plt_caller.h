#ifndef PLT_CALLER_H
#define PLT_CALLER_H
#include <memory>
#include <stdint.h>

class ptracer;
struct found_info;

class plt_caller
{
public:
  plt_caller ();
  bool call (const found_info &info, const char *soname);

private:
};
#endif /* PLT_CALLER_H */
