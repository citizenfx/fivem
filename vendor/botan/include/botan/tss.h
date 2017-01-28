/*
* RTSS (threshold secret sharing)
* (C) 2009 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/

#ifndef BOTAN_RTSS_H__
#define BOTAN_RTSS_H__

#include <botan/secmem.h>
#include <botan/hash.h>
#include <botan/rng.h>
#include <vector>

namespace Botan {

/**
* A split secret, using the format from draft-mcgrew-tss-03
*/
class BOTAN_DLL RTSS_Share
   {
   public:
      /**
      * @param M the number of shares needed to reconstruct
      * @param N the number of shares generated
      * @param secret the secret to split
      * @param secret_len the length of the secret
      * @param identifier the 16 byte share identifier
      * @param rng the random number generator to use
      */
      static std::vector<RTSS_Share>
         split(uint8_t M, uint8_t N,
               const uint8_t secret[], uint16_t secret_len,
               const uint8_t identifier[16],
               RandomNumberGenerator& rng);

      /**
      * @param shares the list of shares
      */
      static secure_vector<uint8_t>
        reconstruct(const std::vector<RTSS_Share>& shares);

      RTSS_Share() {}

      /**
      * @param hex_input the share encoded in hexadecimal
      */
      explicit RTSS_Share(const std::string& hex_input);

      /**
      * @return hex representation
      */
      std::string to_string() const;

      /**
      * @return share identifier
      */
      uint8_t share_id() const;

      /**
      * @return size of this share in bytes
      */
      size_t size() const { return m_contents.size(); }

      /**
      * @return if this TSS share was initialized or not
      */
      bool initialized() const { return (m_contents.size() > 0); }
   private:
      secure_vector<uint8_t> m_contents;
   };

}

#endif
