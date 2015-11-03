/*
* CBC-MAC
* (C) 1999-2007 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/

#ifndef BOTAN_CBC_MAC_H__
#define BOTAN_CBC_MAC_H__

#include <botan/mac.h>
#include <botan/block_cipher.h>

namespace Botan {

/**
* CBC-MAC
*/
class BOTAN_DLL CBC_MAC : public MessageAuthenticationCode
   {
   public:
      std::string name() const override;
      MessageAuthenticationCode* clone() const override;
      size_t output_length() const override { return m_cipher->block_size(); }
      void clear() override;

      Key_Length_Specification key_spec() const override
         {
         return m_cipher->key_spec();
         }

      /**
      * @param cipher the underlying block cipher to use
      */
      CBC_MAC(BlockCipher* cipher);

      static CBC_MAC* make(const Spec& spec);
   private:
      void add_data(const byte[], size_t) override;
      void final_result(byte[]) override;
      void key_schedule(const byte[], size_t) override;

      std::unique_ptr<BlockCipher> m_cipher;
      secure_vector<byte> m_state;
      size_t m_position = 0;
   };

}

#endif
