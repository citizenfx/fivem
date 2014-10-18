/*
* Certificate Store
* (C) 1999-2010,2013 Jack Lloyd
*
* Distributed under the terms of the Botan license
*/

#ifndef BOTAN_CERT_STORE_H__
#define BOTAN_CERT_STORE_H__

#include <botan/x509cert.h>
#include <botan/x509_crl.h>

namespace Botan {

/**
* Certificate Store Interface
*/
class BOTAN_DLL Certificate_Store
   {
   public:
      virtual ~Certificate_Store() {}

      /**
      * Subject DN and (optionally) key identifier
      */
      virtual const X509_Certificate*
         find_cert(const X509_DN& subject_dn, const std::vector<byte>& key_id) const = 0;

      virtual const X509_CRL* find_crl_for(const X509_Certificate& subject) const;

      bool certificate_known(const X509_Certificate& cert) const
         {
         return find_cert(cert.subject_dn(), cert.subject_key_id());
         }

      // remove this (used by TLS::Server)
      virtual std::vector<X509_DN> all_subjects() const = 0;
   };

/**
* In Memory Certificate Store
*/
class BOTAN_DLL Certificate_Store_In_Memory : public Certificate_Store
   {
   public:
      /**
      * Attempt to parse all files in dir (including subdirectories)
      * as certificates. Ignores errors.
      */
      Certificate_Store_In_Memory(const std::string& dir);

      Certificate_Store_In_Memory() {}

      void add_certificate(const X509_Certificate& cert);

      void add_crl(const X509_CRL& crl);

      std::vector<X509_DN> all_subjects() const override;

      const X509_Certificate* find_cert(
         const X509_DN& subject_dn,
         const std::vector<byte>& key_id) const override;

      const X509_CRL* find_crl_for(const X509_Certificate& subject) const override;
   private:
      // TODO: Add indexing on the DN and key id to avoid linear search
      std::vector<X509_Certificate> m_certs;
      std::vector<X509_CRL> m_crls;
   };

class BOTAN_DLL Certificate_Store_Overlay : public Certificate_Store
   {
   public:
      Certificate_Store_Overlay(const std::vector<X509_Certificate>& certs) :
         m_certs(certs) {}

      std::vector<X509_DN> all_subjects() const override;

      const X509_Certificate* find_cert(
         const X509_DN& subject_dn,
         const std::vector<byte>& key_id) const override;
   private:
      const std::vector<X509_Certificate>& m_certs;
   };

}

#endif
