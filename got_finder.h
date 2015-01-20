#ifndef GOT_FINDER_H
#define GOT_FINDER_H
#include <stdint.h>
#include <unistd.h>
#include <memory>

class ptracer;

class got_finder_client
{
public:
  virtual ~got_finder_client ();

  virtual bool found (ptracer *ptracer, intptr_t plt_got_location,
                      intptr_t target_location, intptr_t target) = 0;
};

class got_finder
{
public:
  got_finder ();
  bool find (ptracer *ptracer, const char *name, pid_t tid,
             got_finder_client *client);

private:
  bool check_elf (unsigned long start, ptracer *ptracer, bool *is_elf32);
  bool deal_with (unsigned long start, unsigned long end, ptracer *ptracer,
                  const char *name, got_finder_client *client);
  template <class elf_header_type>
  bool deal_with_elf (unsigned long start, unsigned long end, ptracer *ptracer,
                      const char *name, got_finder_client *client);
  template <class dynamic_type>
  bool fill_impl_with_dyn (unsigned long start, unsigned long end,
                           ptracer *ptracer);
  struct got_finder_impl;
  std::auto_ptr<got_finder_impl> impl_;
};

#endif /* GOT_FINDER_H */
