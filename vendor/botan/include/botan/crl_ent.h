/*
* CRL Entry
* (C) 1999-2007 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/

#ifndef BOTAN_CRL_ENTRY_H__
#define BOTAN_CRL_ENTRY_H__

#include <botan/asn1_time.h>

namespace Botan {

class X509_Certificate;

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
class BOTAN_DLL CRL_Entry final : public ASN1_Object
   {
   public:
      void encode_into(class DER_Encoder&) const override;
      void decode_from(class BER_Decoder&) override;

      /**
      * Get the serial number of the certificate associated with this entry.
      * @return certificate's serial number
      */
      std::vector<uint8_t> serial_number() const { return m_serial; }

      /**
      * Get the revocation date of the certificate associated with this entry
      * @return certificate's revocation date
      */
      X509_Time expire_time() const { return m_time; }

      /**
      * Get the entries reason code
      * @return reason code
      */
      CRL_Code reason_code() const { return m_reason; }

      /**
      * Construct an empty CRL entry.
      * @param throw_on_unknown_critical_extension should we throw an exception
      * if an unknown CRL extension marked as critical is encountered
      */
      explicit CRL_Entry(bool throw_on_unknown_critical_extension = false);

      /**
      * Construct an CRL entry.
      * @param cert the certificate to revoke
      * @param reason the reason code to set in the entry
      */
      CRL_Entry(const X509_Certificate& cert,
                CRL_Code reason = UNSPECIFIED);

   private:
      bool m_throw_on_unknown_critical;
      std::vector<uint8_t> m_serial;
      X509_Time m_time;
      CRL_Code m_reason;
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
