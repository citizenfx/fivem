/*
* (C) 2016 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/

#ifndef BOTAN_UTIL_MUTEX_H__
#define BOTAN_UTIL_MUTEX_H__

#include <botan/build.h>
#include <botan/types.h>

#if defined(BOTAN_TARGET_OS_HAS_THREADS)

#include <mutex>

namespace Botan {

template<typename T> using lock_guard_type = std::lock_guard<T>;
typedef std::mutex mutex_type;

}

#elif defined(BOTAN_TARGET_OS_TYPE_IS_UNIKERNEL)

// No threads

namespace Botan {

template<typename Mutex>
class lock_guard
   {
   public:
      explicit lock_guard(Mutex& m) : m_mutex(m)
         { m_mutex.lock(); }

      ~lock_guard() { m_mutex.unlock(); }

      lock_guard(const lock_guard& other) = delete;
      lock_guard& operator=(const lock_guard& other) = delete;
   private:
      Mutex& m_mutex;
   };

class noop_mutex
   {
   public:
      void lock() {}
      void unlock() {}
   };

typedef noop_mutex mutex_type;
template<typename T> using lock_guard_type = lock_guard<T>;

}

#else
  #error "Threads unexpectedly disabled in non unikernel build"
#endif

#endif
