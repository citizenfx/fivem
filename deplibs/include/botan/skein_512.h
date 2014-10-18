/*
* The Skein-512 hash function
* (C) 2009,2014 Jack Lloyd
*
* Distributed under the terms of the Botan license
*/

#ifndef BOTAN_SKEIN_512_H__
#define BOTAN_SKEIN_512_H__

#include <botan/hash.h>
#include <botan/threefish.h>
#include <string>
#include <memory>

namespace Botan {

/**
* Skein-512, a SHA-3 candidate
*/
class BOTAN_DLL Skein_512 : public HashFunction
   {
   public:
      /**
      * @param output_bits the output size of Skein in bits
      * @param personalization is a string that will paramaterize the
      * hash output
      */
      Skein_512(size_t output_bits = 512,
                const std::string& personalization = "");

      size_t hash_block_size() const { return 64; }
      size_t output_length() const { return output_bits / 8; }

      HashFunction* clone() const;
      std::string name() const;
      void clear();
   private:
      enum type_code {
         SKEIN_KEY = 0,
         SKEIN_CONFIG = 4,
         SKEIN_PERSONALIZATION = 8,
         SKEIN_PUBLIC_KEY = 12,
         SKEIN_KEY_IDENTIFIER = 16,
         SKEIN_NONCE = 20,
         SKEIN_MSG = 48,
         SKEIN_OUTPUT = 63
      };

      void add_data(const byte input[], size_t length);
      void final_result(byte out[]);

      void ubi_512(const byte msg[], size_t msg_len);

      void initial_block();
      void reset_tweak(type_code type, bool final);

      std::string personalization;
      size_t output_bits;

      std::unique_ptr<Threefish_512> m_threefish;
      secure_vector<u64bit> T;
      secure_vector<byte> buffer;
      size_t buf_pos;
   };

}

#endif
