/*
* OCSP subtypes
* (C) 2012 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/

#ifndef BOTAN_OCSP_TYPES_H__
#define BOTAN_OCSP_TYPES_H__

#include <botan/x509cert.h>
#include <botan/asn1_time.h>
#include <botan/bigint.h>

namespace Botan {

namespace OCSP {

class BOTAN_DLL CertID : public ASN1_Object
   {
   public:
      CertID() {}

      CertID(const X509_Certificate& issuer,
             const X509_Certificate& subject);

      bool is_id_for(const X509_Certificate& issuer,
                     const X509_Certificate& subject) const;

      void encode_into(class DER_Encoder& to) const override;

      void decode_from(class BER_Decoder& from) override;
   private:
      std::vector<byte> extract_key_bitstr(const X509_Certificate& cert) const;

      AlgorithmIdentifier m_hash_id;
      std::vector<byte> m_issuer_dn_hash;
      std::vector<byte> m_issuer_key_hash;
      BigInt m_subject_serial;
   };

class BOTAN_DLL SingleResponse : public ASN1_Object
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
