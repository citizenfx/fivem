/*
* RTSS (threshold secret sharing)
* (C) 2009 Jack Lloyd
*
* Distributed under the terms of the Botan license
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
         split(byte M, byte N,
               const byte secret[], u16bit secret_len,
               const byte identifier[16],
               RandomNumberGenerator& rng);

      /**
      * @param shares the list of shares
      */
      static secure_vector<byte>
        reconstruct(const std::vector<RTSS_Share>& shares);

      RTSS_Share() {}

      /**
      * @param hex_input the share encoded in hexadecimal
      */
      RTSS_Share(const std::string& hex_input);

      /**
      * @return hex representation
      */
      std::string to_string() const;

      /**
      * @return share identifier
      */
      byte share_id() const;

      /**
      * @return size of this share in bytes
      */
      size_t size() const { return contents.size(); }

      /**
      * @return if this TSS share was initialized or not
      */
      bool initialized() const { return (contents.size() > 0); }
   private:
      secure_vector<byte> contents;
   };

}

#endif
