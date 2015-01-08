#include <stdio.h>
class shit
{
public:
  shit ();
};

shit::shit () { printf ("shit::shit\n"); }

static shit s;
