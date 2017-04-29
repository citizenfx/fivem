/*
 * GMAC
 * (C) 2016 Matthias Gierlings, René Korthaus
 *
 * Botan is released under the Simplified BSD License (see license.txt)
 */

#ifndef BOTAN_GMAC_H__
#define BOTAN_GMAC_H__

#include <botan/gcm.h>
#include <botan/mac.h>
#include <botan/types.h>
#include <algorithm>

namespace Botan {

/**
* GMAC
*/
class BOTAN_DLL GMAC : public MessageAuthenticationCode,
                       public GHASH

   {
   public:
      void clear() override;
      std::string name() const override;
      size_t output_length() const override;
      MessageAuthenticationCode* clone() const override;

      /**
      * Must be called to set the initialization vector prior to GMAC
      * calculation.
      *
      * @param nonce Initialization vector.
      * @param nonce_len size of initialization vector.
      */
      void start(const uint8_t nonce[], size_t nonce_len);

      /**
      * Must be called to set the initialization vector prior to GMAC
      * calculation.
      *
      * @param nonce Initialization vector.
      */
      void start(const secure_vector<uint8_t>& nonce);

      /**
      * Must be called to set the initialization vector prior to GMAC
      * calculation.
      *
      * @param nonce Initialization vector.
      */
      void start(const std::vector<uint8_t>& nonce);

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
