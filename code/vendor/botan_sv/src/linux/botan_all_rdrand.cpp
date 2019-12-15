/*
* Botan 2.12.1 Amalgamation
* (C) 1999-2018 The Botan Authors
*
* Botan is released under the Simplified BSD License (see license.txt)
*/

#include "botan_all.h"
#include "botan_all_internal.h"

#if defined(__GNUG__) && !defined(__clang__)
#pragma GCC target ("rdrnd")
#endif
/*
* RDRAND RNG
* (C) 2016,2019 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/


#if !defined(BOTAN_USE_GCC_INLINE_ASM)
  #include <immintrin.h>
#endif

namespace Botan {

namespace {

#if defined(BOTAN_TARGET_ARCH_IS_X86_64)
   typedef uint64_t rdrand_output;
#else
   typedef uint32_t rdrand_output;
#endif

BOTAN_FUNC_ISA("rdrnd")
rdrand_output read_rdrand()
   {
   /*
   * According to Intel, RDRAND is guaranteed to generate a random
   * number within 10 retries on a working CPU
   */
   const size_t RDRAND_RETRIES = 10;

   for(size_t i = 0; i < RDRAND_RETRIES; ++i)
      {
      rdrand_output r = 0;
      int cf = 0;

#if defined(BOTAN_USE_GCC_INLINE_ASM)
      // same asm seq works for 32 and 64 bit
      asm("rdrand %0; adcl $0,%1" :
          "=r" (r), "=r" (cf) : "0" (r), "1" (cf) : "cc");
#elif defined(BOTAN_TARGET_ARCH_IS_X86_64)
      cf = _rdrand64_step(&r);
#else
      cf = _rdrand32_step(&r);
#endif
      if(1 == cf)
         {
         return r;
         }
      }

   throw PRNG_Unseeded("RDRAND read failed");
   }

}

void RDRAND_RNG::randomize(uint8_t out[], size_t out_len)
   {
   while(out_len >= sizeof(rdrand_output))
      {
      const rdrand_output r = read_rdrand();
      store_le(r, out);
      out += sizeof(rdrand_output);
      out_len -= sizeof(rdrand_output);
      }

   if(out_len > 0) // at most sizeof(rdrand_output)-1
      {
      const rdrand_output r = read_rdrand();
      for(size_t i = 0; i != out_len; ++i)
         out[i] = get_byte(i, r);
      }
   }

RDRAND_RNG::RDRAND_RNG()
   {
   if(!RDRAND_RNG::available())
      throw Invalid_State("Current CPU does not support RDRAND instruction");
   }

//static
bool RDRAND_RNG::available()
   {
   return CPUID::has_rdrand();
   }

//static
uint32_t RDRAND_RNG::rdrand()
   {
   return static_cast<uint32_t>(read_rdrand());
   }

//static
BOTAN_FUNC_ISA("rdrnd")
uint32_t RDRAND_RNG::rdrand_status(bool& ok)
   {
   ok = false;

   try
      {
      const uint32_t r = static_cast<uint32_t>(read_rdrand());
      ok = true;
      return r;
      }
   catch(PRNG_Unseeded&) {}

   return 0;
   }

}
