#ifndef FOUND_INFO_H
#define FOUND_INFO_H
#pragma once
struct found_info
{
  class ptracer *ptracer;
  intptr_t plt_got_location;
  intptr_t target_location;
  intptr_t target;
};
#endif /* FOUND_INFO_H */
