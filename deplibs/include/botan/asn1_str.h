/*
* ASN.1 string type
* (C) 1999-2010 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/

#ifndef BOTAN_ASN1_STRING_H__
#define BOTAN_ASN1_STRING_H__

#include <botan/asn1_obj.h>

namespace Botan {

/**
* Simple String
*/
class BOTAN_DLL ASN1_String : public ASN1_Object
   {
   public:
      void encode_into(class DER_Encoder&) const;
      void decode_from(class BER_Decoder&);

      std::string value() const;
      std::string iso_8859() const;

      ASN1_Tag tagging() const;

      ASN1_String(const std::string& = "");
      ASN1_String(const std::string&, ASN1_Tag);
   private:
      std::string iso_8859_str;
      ASN1_Tag tag;
   };

}

#endif
