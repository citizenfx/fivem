/*
* PKCS #5 v2.0 PBE
* (C) 1999-2007 Jack Lloyd
*
* Distributed under the terms of the Botan license
*/

#ifndef BOTAN_PBE_PKCS_v20_H__
#define BOTAN_PBE_PKCS_v20_H__

#include <botan/pbe.h>
#include <botan/block_cipher.h>
#include <botan/mac.h>
#include <botan/pipe.h>
#include <chrono>

namespace Botan {

/**
* PKCS #5 v2.0 PBE
*/
class BOTAN_DLL PBE_PKCS5v20 : public PBE
   {
   public:
      OID get_oid() const;

      std::vector<byte> encode_params() const;

      std::string name() const;

      void write(const byte buf[], size_t buf_len);
      void start_msg();
      void end_msg();

      /**
      * Load a PKCS #5 v2.0 encrypted stream
      * @param params the PBES2 parameters
      * @param passphrase the passphrase to use for decryption
      */
      PBE_PKCS5v20(const std::vector<byte>& params,
                   const std::string& passphrase);

      /**
      * @param cipher the block cipher to use
      * @param mac the MAC to use
      * @param passphrase the passphrase to use for encryption
      * @param msec how many milliseconds to run the PBKDF
      * @param rng a random number generator
      */
      PBE_PKCS5v20(BlockCipher* cipher,
                   MessageAuthenticationCode* mac,
                   const std::string& passphrase,
                   std::chrono::milliseconds msec,
                   RandomNumberGenerator& rng);

      ~PBE_PKCS5v20();
   private:
      void flush_pipe(bool);

      Cipher_Dir direction;
      BlockCipher* block_cipher;
      MessageAuthenticationCode* m_prf;
      secure_vector<byte> salt, key, iv;
      size_t iterations, key_length;
      Pipe pipe;
   };

}

#endif
