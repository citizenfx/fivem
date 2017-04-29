/*
* ASN.1 Attribute
* (C) 1999-2007,2012 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/

#ifndef BOTAN_ASN1_ATTRIBUTE_H__
#define BOTAN_ASN1_ATTRIBUTE_H__

#include <botan/asn1_obj.h>
#include <botan/asn1_oid.h>
#include <vector>

namespace Botan {

/**
* Attribute
*/
class BOTAN_DLL Attribute final : public ASN1_Object
   {
   public:
      void encode_into(class DER_Encoder& to) const override;
      void decode_from(class BER_Decoder& from) override;

      // public member variable:
      OID oid;

      // public member variable:
      std::vector<uint8_t> parameters;

      Attribute() {}
      Attribute(const OID&, const std::vector<uint8_t>&);
      Attribute(const std::string&, const std::vector<uint8_t>&);
   };

}

#endif
