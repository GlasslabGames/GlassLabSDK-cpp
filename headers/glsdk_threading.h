#ifndef GLSDK_THREADING
#define GLSDK_THREADING

#if _WIN32
  #define WINTHREAD_ENABLED
  #include <Windows.h>
#else
#define PTHREAD_ENABLED
  #include <pthread.h>
#endif

#ifdef WINTHREAD_ENABLED
  void gl_lockMutex(HANDLE &mutex);
#elif defined(PTHREAD_ENABLED)
  void gl_lockMutex(pthread_mutex_t &mutex);
#endif

#ifdef WINTHREAD_ENABLED
void gl_unlockMutex(HANDLE &mutex);
#elif defined(PTHREAD_ENABLED)
void gl_unlockMutex(pthread_mutex_t &mutex);
#endif

#ifdef WINTHREAD_ENABLED
  void gl_broadcastEvent(HANDLE &condition);
#elif defined(PTHREAD_ENABLED)
  void gl_broadcastEvent(pthread_cond_t &condition);
#endif

#ifdef WINTHREAD_ENABLED
  void gl_waitEvent(HANDLE &condition, HANDLE &mutex);
#elif defined(PTHREAD_ENABLED)
  void gl_waitEvent(pthread_cond_t &condition, pthread_mutex_t &mutex);
#endif

#endif // GLSDK_THREADING