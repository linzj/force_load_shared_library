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
  __android_log_print (ANDROID_LOG_DEBUG, "LINZJ", "shit::shit\n");
}

static shit s;
