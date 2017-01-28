/*
* Algorithm Identifier
* (C) 1999-2007 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/

#ifndef BOTAN_ALGORITHM_IDENTIFIER_H__
#define BOTAN_ALGORITHM_IDENTIFIER_H__

#include <botan/asn1_obj.h>
#include <botan/asn1_oid.h>
#include <string>

namespace Botan {

/**
* Algorithm Identifier
*/
class BOTAN_DLL AlgorithmIdentifier final : public ASN1_Object
   {
   public:
      enum Encoding_Option { USE_NULL_PARAM };

      void encode_into(class DER_Encoder&) const override;
      void decode_from(class BER_Decoder&) override;

      AlgorithmIdentifier() {}
      AlgorithmIdentifier(const OID&, Encoding_Option);
      AlgorithmIdentifier(const std::string&, Encoding_Option);

      AlgorithmIdentifier(const OID&, const std::vector<uint8_t>&);
      AlgorithmIdentifier(const std::string&, const std::vector<uint8_t>&);

      // public member variable:
      OID oid;

      // public member variable:
      std::vector<uint8_t> parameters;
   };

/*
* Comparison Operations
*/
bool BOTAN_DLL operator==(const AlgorithmIdentifier&,
                          const AlgorithmIdentifier&);
bool BOTAN_DLL operator!=(const AlgorithmIdentifier&,
                          const AlgorithmIdentifier&);

}

#endif
