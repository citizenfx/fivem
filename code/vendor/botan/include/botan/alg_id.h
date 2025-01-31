/*
* Algorithm Identifier
* (C) 1999-2007 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/

#ifndef BOTAN_ALGORITHM_IDENTIFIER_H_
#define BOTAN_ALGORITHM_IDENTIFIER_H_

#include <botan/asn1_obj.h>
#include <botan/asn1_oid.h>
#include <string>
#include <vector>

namespace Botan {

/**
* Algorithm Identifier
*/
class BOTAN_PUBLIC_API(2,0) AlgorithmIdentifier final : public ASN1_Object
   {
   public:
      enum Encoding_Option { USE_NULL_PARAM, USE_EMPTY_PARAM };

      void encode_into(class DER_Encoder&) const override;
      void decode_from(class BER_Decoder&) override;

      AlgorithmIdentifier() = default;

      AlgorithmIdentifier(const OID& oid, Encoding_Option enc);
      AlgorithmIdentifier(const std::string& oid_name, Encoding_Option enc);

      AlgorithmIdentifier(const OID& oid, const std::vector<uint8_t>& params);
      AlgorithmIdentifier(const std::string& oid_name, const std::vector<uint8_t>& params);

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

/*
* Comparison Operations
*/
bool BOTAN_PUBLIC_API(2,0) operator==(const AlgorithmIdentifier&,
                                      const AlgorithmIdentifier&);
bool BOTAN_PUBLIC_API(2,0) operator!=(const AlgorithmIdentifier&,
                                      const AlgorithmIdentifier&);

}

#endif
