/*
* OCSP
* (C) 2012 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/

#ifndef BOTAN_OCSP_H__
#define BOTAN_OCSP_H__

#include <botan/cert_status.h>
#include <botan/ocsp_types.h>

namespace Botan {

class Certificate_Store;

namespace OCSP {

/**
* An OCSP request.
*/
class BOTAN_DLL Request
   {
   public:
      /**
      * Create an OCSP request.
      * @param issuer_cert issuer certificate
      * @param subject_cert subject certificate
      */
      Request(const X509_Certificate& issuer_cert,
              const X509_Certificate& subject_cert);

      /**
      * @return BER-encoded OCSP request
      */
      std::vector<uint8_t> BER_encode() const;

      /**
      * @return Base64-encoded OCSP request
      */
      std::string base64_encode() const;

      /**
      * @return issuer certificate
      */
      const X509_Certificate& issuer() const { return m_issuer; }

      /**
      * @return subject certificate
      */
      const X509_Certificate& subject() const { return m_subject; }

      const std::vector<uint8_t>& issuer_key_hash() const
         { return m_certid.issuer_key_hash(); }
   private:
      X509_Certificate m_issuer, m_subject;
      CertID m_certid;
   };

/**
* OCSP response.
*
* Note this class is only usable as an OCSP client
*/
class BOTAN_DLL Response
   {
   public:
      /**
      * Creates an empty OCSP response.
      */
      Response() {}

      /**
      * Parses an OCSP response.
      * @param response_bits response bits received
      */
      Response(const std::vector<uint8_t>& response_bits) :
         Response(response_bits.data(), response_bits.size())
         {}

      /**
      * Parses an OCSP response.
      * @param response_bits response bits received
      * @param response_bits_len length of response in bytes
      */
      Response(const uint8_t response_bits[],
               size_t response_bits_len);

      /**
      * Check signature and return status
      * The optional cert_path is the (already validated!) certificate path of
      * the end entity which is being inquired about
      * @param trust_roots list of certstores containing trusted roots
      * @param cert_path optionally, the (already verified!) certificate path for the certificate
      * this is an OCSP response for. This is necessary to find the correct intermediate CA in
      * some cases.
      */
      Certificate_Status_Code check_signature(const std::vector<Certificate_Store*>& trust_roots,
                                              const std::vector<std::shared_ptr<const X509_Certificate>>& cert_path = {}) const;

      /**
      * Verify that issuer's key signed this response
      * @param issuer certificate of issuer
      * @return if signature valid OCSP_SIGNATURE_OK else an error code
      */
      Certificate_Status_Code verify_signature(const X509_Certificate& issuer) const;

      /**
      * @return the time this OCSP response was supposedly produced at
      */
      const X509_Time& produced_at() const { return m_produced_at; }

      /**
      * @return DN of signer, if provided in response (may be empty)
      */
      const X509_DN& signer_name() const { return m_signer_name; }

      /**
      * @return key hash, if provided in response (may be empty)
      */
      const std::vector<uint8_t>& signer_key_hash() const { return m_key_hash; }

      const std::vector<uint8_t>& raw_bits() const { return m_response_bits; }

      /**
       * Searches the OCSP response for issuer and subject certificate.
       * @param issuer issuer certificate
       * @param subject subject certificate
       * @param ref_time the reference time
       * @return OCSP status code, possible values:
       *         CERT_IS_REVOKED,
       *         OCSP_NOT_YET_VALID,
       *         OCSP_HAS_EXPIRED,
       *         OCSP_RESPONSE_GOOD,
       *         OCSP_BAD_STATUS,
       *         OCSP_CERT_NOT_LISTED
       */
      Certificate_Status_Code status_for(const X509_Certificate& issuer,
                                         const X509_Certificate& subject,
                                         std::chrono::system_clock::time_point ref_time = std::chrono::system_clock::now()) const;

   private:
      std::vector<uint8_t> m_response_bits;
      X509_Time m_produced_at;
      X509_DN m_signer_name;
      std::vector<uint8_t> m_key_hash;
      std::vector<uint8_t> m_tbs_bits;
      AlgorithmIdentifier m_sig_algo;
      std::vector<uint8_t> m_signature;
      std::vector<X509_Certificate> m_certs;

      std::vector<SingleResponse> m_responses;
   };

#if defined(BOTAN_HAS_HTTP_UTIL)

/**
* Makes an online OCSP request via HTTP and returns the OCSP response.
* @param issuer issuer certificate
* @param subject subject certificate
* @param trusted_roots trusted roots for the OCSP response
* @return OCSP response
*/
BOTAN_DLL Response online_check(const X509_Certificate& issuer,
                                const X509_Certificate& subject,
                                Certificate_Store* trusted_roots);

#endif

}

}

#endif
