/*
* OCSP subtypes
* (C) 2012 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/

#ifndef BOTAN_OCSP_TYPES_H_
#define BOTAN_OCSP_TYPES_H_

#include <botan/x509cert.h>
#include <botan/asn1_time.h>
#include <botan/bigint.h>

namespace Botan {

namespace OCSP {

class BOTAN_PUBLIC_API(2,0) CertID final : public ASN1_Object
   {
   public:
      CertID() = default;

      CertID(const X509_Certificate& issuer,
             const BigInt& subject_serial);

      bool is_id_for(const X509_Certificate& issuer,
                     const X509_Certificate& subject) const;

      void encode_into(class DER_Encoder& to) const override;

      void decode_from(class BER_Decoder& from) override;

      const std::vector<uint8_t>& issuer_key_hash() const { return m_issuer_key_hash; }

   private:
      AlgorithmIdentifier m_hash_id;
      std::vector<uint8_t> m_issuer_dn_hash;
      std::vector<uint8_t> m_issuer_key_hash;
      BigInt m_subject_serial;
   };

class BOTAN_PUBLIC_API(2,0) SingleResponse final : public ASN1_Object
   {
   public:
      const CertID& certid() const { return m_certid; }

      size_t cert_status() const { return m_cert_status; }

      X509_Time this_update() const { return m_thisupdate; }

      X509_Time next_update() const { return m_nextupdate; }

      void encode_into(class DER_Encoder& to) const override;

      void decode_from(class BER_Decoder& from) override;
   private:
      CertID m_certid;
      size_t m_cert_status = 2; // unknown
      X509_Time m_thisupdate;
      X509_Time m_nextupdate;
   };

}

}

#endif
