#ifdef ANDROID
#include <android/log.h>
#else
#include <stdio.h>
#endif // ANDROID
class shit
{
public:
  shit ();
};

shit::shit ()
{
#ifdef ANDROID
  __android_log_print (ANDROID_LOG_DEBUG, "LINZJ", "shit::shit\n");
#else
  printf ("shit::shit\n");
#endif // ANDROID
}

static shit s;
