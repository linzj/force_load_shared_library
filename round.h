#ifndef ROUND_H
#define ROUND_H
#include <stdint.h>

template <class T>
static inline T
round_up (T t)
{
  return (t + sizeof (intptr_t) - 1) & ~(sizeof (intptr_t) - 1);
}
#endif /* ROUND_H */
