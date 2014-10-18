/*
* ANSI X9.19 MAC
* (C) 1999-2007 Jack Lloyd
*
* Distributed under the terms of the Botan license
*/

#ifndef BOTAN_ANSI_X919_MAC_H__
#define BOTAN_ANSI_X919_MAC_H__

#include <botan/mac.h>
#include <botan/block_cipher.h>

namespace Botan {

/**
* DES/3DES-based MAC from ANSI X9.19
*/
class BOTAN_DLL ANSI_X919_MAC : public MessageAuthenticationCode
   {
   public:
      void clear();
      std::string name() const;
      size_t output_length() const { return 8; }

      MessageAuthenticationCode* clone() const;

      Key_Length_Specification key_spec() const
         {
         return Key_Length_Specification(8, 16, 8);
         }

      /**
      * @param cipher the underlying block cipher to use
      */
      ANSI_X919_MAC(BlockCipher* cipher);

      ANSI_X919_MAC(const ANSI_X919_MAC&) = delete;
      ANSI_X919_MAC& operator=(const ANSI_X919_MAC&) = delete;
   private:
      void add_data(const byte[], size_t);
      void final_result(byte[]);
      void key_schedule(const byte[], size_t);

      std::unique_ptr<BlockCipher> m_des1, m_des2;
      secure_vector<byte> m_state;
      size_t m_position;
   };

}

#endif
