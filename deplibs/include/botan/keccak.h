/*
* Keccak
* (C) 2010 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/

#ifndef BOTAN_KECCAK_H__
#define BOTAN_KECCAK_H__

#include <botan/hash.h>
#include <botan/secmem.h>
#include <string>

namespace Botan {

/**
* Keccak[1600], a SHA-3 candidate
*/
class BOTAN_DLL Keccak_1600 : public HashFunction
   {
   public:

      /**
      * @param output_bits the size of the hash output; must be one of
      *                    224, 256, 384, or 512
      */
      Keccak_1600(size_t output_bits = 512);

      size_t hash_block_size() const override { return bitrate / 8; }
      size_t output_length() const override { return output_bits / 8; }

      HashFunction* clone() const override;
      std::string name() const override;
      void clear() override;
   private:
      void add_data(const byte input[], size_t length) override;
      void final_result(byte out[]) override;

      size_t output_bits, bitrate;
      secure_vector<u64bit> S;
      size_t S_pos;
   };

}

#endif
