/*
* Botan 2.0.1 Amalgamation
* (C) 1999-2013,2014,2015,2016 Jack Lloyd and others
*
* Botan is released under the Simplified BSD License (see license.txt)
*/

#include "botan_all.h"
#include "botan_all_internal.h"

#if defined(__GNUG__)
#pragma GCC target ("rdseed")
#endif
/*
* Entropy Source Using Intel's rdseed instruction
* (C) 2015 Jack Lloyd, Daniel Neus
*
* Botan is released under the Simplified BSD License (see license.txt)
*/


#if !defined(BOTAN_USE_GCC_INLINE_ASM)
  #include <immintrin.h>
#endif

namespace Botan {

BOTAN_FUNC_ISA("rdseed")
size_t Intel_Rdseed::poll(RandomNumberGenerator& rng) {
   if(CPUID::has_rdseed())
      {
      for(size_t p = 0; p != BOTAN_ENTROPY_INTEL_RNG_POLLS; ++p)
         {
         for(size_t i = 0; i != BOTAN_ENTROPY_RDSEED_RETRIES; ++i)
            {
            uint32_t r = 0;

#if defined(BOTAN_USE_GCC_INLINE_ASM)
            int cf = 0;

            // Encoding of rdseed %eax
            asm(".byte 0x0F, 0xC7, 0xF8; adcl $0,%1" :
                "=a" (r), "=r" (cf) : "0" (r), "1" (cf) : "cc");
#else
            int cf = _rdseed32_step(&r);
#endif
            if(1 == cf)
               {
               rng.add_entropy_T(r);
               break;
               }
            }
         }
      }

   return 0;
   }

}
