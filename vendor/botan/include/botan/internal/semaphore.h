/*
* Semaphore
* (C) 2013 Joel Low
*
* Botan is released under the Simplified BSD License (see license.txt)
*/

#ifndef BOTAN_SEMAPHORE_H__
#define BOTAN_SEMAPHORE_H__

#include <botan/mutex.h>

#if defined(BOTAN_TARGET_OS_HAS_THREADS)
#include <condition_variable>
#endif

namespace Botan {

#if defined(BOTAN_TARGET_OS_HAS_THREADS)
class Semaphore
   {
   public:
      explicit Semaphore(int value = 0) : m_value(value), m_wakeups(0) {}

      void acquire();

      void release(size_t n = 1);

   private:
      int m_value;
      int m_wakeups;
      mutex_type m_mutex;
      std::condition_variable m_cond;
   };
#endif

}

#endif
