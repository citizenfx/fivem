/*
* Mlock Allocator
* (C) 2012 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/

#ifndef BOTAN_MLOCK_ALLOCATOR_H__
#define BOTAN_MLOCK_ALLOCATOR_H__

#include <botan/types.h>
#include <vector>
#include <botan/mutex.h>

namespace Botan {

class BOTAN_DLL mlock_allocator
   {
   public:
      static mlock_allocator& instance();

      void* allocate(size_t num_elems, size_t elem_size);

      bool deallocate(void* p, size_t num_elems, size_t elem_size);

      mlock_allocator(const mlock_allocator&) = delete;

      mlock_allocator& operator=(const mlock_allocator&) = delete;

   private:
      mlock_allocator();

      ~mlock_allocator();

      mutex_type m_mutex;
      std::vector<std::pair<size_t, size_t>> m_freelist;
      uint8_t* m_pool = nullptr;
      size_t m_poolsize = 0;
   };

}

#endif
