/*
* (C) 2015 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/

#ifndef BOTAN_EME_RAW_H__
#define BOTAN_EME_RAW_H__

#include <botan/eme.h>

namespace Botan {

class BOTAN_DLL EME_Raw final : public EME
   {
   public:
      size_t maximum_input_size(size_t i) const override;

      EME_Raw() {}
   private:
      secure_vector<uint8_t> pad(const uint8_t[], size_t, size_t,
                             RandomNumberGenerator&) const override;

      secure_vector<uint8_t> unpad(uint8_t& valid_mask,
                                const uint8_t in[],
                                size_t in_len) const override;
   };

}

#endif
