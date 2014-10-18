/*
* CRL Entry
* (C) 1999-2007 Jack Lloyd
*
* Distributed under the terms of the Botan license
*/

#ifndef BOTAN_CRL_ENTRY_H__
#define BOTAN_CRL_ENTRY_H__

#include <botan/x509cert.h>
#include <botan/asn1_time.h>

namespace Botan {

/**
* X.509v2 CRL Reason Code.
*/
enum CRL_Code {
   UNSPECIFIED            = 0,
   KEY_COMPROMISE         = 1,
   CA_COMPROMISE          = 2,
   AFFILIATION_CHANGED    = 3,
   SUPERSEDED             = 4,
   CESSATION_OF_OPERATION = 5,
   CERTIFICATE_HOLD       = 6,
   REMOVE_FROM_CRL        = 8,
   PRIVLEDGE_WITHDRAWN    = 9,
   AA_COMPROMISE          = 10,

   DELETE_CRL_ENTRY       = 0xFF00,
   OCSP_GOOD              = 0xFF01,
   OCSP_UNKNOWN           = 0xFF02
};

/**
* This class represents CRL entries
*/
class BOTAN_DLL CRL_Entry : public ASN1_Object
   {
   public:
      void encode_into(class DER_Encoder&) const;
      void decode_from(class BER_Decoder&);

      /**
      * Get the serial number of the certificate associated with this entry.
      * @return certificate's serial number
      */
      std::vector<byte> serial_number() const { return serial; }

      /**
      * Get the revocation date of the certificate associated with this entry
      * @return certificate's revocation date
      */
      X509_Time expire_time() const { return time; }

      /**
      * Get the entries reason code
      * @return reason code
      */
      CRL_Code reason_code() const { return reason; }

      /**
      * Construct an empty CRL entry.
      */
      CRL_Entry(bool throw_on_unknown_critical_extension = false);

      /**
      * Construct an CRL entry.
      * @param cert the certificate to revoke
      * @param reason the reason code to set in the entry
      */
      CRL_Entry(const X509_Certificate& cert,
                CRL_Code reason = UNSPECIFIED);

   private:
      bool throw_on_unknown_critical;
      std::vector<byte> serial;
      X509_Time time;
      CRL_Code reason;
   };

/**
* Test two CRL entries for equality in all fields.
*/
BOTAN_DLL bool operator==(const CRL_Entry&, const CRL_Entry&);

/**
* Test two CRL entries for inequality in at least one field.
*/
BOTAN_DLL bool operator!=(const CRL_Entry&, const CRL_Entry&);

}

#endif
