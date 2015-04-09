/*
* ECB/CBC Padding Methods
* (C) 1999-2008,2013 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/

#ifndef BOTAN_MODE_PADDING_H__
#define BOTAN_MODE_PADDING_H__

#include <botan/secmem.h>
#include <string>

namespace Botan {

/**
* Block Cipher Mode Padding Method
* This class is pretty limited, it cannot deal well with
* randomized padding methods, or any padding method that
* wants to add more than one block. For instance, it should
* be possible to define cipher text stealing mode as simply
* a padding mode for CBC, which happens to consume the last
* two block (and requires use of the block cipher).
*/
class BOTAN_DLL BlockCipherModePaddingMethod
   {
   public:
      virtual void add_padding(secure_vector<byte>& buffer,
                               size_t final_block_bytes,
                               size_t block_size) const = 0;

      /**
      * @param block the last block
      * @param size the of the block
      */
      virtual size_t unpad(const byte block[],
                           size_t size) const = 0;

      /**
      * @param block_size of the cipher
      * @return valid block size for this padding mode
      */
      virtual bool valid_blocksize(size_t block_size) const = 0;

      /**
      * @return name of the mode
      */
      virtual std::string name() const = 0;

      /**
      * virtual destructor
      */
      virtual ~BlockCipherModePaddingMethod() {}
   };

/**
* PKCS#7 Padding
*/
class BOTAN_DLL PKCS7_Padding : public BlockCipherModePaddingMethod
   {
   public:
      void add_padding(secure_vector<byte>& buffer,
                       size_t final_block_bytes,
                       size_t block_size) const override;

      size_t unpad(const byte[], size_t) const;

      bool valid_blocksize(size_t bs) const { return (bs > 0 && bs < 256); }

      std::string name() const { return "PKCS7"; }
   };

/**
* ANSI X9.23 Padding
*/
class BOTAN_DLL ANSI_X923_Padding : public BlockCipherModePaddingMethod
   {
   public:
      void add_padding(secure_vector<byte>& buffer,
                       size_t final_block_bytes,
                       size_t block_size) const override;

      size_t unpad(const byte[], size_t) const;

      bool valid_blocksize(size_t bs) const { return (bs > 0 && bs < 256); }

      std::string name() const { return "X9.23"; }
   };

/**
* One And Zeros Padding
*/
class BOTAN_DLL OneAndZeros_Padding : public BlockCipherModePaddingMethod
   {
   public:
      void add_padding(secure_vector<byte>& buffer,
                       size_t final_block_bytes,
                       size_t block_size) const override;

      size_t unpad(const byte[], size_t) const;

      bool valid_blocksize(size_t bs) const { return (bs > 0); }

      std::string name() const { return "OneAndZeros"; }
   };

/**
* Null Padding
*/
class BOTAN_DLL Null_Padding : public BlockCipherModePaddingMethod
   {
   public:
      void add_padding(secure_vector<byte>&, size_t, size_t) const override {}

      size_t unpad(const byte[], size_t size) const { return size; }

      bool valid_blocksize(size_t) const { return true; }

      std::string name() const { return "NoPadding"; }
   };

BlockCipherModePaddingMethod* get_bc_pad(const std::string& algo_spec);

}

#endif
