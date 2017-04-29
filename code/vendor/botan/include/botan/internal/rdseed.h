/*
* Entropy Source Using Intel's rdseed instruction
* (C) 2015 Jack Lloyd, Daniel Neus
*
* Botan is released under the Simplified BSD License (see license.txt)
*/

#ifndef BOTAN_ENTROPY_SRC_RDSEED_H__
#define BOTAN_ENTROPY_SRC_RDSEED_H__

#include <botan/entropy_src.h>

namespace Botan {

/**
* Entropy source using the rdseed instruction first introduced on
* Intel's Broadwell architecture.
*/
class Intel_Rdseed final : public Entropy_Source
   {
   public:
      std::string name() const override { return "rdseed"; }
      size_t poll(RandomNumberGenerator& rng) override;
   };

}

#endif
