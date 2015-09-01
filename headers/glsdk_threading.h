#ifndef GLSDK_THREADING
#define GLSDK_THREADING

#if _WIN32
  #define WINTHREAD_ENABLED
  #include <Windows.h>
#else
#define PTHREAD_ENABLED
  #include <pthread.h>
#endif

#ifdef PTHREAD_ENABLED
class GLMutex
{
public:
    pthread_mutex_t mutex;
    pthread_t owner;
    GLMutex();
    ~GLMutex();
};
#endif

#ifdef WINTHREAD_ENABLED
  void gl_lockMutex(HANDLE &mutex);
#elif defined(PTHREAD_ENABLED)
  void gl_lockMutex(GLMutex &mutex);
#endif

#ifdef WINTHREAD_ENABLED
void gl_unlockMutex(HANDLE &mutex);
#elif defined(PTHREAD_ENABLED)
void gl_unlockMutex(GLMutex &mutex);
#endif

#ifdef WINTHREAD_ENABLED
  void gl_broadcastEvent(HANDLE &condition);
#elif defined(PTHREAD_ENABLED)
  void gl_broadcastEvent(pthread_cond_t &condition);
#endif

#ifdef WINTHREAD_ENABLED
  void gl_waitEvent(HANDLE &condition, HANDLE &mutex);
#elif defined(PTHREAD_ENABLED)
  void gl_waitEvent(pthread_cond_t &condition, GLMutex &mutex);
#endif

#endif // GLSDK_THREADING