/*
* OCSP
* (C) 2012 Jack Lloyd
*
* Distributed under the terms of the Botan license
*/

#ifndef BOTAN_OCSP_H__
#define BOTAN_OCSP_H__

#include <botan/cert_status.h>
#include <botan/ocsp_types.h>

namespace Botan {

class Certificate_Store;

namespace OCSP {

class BOTAN_DLL Request
   {
   public:
      Request(const X509_Certificate& issuer_cert,
              const X509_Certificate& subject_cert) :
         m_issuer(issuer_cert),
         m_subject(subject_cert)
         {}

      std::vector<byte> BER_encode() const;

      std::string base64_encode() const;

      const X509_Certificate& issuer() const { return m_issuer; }

      const X509_Certificate& subject() const { return m_subject; }
   private:
      X509_Certificate m_issuer, m_subject;
   };

class BOTAN_DLL Response
   {
   public:
      Response() {}

      Response(const Certificate_Store& trusted_roots,
               const std::vector<byte>& response);

      Certificate_Status_Code status_for(const X509_Certificate& issuer,
                                               const X509_Certificate& subject) const;

   private:
      std::vector<SingleResponse> m_responses;
   };

BOTAN_DLL Response online_check(const X509_Certificate& issuer,
                                const X509_Certificate& subject,
                                const Certificate_Store* trusted_roots);

}

}

#endif
