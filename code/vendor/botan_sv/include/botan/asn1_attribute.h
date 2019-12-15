/*
* ASN.1 Attribute
* (C) 1999-2007,2012 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/

#ifndef BOTAN_ASN1_ATTRIBUTE_H_
#define BOTAN_ASN1_ATTRIBUTE_H_

#include <botan/asn1_obj.h>
#include <botan/asn1_oid.h>
#include <vector>

namespace Botan {

/**
* Attribute
*/
class BOTAN_PUBLIC_API(2,0) Attribute final : public ASN1_Object
   {
   public:
      void encode_into(class DER_Encoder& to) const override;
      void decode_from(class BER_Decoder& from) override;

      Attribute() = default;
      Attribute(const OID&, const std::vector<uint8_t>&);
      Attribute(const std::string&, const std::vector<uint8_t>&);

      const OID& get_oid() const { return oid; }

      const std::vector<uint8_t>& get_parameters() const { return parameters; }

   BOTAN_DEPRECATED_PUBLIC_MEMBER_VARIABLES:
      /*
      * These values are public for historical reasons, but in a future release
      * they will be made private. Do not access them.
      */
      OID oid;
      std::vector<uint8_t> parameters;
   };

}

#endif
