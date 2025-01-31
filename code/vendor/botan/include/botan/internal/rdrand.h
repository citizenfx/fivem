/*
* Entropy Source Using Intel's rdrand instruction
* (C) 2012 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/

#ifndef BOTAN_ENTROPY_SRC_RDRAND_H_
#define BOTAN_ENTROPY_SRC_RDRAND_H_

#include <botan/entropy_src.h>

namespace Botan {

/**
* Entropy source using the rdrand instruction first introduced on
* Intel's Ivy Bridge architecture.
*/
class Intel_Rdrand final : public Entropy_Source
   {
   public:
      std::string name() const override { return "rdrand"; }
      size_t poll(RandomNumberGenerator& rng) override;
   };

}

#endif
