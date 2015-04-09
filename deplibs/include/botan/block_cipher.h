/*
* Block Cipher Base Class
* (C) 1999-2009 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/

#ifndef BOTAN_BLOCK_CIPHER_H__
#define BOTAN_BLOCK_CIPHER_H__

#include <botan/scan_name.h>
#include <botan/sym_algo.h>

namespace Botan {

/**
* This class represents a block cipher object.
*/
class BOTAN_DLL BlockCipher : public SymmetricAlgorithm
   {
   public:
      typedef SCAN_Name Spec;

      /**
      * @return block size of this algorithm
      */
      virtual size_t block_size() const = 0;

      /**
      * @return native parallelism of this cipher in blocks
      */
      virtual size_t parallelism() const { return 1; }

      /**
      * @return prefererred parallelism of this cipher in bytes
      */
      size_t parallel_bytes() const
         {
         return parallelism() * block_size() * BOTAN_BLOCK_CIPHER_PAR_MULT;
         }

      /**
      * Encrypt a block.
      * @param in The plaintext block to be encrypted as a byte array.
      * Must be of length block_size().
      * @param out The byte array designated to hold the encrypted block.
      * Must be of length block_size().
      */
      void encrypt(const byte in[], byte out[]) const
         { encrypt_n(in, out, 1); }

      /**
      * Decrypt a block.
      * @param in The ciphertext block to be decypted as a byte array.
      * Must be of length block_size().
      * @param out The byte array designated to hold the decrypted block.
      * Must be of length block_size().
      */
      void decrypt(const byte in[], byte out[]) const
         { decrypt_n(in, out, 1); }

      /**
      * Encrypt a block.
      * @param block the plaintext block to be encrypted
      * Must be of length block_size(). Will hold the result when the function
      * has finished.
      */
      void encrypt(byte block[]) const { encrypt_n(block, block, 1); }

      /**
      * Decrypt a block.
      * @param block the ciphertext block to be decrypted
      * Must be of length block_size(). Will hold the result when the function
      * has finished.
      */
      void decrypt(byte block[]) const { decrypt_n(block, block, 1); }

      /**
      * Encrypt one or more blocks
      * @param block the input/output buffer (multiple of block_size())
      */
      template<typename Alloc>
      void encrypt(std::vector<byte, Alloc>& block) const
         {
         return encrypt_n(&block[0], &block[0], block.size() / block_size());
         }

      /**
      * Decrypt one or more blocks
      * @param block the input/output buffer (multiple of block_size())
      */
      template<typename Alloc>
      void decrypt(std::vector<byte, Alloc>& block) const
         {
         return decrypt_n(&block[0], &block[0], block.size() / block_size());
         }

      /**
      * Encrypt one or more blocks
      * @param in the input buffer (multiple of block_size())
      * @param out the output buffer (same size as in)
      */
      template<typename Alloc, typename Alloc2>
      void encrypt(const std::vector<byte, Alloc>& in,
                   std::vector<byte, Alloc2>& out) const
         {
         return encrypt_n(&in[0], &out[0], in.size() / block_size());
         }

      /**
      * Decrypt one or more blocks
      * @param in the input buffer (multiple of block_size())
      * @param out the output buffer (same size as in)
      */
      template<typename Alloc, typename Alloc2>
      void decrypt(const std::vector<byte, Alloc>& in,
                   std::vector<byte, Alloc2>& out) const
         {
         return decrypt_n(&in[0], &out[0], in.size() / block_size());
         }

      /**
      * Encrypt one or more blocks
      * @param in the input buffer (multiple of block_size())
      * @param out the output buffer (same size as in)
      * @param blocks the number of blocks to process
      */
      virtual void encrypt_n(const byte in[], byte out[],
                             size_t blocks) const = 0;

      /**
      * Decrypt one or more blocks
      * @param in the input buffer (multiple of block_size())
      * @param out the output buffer (same size as in)
      * @param blocks the number of blocks to process
      */
      virtual void decrypt_n(const byte in[], byte out[],
                             size_t blocks) const = 0;

      /**
      * @return new object representing the same algorithm as *this
      */
      virtual BlockCipher* clone() const = 0;
   };

/**
* Represents a block cipher with a single fixed block size
*/
template<size_t BS, size_t KMIN, size_t KMAX = 0, size_t KMOD = 1>
class Block_Cipher_Fixed_Params : public BlockCipher
   {
   public:
      enum { BLOCK_SIZE = BS };
      size_t block_size() const { return BS; }

      Key_Length_Specification key_spec() const
         {
         return Key_Length_Specification(KMIN, KMAX, KMOD);
         }
   };

}

#endif
