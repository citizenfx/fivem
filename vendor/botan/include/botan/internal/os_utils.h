/*
* OS specific utility functions
* (C) 2015,2016 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/

#ifndef BOTAN_OS_UTILS_H__
#define BOTAN_OS_UTILS_H__

#include <botan/types.h>

namespace Botan {

namespace OS {

/**
* Returns the OS assigned process ID, if available. Otherwise throws.
*/
uint32_t get_process_id();

/**
* Return the highest resolution clock available on the system.
*
* The epoch and update rate of this clock is arbitrary and depending
* on the hardware it may not tick at a constant rate.
*
* Returns the value of the hardware cycle counter, if available.
* On Windows calls QueryPerformanceCounter.
* Under GCC or Clang on supported platforms the hardware cycle counter is queried:
*  x86, PPC, Alpha, SPARC, IA-64, S/390x, and HP-PA
* On other platforms clock_gettime is used with some monotonic timer, if available.
* As a final fallback std::chrono::high_resolution_clock is used.
*/
uint64_t get_processor_timestamp();

/**
* Returns the value of the system clock with best resolution available,
* normalized to nanoseconds resolution.
*/
uint64_t get_system_timestamp_ns();

/*
* Returns the maximum amount of memory (in bytes) we could/should
* hyptothetically allocate. Reads "BOTAN_MLOCK_POOL_SIZE" from
* environment which can be set to zero.
*/
size_t get_memory_locking_limit();

/*
* Request so many bytes of page-aligned RAM locked into memory using
* mlock, VirtualLock, or similar. Returns null on failure. The memory
* returned is zeroed. Free it with free_locked_pages.
*/
void* allocate_locked_pages(size_t length);

/*
* Free memory allocated by allocate_locked_pages
*/
void free_locked_pages(void* ptr, size_t length);

}

}

#endif
