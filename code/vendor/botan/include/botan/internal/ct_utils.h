/*
* Functions for constant time operations on data and testing of
* constant time annotations using valgrind.
*
* For more information about constant time programming see
* Wagner, Molnar, et al "The Program Counter Security Model"
*
* (C) 2010 Falko Strenzke
* (C) 2015,2016 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/

#ifndef BOTAN_TIMING_ATTACK_CM_H__
#define BOTAN_TIMING_ATTACK_CM_H__

#include <botan/secmem.h>
#include <vector>

#if defined(BOTAN_HAS_VALGRIND)
  #include <valgrind/memcheck.h>
#endif

namespace Botan {

namespace CT {

/**
* Use valgrind to mark the contents of memory as being undefined.
* Valgrind will accept operations which manipulate undefined values,
* but will warn if an undefined value is used to decided a conditional
* jump or a load/store address. So if we poison all of our inputs we
* can confirm that the operations in question are truly const time
* when compiled by whatever compiler is in use.
*
* Even better, the VALGRIND_MAKE_MEM_* macros work even when the
* program is not run under valgrind (though with a few cycles of
* overhead, which is unfortunate in final binaries as these
* annotations tend to be used in fairly important loops).
*
* This approach was first used in ctgrind (https://github.com/agl/ctgrind)
* but calling the valgrind mecheck API directly works just as well and
* doesn't require a custom patched valgrind.
*/
template<typename T>
inline void poison(const T* p, size_t n)
   {
#if defined(BOTAN_HAS_VALGRIND)
   VALGRIND_MAKE_MEM_UNDEFINED(p, n * sizeof(T));
#else
   BOTAN_UNUSED(p);
   BOTAN_UNUSED(n);
#endif
   }

template<typename T>
inline void unpoison(const T* p, size_t n)
   {
#if defined(BOTAN_HAS_VALGRIND)
   VALGRIND_MAKE_MEM_DEFINED(p, n * sizeof(T));
#else
   BOTAN_UNUSED(p);
   BOTAN_UNUSED(n);
#endif
   }

template<typename T>
inline void unpoison(T& p)
   {
#if defined(BOTAN_HAS_VALGRIND)
   VALGRIND_MAKE_MEM_DEFINED(&p, sizeof(T));
#else
   BOTAN_UNUSED(p);
#endif
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

template<typename PredT, typename ValT>
inline ValT val_or_zero(PredT pred_val, ValT val)
   {
   return select(CT::expand_mask<ValT>(pred_val), val, static_cast<ValT>(0));
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
inline T is_lte(T x, T y)
   {
   return expand_mask<T>(x <= y);
   }

template<typename T>
inline void conditional_copy_mem(T value,
                                 T* to,
                                 const T* from0,
                                 const T* from1,
                                 size_t elems)
   {
   const T mask = CT::expand_mask(value);

   for(size_t i = 0; i != elems; ++i)
      {
      to[i] = CT::select(mask, from0[i], from1[i]);
      }
   }

template<typename T>
inline void cond_zero_mem(T cond,
                          T* array,
                          size_t elems)
   {
   const T mask = CT::expand_mask(cond);
   const T zero(0);

   for(size_t i = 0; i != elems; ++i)
      {
      array[i] = CT::select(mask, zero, array[i]);
      }
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

inline secure_vector<uint8_t> strip_leading_zeros(const uint8_t in[], size_t length)
   {
   size_t leading_zeros = 0;

   uint8_t only_zeros = 0xFF;

   for(size_t i = 0; i != length; ++i)
      {
      only_zeros &= CT::is_zero(in[i]);
      leading_zeros += CT::select<uint8_t>(only_zeros, 1, 0);
      }

   return secure_vector<uint8_t>(in + leading_zeros, in + length);
   }

inline secure_vector<uint8_t> strip_leading_zeros(const secure_vector<uint8_t>& in)
   {
   return strip_leading_zeros(in.data(), in.size());
   }

}

}

#endif
