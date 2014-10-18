/*
* TLS Cipher Suites
* (C) 2004-2011,2012 Jack Lloyd
*
* Released under the terms of the Botan license
*/

#ifndef BOTAN_TLS_CIPHER_SUITES_H__
#define BOTAN_TLS_CIPHER_SUITES_H__

#include <botan/types.h>
#include <string>
#include <vector>

namespace Botan {

namespace TLS {

/**
* Ciphersuite Information
*/
class BOTAN_DLL Ciphersuite
   {
   public:
      /**
      * Convert an SSL/TLS ciphersuite to algorithm fields
      * @param suite the ciphersuite code number
      * @return ciphersuite object
      */
      static Ciphersuite by_id(u16bit suite);

      /**
      * Lookup a ciphersuite by name
      * @param name the name (eg TLS_RSA_WITH_RC4_128_SHA)
      * @return ciphersuite object
      */
      static Ciphersuite by_name(const std::string& name);

      /**
      * Generate a static list of all known ciphersuites and return it.
      *
      * @return list of all known ciphersuites
      */
      static const std::vector<Ciphersuite>& all_known_ciphersuites();

      /**
      * Formats the ciphersuite back to an RFC-style ciphersuite string
      * @return RFC ciphersuite string identifier
      */
      std::string to_string() const;

      /**
      * @return ciphersuite number
      */
      u16bit ciphersuite_code() const { return m_ciphersuite_code; }

      /**
      * @return true if this is a PSK ciphersuite
      */
      bool psk_ciphersuite() const;

      /**
      * @return true if this is an ECC ciphersuite
      */
      bool ecc_ciphersuite() const;

      /**
      * @return key exchange algorithm used by this ciphersuite
      */
      std::string kex_algo() const { return m_kex_algo; }

      /**
      * @return signature algorithm used by this ciphersuite
      */
      std::string sig_algo() const { return m_sig_algo; }

      /**
      * @return symmetric cipher algorithm used by this ciphersuite
      */
      std::string cipher_algo() const { return m_cipher_algo; }

      /**
      * @return message authentication algorithm used by this ciphersuite
      */
      std::string mac_algo() const { return m_mac_algo; }

      std::string prf_algo() const
         {
         return (m_prf_algo != "") ? m_prf_algo : m_mac_algo;
         }

      /**
      * @return cipher key length used by this ciphersuite
      */
      size_t cipher_keylen() const { return m_cipher_keylen; }

      size_t cipher_ivlen() const { return m_cipher_ivlen; }

      size_t mac_keylen() const { return m_mac_keylen; }

      /**
      * @return true if this is a valid/known ciphersuite
      */
      bool valid() const;

      Ciphersuite() {}

   private:

      Ciphersuite(u16bit ciphersuite_code,
                  const char* sig_algo,
                  const char* kex_algo,
                  const char* cipher_algo,
                  size_t cipher_keylen,
                  size_t cipher_ivlen,
                  const char* mac_algo,
                  size_t mac_keylen,
                  const char* prf_algo = "");

      u16bit m_ciphersuite_code = 0;

      std::string m_sig_algo;
      std::string m_kex_algo;
      std::string m_cipher_algo;
      std::string m_mac_algo;
      std::string m_prf_algo;

      size_t m_cipher_keylen = 0;
      size_t m_cipher_ivlen = 0;
      size_t m_mac_keylen = 0;
   };

}

}

#endif
