/*
* Botan 2.12.1 Amalgamation
* (C) 1999-2018 The Botan Authors
*
* Botan is released under the Simplified BSD License (see license.txt)
*/

#include "botan_all.h"
#include "botan_all_internal.h"

#if defined(__GNUG__) && !defined(__clang__)
#pragma GCC target ("rdseed")
#endif
#if defined(__GNUG__) && !defined(__clang__)
#pragma GCC target ("sse2")
#endif
/*
* Entropy Source Using Intel's rdseed instruction
* (C) 2015 Daniel Neus
* (C) 2015,2019 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/


#include <immintrin.h>

namespace Botan {

namespace {

BOTAN_FUNC_ISA("rdseed")
bool read_rdseed(secure_vector<uint32_t>& seed)
   {
   /*
   * RDSEED is not guaranteed to generate an output within any specific number
   * of attempts. However in testing on a Skylake system, with all hyperthreads
   * occupied in tight RDSEED loops, RDSEED will still usually succeed in under
   * 150 attempts. The maximum ever seen was 230 attempts until success. When
   * idle, RDSEED usually succeeds in 1 or 2 attempts.
   *
   * We set an upper bound of 512 attempts, because it is possible that due
   * to firmware issue RDSEED is simply broken and never succeeds. We do not
   * want to loop forever in that case. If we exceed that limit, then we assume
   * the hardware is actually just broken, and stop the poll.
   */
   const size_t RDSEED_RETRIES = 512;

   for(size_t i = 0; i != RDSEED_RETRIES; ++i)
      {
      uint32_t r = 0;
      int cf = 0;

#if defined(BOTAN_USE_GCC_INLINE_ASM)
      asm("rdseed %0; adcl $0,%1" :
          "=r" (r), "=r" (cf) : "0" (r), "1" (cf) : "cc");
#else
      cf = _rdseed32_step(&r);
#endif

      if(1 == cf)
         {
         seed.push_back(r);
         return true;
         }

      // Intel suggests pausing if RDSEED fails.
      _mm_pause();
      }

   return false; // failed to produce an output after many attempts
   }

}

size_t Intel_Rdseed::poll(RandomNumberGenerator& rng)
   {
   const size_t RDSEED_BYTES = 1024;
   static_assert(RDSEED_BYTES % 4 == 0, "Bad RDSEED configuration");

   if(CPUID::has_rdseed())
      {
      secure_vector<uint32_t> seed;
      seed.reserve(RDSEED_BYTES / 4);

      for(size_t p = 0; p != RDSEED_BYTES / 4; ++p)
         {
         /*
         If at any point we exceed our retry count, we stop the entire seed
         gathering process. This situation will only occur in situations of
         extremely high RDSEED utilization. If RDSEED is currently so highly
         contended, then the rest of the poll is likely to also face contention and
         it is better to quit now rather than (presumably) face very high retry
         times for the rest of the poll.
         */
         if(!read_rdseed(seed))
            break;
         }

      if(seed.size() > 0)
         {
         rng.add_entropy(reinterpret_cast<const uint8_t*>(seed.data()),
                         seed.size() * sizeof(uint32_t));
         }
      }

   // RDSEED is used but not trusted
   return 0;
   }

}
