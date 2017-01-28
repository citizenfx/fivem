/*
* TLS Cipher Suites
* (C) 2004-2011,2012 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
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
      static Ciphersuite by_id(uint16_t suite);

      /**
      * Returns true iff this suite is a known SCSV
      */
      static bool is_scsv(uint16_t suite);

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
      std::string to_string() const { return m_iana_id; }

      /**
      * @return ciphersuite number
      */
      uint16_t ciphersuite_code() const { return m_ciphersuite_code; }

      /**
      * @return true if this is a PSK ciphersuite
      */
      bool psk_ciphersuite() const;

      /**
      * @return true if this is an ECC ciphersuite
      */
      bool ecc_ciphersuite() const;

      /**
       * @return true if this suite uses a CBC cipher
       */
      bool cbc_ciphersuite() const;

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
         if(m_prf_algo && *m_prf_algo)
            return m_prf_algo;
         return m_mac_algo;
         }

      /**
      * @return cipher key length used by this ciphersuite
      */
      size_t cipher_keylen() const { return m_cipher_keylen; }

      size_t nonce_bytes_from_record() const { return m_nonce_bytes_from_record; }

      size_t nonce_bytes_from_handshake() const { return m_nonce_bytes_from_handshake; }

      size_t mac_keylen() const { return m_mac_keylen; }

      /**
      * @return true if this is a valid/known ciphersuite
      */
      bool valid() const { return m_usable; }

      bool operator<(const Ciphersuite& o) const { return ciphersuite_code() < o.ciphersuite_code(); }
      bool operator<(const uint16_t c) const { return ciphersuite_code() < c; }

      Ciphersuite() {}

   private:

      bool is_usable() const;

      Ciphersuite(uint16_t ciphersuite_code,
                  const char* iana_id,
                  const char* sig_algo,
                  const char* kex_algo,
                  const char* cipher_algo,
                  size_t cipher_keylen,
                  size_t nonce_bytes_from_handshake,
                  size_t nonce_bytes_from_record,
                  const char* mac_algo,
                  size_t mac_keylen,
                  const char* prf_algo) :
         m_ciphersuite_code(ciphersuite_code),
         m_iana_id(iana_id),
         m_sig_algo(sig_algo),
         m_kex_algo(kex_algo),
         m_prf_algo(prf_algo),
         m_cipher_algo(cipher_algo),
         m_mac_algo(mac_algo),
         m_cipher_keylen(cipher_keylen),
         m_nonce_bytes_from_handshake(nonce_bytes_from_handshake),
         m_nonce_bytes_from_record(nonce_bytes_from_record),
         m_mac_keylen(mac_keylen)
         {
         m_usable = is_usable();
         }

      uint16_t m_ciphersuite_code = 0;

      /*
      All of these const char* strings are references to compile time
      constants in tls_suite_info.cpp
      */
      const char* m_iana_id = nullptr;

      const char* m_sig_algo = nullptr;
      const char* m_kex_algo = nullptr;
      const char* m_prf_algo = nullptr;

      const char* m_cipher_algo = nullptr;
      const char* m_mac_algo = nullptr;

      size_t m_cipher_keylen = 0;
      size_t m_nonce_bytes_from_handshake = 0;
      size_t m_nonce_bytes_from_record = 0;
      size_t m_mac_keylen = 0;

      bool m_usable = false;
   };

}

}

#endif
