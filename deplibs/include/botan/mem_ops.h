/*
* Memory Operations
* (C) 1999-2009,2012 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/

#ifndef BOTAN_MEMORY_OPS_H__
#define BOTAN_MEMORY_OPS_H__

#include <botan/types.h>
#include <cstring>

namespace Botan {

/**
* Zeroize memory
* @param ptr a pointer to memory to zero out
* @param n the number of bytes pointed to by ptr
*/
BOTAN_DLL void zero_mem(void* ptr, size_t n);

/**
* Zeroize memory
* @param ptr a pointer to an array
* @param n the number of Ts pointed to by ptr
*/
template<typename T> inline void clear_mem(T* ptr, size_t n)
   {
   std::memset(ptr, 0, sizeof(T)*n);
   }

/**
* Copy memory
* @param out the destination array
* @param in the source array
* @param n the number of elements of in/out
*/
template<typename T> inline void copy_mem(T* out, const T* in, size_t n)
   {
   std::memmove(out, in, sizeof(T)*n);
   }

/**
* Set memory to a fixed value
* @param ptr a pointer to an array
* @param n the number of Ts pointed to by ptr
* @param val the value to set each byte to
*/
template<typename T>
inline void set_mem(T* ptr, size_t n, byte val)
   {
   std::memset(ptr, val, sizeof(T)*n);
   }

/**
* Memory comparison, input insensitive
* @param p1 a pointer to an array
* @param p2 a pointer to another array
* @param n the number of Ts in p1 and p2
* @return true iff p1[i] == p2[i] forall i in [0...n)
*/
template<typename T> inline bool same_mem(const T* p1, const T* p2, size_t n)
   {
   volatile T difference = 0;

   for(size_t i = 0; i != n; ++i)
      difference |= (p1[i] ^ p2[i]);

   return difference == 0;
   }

}

#endif
