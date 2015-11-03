/*
* Functions for constant time operations on data and testing of
* constant time annotations using ctgrind.
*
* For more information about constant time programming see
* Wagner, Molnar, et al "The Program Counter Security Model"
*
* (C) 2010 Falko Strenzke
* (C) 2015 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/

#ifndef BOTAN_TIMING_ATTACK_CM_H__
#define BOTAN_TIMING_ATTACK_CM_H__

#include <botan/secmem.h>
#include <vector>

#if defined(BOTAN_USE_CTGRIND)

// These are external symbols from libctgrind.so
extern "C" void ct_poison(const void* address, size_t length);
extern "C" void ct_unpoison(const void* address, size_t length);

#endif

namespace Botan {

namespace CT {

template<typename T>
inline void poison(T* p, size_t n)
   {
#if defined(BOTAN_USE_CTGRIND)
   ct_poison(p, sizeof(T)*n);
#else
   BOTAN_UNUSED(p);
   BOTAN_UNUSED(n);
#endif
   }

template<typename T>
inline void unpoison(T* p, size_t n)
   {
#if defined(BOTAN_USE_CTGRIND)
   ct_unpoison(p, sizeof(T)*n);
#else
   BOTAN_UNUSED(p);
   BOTAN_UNUSED(n);
#endif
   }

template<typename T>
inline void unpoison(T& p)
   {
   unpoison(&p, 1);
   }

/*
* T should be an unsigned machine integer type
* Expand to a mask used for other operations
* @param in an integer
* @return If n is zero, returns zero. Otherwise
* returns a T with all bits set for use as a mask with
* select.
*/
template<typename T>
inline T expand_mask(T x)
   {
   T r = x;
   // First fold r down to a single bit
   for(size_t i = 1; i != sizeof(T)*8; i *= 2)
      r |= r >> i;
   r &= 1;
   r = ~(r - 1);
   return r;
   }

template<typename T>
inline T select(T mask, T from0, T from1)
   {
   return (from0 & mask) | (from1 & ~mask);
   }

template<typename T>
inline T is_zero(T x)
   {
   return ~expand_mask(x);
   }

template<typename T>
inline T is_equal(T x, T y)
   {
   return is_zero(x ^ y);
   }

template<typename T>
inline T is_less(T x, T y)
   {
   /*
   This expands to a constant time sequence with GCC 5.2.0 on x86-64
   but something more complicated may be needed for portable const time.
   */
   return expand_mask<T>(x < y);
   }

template<typename T>
inline void conditional_copy_mem(T value,
                                 T* to,
                                 const T* from0,
                                 const T* from1,
                                 size_t bytes)
   {
   const T mask = CT::expand_mask(value);

   for(size_t i = 0; i != bytes; ++i)
      to[i] = CT::select(mask, from0[i], from1[i]);
   }

template<typename T>
inline T expand_top_bit(T a)
   {
   return expand_mask<T>(a >> (sizeof(T)*8-1));
   }

template<typename T>
inline T max(T a, T b)
   {
   const T a_larger = b - a; // negative if a is larger
   return select(expand_top_bit(a), a, b);
   }

template<typename T>
inline T min(T a, T b)
   {
   const T a_larger = b - a; // negative if a is larger
   return select(expand_top_bit(b), b, a);
   }

template<typename T, typename Alloc>
std::vector<T, Alloc> strip_leading_zeros(const std::vector<T, Alloc>& input)
   {
   size_t leading_zeros = 0;

   uint8_t only_zeros = 0xFF;

   for(size_t i = 0; i != input.size(); ++i)
      {
      only_zeros &= CT::is_zero(input[i]);
      leading_zeros += CT::select<uint8_t>(only_zeros, 1, 0);
      }

   return secure_vector<byte>(input.begin() + leading_zeros, input.end());
   }

}

}

#endif
