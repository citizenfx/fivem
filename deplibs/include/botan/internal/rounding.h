/*
* Integer Rounding Functions
* (C) 1999-2007 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/

#ifndef BOTAN_ROUNDING_H__
#define BOTAN_ROUNDING_H__

#include <botan/types.h>

namespace Botan {

/**
* Round up
* @param n an integer
* @param align_to the alignment boundary
* @return n rounded up to a multiple of align_to
*/
template<typename T>
inline T round_up(T n, T align_to)
   {
   if(align_to == 0)
      return n;

   if(n % align_to || n == 0)
      n += align_to - (n % align_to);
   return n;
   }

/**
* Round down
* @param n an integer
* @param align_to the alignment boundary
* @return n rounded down to a multiple of align_to
*/
template<typename T>
inline T round_down(T n, T align_to)
   {
   if(align_to == 0)
      return n;

   return (n - (n % align_to));
   }

/**
* Clamp
*/
inline size_t clamp(size_t n, size_t lower_bound, size_t upper_bound)
   {
   if(n < lower_bound)
      return lower_bound;
   if(n > upper_bound)
      return upper_bound;
   return n;
   }

}

#endif
