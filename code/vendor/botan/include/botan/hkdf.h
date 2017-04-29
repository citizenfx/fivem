/*
* HKDF
* (C) 2013,2015 Jack Lloyd
* (C) 2016 René Korthaus, Rohde & Schwarz Cybersecurity
*
* Botan is released under the Simplified BSD License (see license.txt)
*/

#ifndef BOTAN_HKDF_H__
#define BOTAN_HKDF_H__

#include <botan/mac.h>
#include <botan/hash.h>
#include <botan/kdf.h>

namespace Botan {

/**
* HKDF from RFC 5869.
*/
class BOTAN_DLL HKDF final : public KDF
   {
   public:
      /**
      * @param prf MAC algorithm to use
      */
      explicit HKDF(MessageAuthenticationCode* prf) : m_prf(prf) {}

      KDF* clone() const override { return new HKDF(m_prf->clone()); }

      std::string name() const override { return "HKDF(" + m_prf->name() + ")"; }

      size_t kdf(uint8_t key[], size_t key_len,
                 const uint8_t secret[], size_t secret_len,
                 const uint8_t salt[], size_t salt_len,
                 const uint8_t label[], size_t label_len) const override;

   private:
      std::unique_ptr<MessageAuthenticationCode> m_prf;
   };

/**
* HKDF Extraction Step from RFC 5869.
*/
class BOTAN_DLL HKDF_Extract final : public KDF
   {
   public:
      /**
      * @param prf MAC algorithm to use
      */
      explicit HKDF_Extract(MessageAuthenticationCode* prf) : m_prf(prf) {}

      KDF* clone() const override { return new HKDF_Extract(m_prf->clone()); }

      std::string name() const override { return "HKDF-Extract(" + m_prf->name() + ")"; }

      size_t kdf(uint8_t key[], size_t key_len,
                 const uint8_t secret[], size_t secret_len,
                 const uint8_t salt[], size_t salt_len,
                 const uint8_t label[], size_t label_len) const override;

   private:
      std::unique_ptr<MessageAuthenticationCode> m_prf;
   };

/**
* HKDF Expansion Step from RFC 5869.
*/
class BOTAN_DLL HKDF_Expand final : public KDF
   {
   public:
      /**
      * @param prf MAC algorithm to use
      */
      explicit HKDF_Expand(MessageAuthenticationCode* prf) : m_prf(prf) {}

      KDF* clone() const override { return new HKDF_Expand(m_prf->clone()); }

      std::string name() const override { return "HKDF-Expand(" + m_prf->name() + ")"; }

      size_t kdf(uint8_t key[], size_t key_len,
                 const uint8_t secret[], size_t secret_len,
                 const uint8_t salt[], size_t salt_len,
                 const uint8_t label[], size_t label_len) const override;

   private:
      std::unique_ptr<MessageAuthenticationCode> m_prf;
   };

}

#endif
