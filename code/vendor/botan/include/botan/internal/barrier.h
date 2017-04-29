/*
* Barrier
* (C) 2016 Joel Low
*
* Botan is released under the Simplified BSD License (see license.txt)
*/

#ifndef BOTAN_UTIL_BARRIER_H__
#define BOTAN_UTIL_BARRIER_H__

#include <botan/mutex.h>

#if defined(BOTAN_TARGET_OS_HAS_THREADS)
#include <condition_variable>
#endif

namespace Botan {

#if defined(BOTAN_TARGET_OS_HAS_THREADS)
// Barrier implements a barrier synchronization primitive. wait() will indicate
// how many threads to synchronize; each thread needing synchronization should
// call sync(). When sync() returns, the barrier is reset to zero, and the
// m_syncs counter is incremented. m_syncs is a counter to ensure that wait()
// can be called after a sync() even if the previously sleeping threads have
// not awoken.)
class Barrier
    {
    public:
        explicit Barrier(int value = 0) : m_value(value), m_syncs(0) {}

        void wait(unsigned delta);

        void sync();

    private:
        int m_value;
        unsigned m_syncs;
        mutex_type m_mutex;
        std::condition_variable m_cond;
    };
#endif

}

#endif
