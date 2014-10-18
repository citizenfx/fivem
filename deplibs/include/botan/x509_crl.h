/*
* X.509 CRL
* (C) 1999-2007 Jack Lloyd
*
* Distributed under the terms of the Botan license
*/

#ifndef BOTAN_X509_CRL_H__
#define BOTAN_X509_CRL_H__

#include <botan/x509_obj.h>
#include <botan/crl_ent.h>
#include <vector>

namespace Botan {

class X509_Certificate;

/**
* This class represents X.509 Certificate Revocation Lists (CRLs).
*/
class BOTAN_DLL X509_CRL : public X509_Object
   {
   public:
      /**
      * This class represents CRL related errors.
      */
      struct BOTAN_DLL X509_CRL_Error : public Exception
         {
         X509_CRL_Error(const std::string& error) :
            Exception("X509_CRL: " + error) {}
         };

      /**
      * Check if this particular certificate is listed in the CRL
      */
      bool is_revoked(const X509_Certificate& cert) const;

      /**
      * Get the entries of this CRL in the form of a vector.
      * @return vector containing the entries of this CRL.
      */
      std::vector<CRL_Entry> get_revoked() const;

      /**
      * Get the issuer DN of this CRL.
      * @return CRLs issuer DN
      */
      X509_DN issuer_dn() const;

      /**
      * Get the AuthorityKeyIdentifier of this CRL.
      * @return this CRLs AuthorityKeyIdentifier
      */
      std::vector<byte> authority_key_id() const;

      /**
      * Get the serial number of this CRL.
      * @return CRLs serial number
      */
      u32bit crl_number() const;

      /**
      * Get the CRL's thisUpdate value.
      * @return CRLs thisUpdate
      */
      X509_Time this_update() const;

      /**
      * Get the CRL's nextUpdate value.
      * @return CRLs nextdUpdate
      */
      X509_Time next_update() const;

      /**
      * Construct a CRL from a data source.
      * @param source the data source providing the DER or PEM encoded CRL.
      * @param throw_on_unknown_critical should we throw an exception
      * if an unknown CRL extension marked as critical is encountered.
      */
      X509_CRL(DataSource& source, bool throw_on_unknown_critical = false);

      /**
      * Construct a CRL from a file containing the DER or PEM encoded CRL.
      * @param filename the name of the CRL file
      * @param throw_on_unknown_critical should we throw an exception
      * if an unknown CRL extension marked as critical is encountered.
      */
      X509_CRL(const std::string& filename,
               bool throw_on_unknown_critical = false);

      /**
      * Construct a CRL from a binary vector
      * @param vec the binary (DER) representation of the CRL
      * @param throw_on_unknown_critical should we throw an exception
      * if an unknown CRL extension marked as critical is encountered.
      */
      X509_CRL(const std::vector<byte>& vec,
               bool throw_on_unknown_critical = false);

   private:
      void force_decode();

      bool throw_on_unknown_critical;
      std::vector<CRL_Entry> revoked;
      Data_Store info;
   };

}

#endif
