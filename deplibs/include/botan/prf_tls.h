/*
* TLS v1.0 and v1.2 PRFs
* (C) 2004-2010 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/

#ifndef BOTAN_TLS_PRF_H__
#define BOTAN_TLS_PRF_H__

#include <botan/kdf.h>
#include <botan/mac.h>

namespace Botan {

/**
* PRF used in TLS 1.0/1.1
*/
class BOTAN_DLL TLS_PRF : public KDF
   {
   public:
      std::string name() const { return "TLS-PRF"; }

      KDF* clone() const { return new TLS_PRF; }

      size_t kdf(byte key[], size_t key_len,
                 const byte secret[], size_t secret_len,
                 const byte salt[], size_t salt_len) const override;

      TLS_PRF();
   private:
      std::unique_ptr<MessageAuthenticationCode> m_hmac_md5;
      std::unique_ptr<MessageAuthenticationCode> m_hmac_sha1;
   };

/**
* PRF used in TLS 1.2
*/
class BOTAN_DLL TLS_12_PRF : public KDF
   {
   public:
      std::string name() const { return "TLS-12-PRF(" + m_mac->name() + ")"; }

      KDF* clone() const { return new TLS_12_PRF(m_mac->clone()); }

      size_t kdf(byte key[], size_t key_len,
                 const byte secret[], size_t secret_len,
                 const byte salt[], size_t salt_len) const override;

      TLS_12_PRF(MessageAuthenticationCode* mac) : m_mac(mac) {}

      static TLS_12_PRF* make(const Spec& spec);
   private:
      std::unique_ptr<MessageAuthenticationCode> m_mac;
   };

}

#endif
