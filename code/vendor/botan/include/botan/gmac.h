/*
 * GMAC
 * (C) 2016 Matthias Gierlings, René Korthaus
 *
 * Botan is released under the Simplified BSD License (see license.txt)
 */

#ifndef BOTAN_GMAC_H_
#define BOTAN_GMAC_H_

#include <botan/mac.h>
#include <botan/gcm.h>
#include <botan/block_cipher.h>

namespace Botan {

/**
* GMAC
*
* GMAC requires a unique initialization vector be used for each message.
* This must be provided via the MessageAuthenticationCode::start() API
*/
class BOTAN_PUBLIC_API(2,0) GMAC final : public MessageAuthenticationCode, public GHASH
   {
   public:
      void clear() override;
      std::string name() const override;
      size_t output_length() const override;
      MessageAuthenticationCode* clone() const override;

      Key_Length_Specification key_spec() const override
         {
         return m_cipher->key_spec();
         }

      /**
      * Creates a new GMAC instance.
      *
      * @param cipher the underlying block cipher to use
      */
      explicit GMAC(BlockCipher* cipher);

      GMAC(const GMAC&) = delete;
      GMAC& operator=(const GMAC&) = delete;

   private:
      void add_data(const uint8_t[], size_t) override;
      void final_result(uint8_t[]) override;
      void start_msg(const uint8_t nonce[], size_t nonce_len) override;
      void key_schedule(const uint8_t key[], size_t size) override;

      static const size_t GCM_BS = 16;
      secure_vector<uint8_t> m_aad_buf;
      std::unique_ptr<BlockCipher> m_cipher;
      bool m_initialized;
   };

}
#endif
