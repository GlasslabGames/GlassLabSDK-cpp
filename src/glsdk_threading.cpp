
#include "glsdk_threading.h"
#include <iostream>
#include <errno.h>

void gl_lockMutex(
  #ifdef WINTHREAD_ENABLED
    HANDLE &mutex
  #elif defined(PTHREAD_ENABLED)
    pthread_mutex_t &mutex
  #endif
  )
{
  #ifdef WINTHREAD_ENABLED
  DWORD result = WaitForSingleObject(mutex, 5000);
  unsigned int i = 0;
  while (result == WAIT_TIMEOUT)
  {
    result = WaitForSingleObject(mutex, 5000);
    i++;
    std::cout << "Thread waiting for mutex for " << (i*5) << " seconds..." << std::endl;
  }
  
  if (result != WAIT_OBJECT_0)
  {
    std::cout << "ERROR: Encountered error when locking mutex - Error code: " << GetLastError() << std::endl;
    throw ERROR_UNHANDLED_EXCEPTION;
  }

  #elif defined(PTHREAD_ENABLED)
    int result = pthread_mutex_trylock(&mutex);
    if (result != 0 && result != EDEADLK)
    {
        result = pthread_mutex_lock(&mutex);
    }
  #endif
}

void gl_unlockMutex(
  #ifdef WINTHREAD_ENABLED
    HANDLE &mutex
  #elif defined(PTHREAD_ENABLED)
    pthread_mutex_t &mutex
  #endif
  )
{
  #ifdef WINTHREAD_ENABLED
    if (!ReleaseMutex(mutex))
    {
        if (GetLastError() == 288)
        {
            std::cout << "WARNING: Attempted to release mutex that wasn't owned!" << std::endl;
        }
        else
        {
            std::cout << "Error occurred when releasing db mutex, Error code: " << GetLastError() << std::endl;
            throw ERROR_UNHANDLED_EXCEPTION;
        }
    }
  #elif defined(PTHREAD_ENABLED)
    pthread_mutex_unlock(&mutex);
  #endif
}

void gl_broadcastEvent(
  #ifdef WINTHREAD_ENABLED
    HANDLE &condition
  #elif defined(PTHREAD_ENABLED)
    pthread_cond_t &condition
  #endif
  )
{
#ifdef WINTHREAD_ENABLED
  if (!SetEvent(condition))
  {
    std::cout << "SetEvent failed, Error code: " << GetLastError() << std::endl;
    throw ERROR_UNHANDLED_EXCEPTION;
  }
#elif defined(PTHREAD_ENABLED)
  pthread_cond_broadcast(&condition);
#endif
}

// gl_waitEvent follows POSIX thread patterns, where the passed in mutex is assumed to be locked upon calling this function,
// then unlocked while waiting for the condition, and locked again when possible after condition is triggered before moving forward.
void gl_waitEvent(
  #ifdef WINTHREAD_ENABLED
    HANDLE &condition, HANDLE &mutex
  #elif defined(PTHREAD_ENABLED)
    pthread_cond_t &condition, pthread_mutex_t &mutex
  #endif
)
{
#ifdef WINTHREAD_ENABLED
  if (!ReleaseMutex(mutex))
  {
    std::cout << "Error occured when attempting to unlocking mutex" << std::endl;
    throw ERROR_UNHANDLED_EXCEPTION;
  }

  DWORD waitReturnCode = WaitForSingleObject(condition, INFINITE);
  if (waitReturnCode == WAIT_OBJECT_0)
  {
    if (WaitForSingleObject(mutex, INFINITE) != WAIT_OBJECT_0)
    {
      std::cout << "Error occured when attempting to lock mutex" << std::endl;
      throw ERROR_UNHANDLED_EXCEPTION;
    }
  }
  else
#elif defined(PTHREAD_ENABLED)
  int waitReturnCode = pthread_cond_wait(&condition, &mutex);
  if (waitReturnCode != 0)
#endif
  {
    std::cout << "Error occurred when waiting on condition, return code received: %i" <<
#ifdef WINTHREAD_ENABLED
      GetLastError()
#elif defined(PTHREAD_ENABLED)
      waitReturnCode
#endif
    << std::endl;

    throw;
  }
}