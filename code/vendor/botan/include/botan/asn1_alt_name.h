/*
* Common ASN.1 Objects
* (C) 1999-2007 Jack Lloyd
*     2007 Yves Jerschow
*
* Botan is released under the Simplified BSD License (see license.txt)
*/

#ifndef BOTAN_ASN1_ALT_NAME_H__
#define BOTAN_ASN1_ALT_NAME_H__

#include <botan/asn1_obj.h>
#include <botan/asn1_str.h>
#include <botan/asn1_oid.h>
#include <map>

namespace Botan {

/**
* Alternative Name
*/
class BOTAN_DLL AlternativeName final : public ASN1_Object
   {
   public:
      void encode_into(class DER_Encoder&) const override;
      void decode_from(class BER_Decoder&) override;

      std::multimap<std::string, std::string> contents() const;

      void add_attribute(const std::string&, const std::string&);
      std::multimap<std::string, std::string> get_attributes() const;

      void add_othername(const OID&, const std::string&, ASN1_Tag);
      std::multimap<OID, ASN1_String> get_othernames() const;

      bool has_items() const;

      AlternativeName(const std::string& = "", const std::string& = "",
                      const std::string& = "", const std::string& = "");
   private:
      std::multimap<std::string, std::string> m_alt_info;
      std::multimap<OID, ASN1_String> m_othernames;
   };

}

#endif
