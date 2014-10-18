/*
* Stream Cipher
* (C) 1999-2007 Jack Lloyd
*
* Distributed under the terms of the Botan license
*/

#ifndef BOTAN_STREAM_CIPHER_H__
#define BOTAN_STREAM_CIPHER_H__

#include <botan/sym_algo.h>

namespace Botan {

/**
* Base class for all stream ciphers
*/
class BOTAN_DLL StreamCipher : public SymmetricAlgorithm
   {
   public:
      /**
      * Encrypt or decrypt a message
      * @param in the plaintext
      * @param out the byte array to hold the output, i.e. the ciphertext
      * @param len the length of both in and out in bytes
      */
      virtual void cipher(const byte in[], byte out[], size_t len) = 0;

      /**
      * Encrypt or decrypt a message
      * @param buf the plaintext / ciphertext
      * @param len the length of buf in bytes
      */
      void cipher1(byte buf[], size_t len)
         { cipher(buf, buf, len); }

      template<typename Alloc>
         void encipher(std::vector<byte, Alloc>& inout)
         { cipher(&inout[0], &inout[0], inout.size()); }

      template<typename Alloc>
         void encrypt(std::vector<byte, Alloc>& inout)
         { cipher(&inout[0], &inout[0], inout.size()); }

      template<typename Alloc>
         void decrypt(std::vector<byte, Alloc>& inout)
         { cipher(&inout[0], &inout[0], inout.size()); }

      /**
      * Resync the cipher using the IV
      * @param iv the initialization vector
      * @param iv_len the length of the IV in bytes
      */
      virtual void set_iv(const byte iv[], size_t iv_len);

      /**
      * @param iv_len the length of the IV in bytes
      * @return if the length is valid for this algorithm
      */
      virtual bool valid_iv_length(size_t iv_len) const;

      /**
      * Get a new object representing the same algorithm as *this
      */
      virtual StreamCipher* clone() const = 0;
   };

}

#endif
