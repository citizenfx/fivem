/*
* CRL Entry
* (C) 1999-2007 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/

#ifndef BOTAN_CRL_ENTRY_H_
#define BOTAN_CRL_ENTRY_H_

#include <botan/asn1_time.h>

namespace Botan {

class Extensions;
class X509_Certificate;
struct CRL_Entry_Data;

/**
* X.509v2 CRL Reason Code.
*/
enum CRL_Code : uint32_t {
   UNSPECIFIED            = 0,
   KEY_COMPROMISE         = 1,
   CA_COMPROMISE          = 2,
   AFFILIATION_CHANGED    = 3,
   SUPERSEDED             = 4,
   CESSATION_OF_OPERATION = 5,
   CERTIFICATE_HOLD       = 6,
   REMOVE_FROM_CRL        = 8,
   PRIVLEDGE_WITHDRAWN    = 9,
   PRIVILEGE_WITHDRAWN    = 9,
   AA_COMPROMISE          = 10,

   DELETE_CRL_ENTRY       = 0xFF00,
   OCSP_GOOD              = 0xFF01,
   OCSP_UNKNOWN           = 0xFF02
};

/**
* This class represents CRL entries
*/
class BOTAN_PUBLIC_API(2,0) CRL_Entry final : public ASN1_Object
   {
   public:
      void encode_into(class DER_Encoder&) const override;
      void decode_from(class BER_Decoder&) override;

      /**
      * Get the serial number of the certificate associated with this entry.
      * @return certificate's serial number
      */
      const std::vector<uint8_t>& serial_number() const;

      /**
      * Get the revocation date of the certificate associated with this entry
      * @return certificate's revocation date
      */
      const X509_Time& expire_time() const;

      /**
      * Get the entries reason code
      * @return reason code
      */
      CRL_Code reason_code() const;

      /**
      * Get the extensions on this CRL entry
      */
      const Extensions& extensions() const;

      /**
      * Create uninitialized CRL_Entry object
      */
      CRL_Entry() = default;

      /**
      * Construct an CRL entry.
      * @param cert the certificate to revoke
      * @param reason the reason code to set in the entry
      */
      CRL_Entry(const X509_Certificate& cert,
                CRL_Code reason = UNSPECIFIED);

   private:
      friend class X509_CRL;

      const CRL_Entry_Data& data() const;

      std::shared_ptr<CRL_Entry_Data> m_data;
   };

/**
* Test two CRL entries for equality in all fields.
*/
BOTAN_PUBLIC_API(2,0) bool operator==(const CRL_Entry&, const CRL_Entry&);

/**
* Test two CRL entries for inequality in at least one field.
*/
BOTAN_PUBLIC_API(2,0) bool operator!=(const CRL_Entry&, const CRL_Entry&);

}

#endif
