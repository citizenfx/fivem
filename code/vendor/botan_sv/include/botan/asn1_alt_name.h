/*
* (C) 1999-2007 Jack Lloyd
*     2007 Yves Jerschow
*
* Botan is released under the Simplified BSD License (see license.txt)
*/

#ifndef BOTAN_X509_ALT_NAME_H_
#define BOTAN_X509_ALT_NAME_H_

#include <botan/asn1_obj.h>
#include <botan/asn1_str.h>
#include <botan/asn1_oid.h>
#include <map>

namespace Botan {

/**
* Alternative Name
*/
class BOTAN_PUBLIC_API(2,0) AlternativeName final : public ASN1_Object
   {
   public:
      void encode_into(class DER_Encoder&) const override;
      void decode_from(class BER_Decoder&) override;

      std::multimap<std::string, std::string> contents() const;

      bool has_field(const std::string& attr) const;
      std::vector<std::string> get_attribute(const std::string& attr) const;

      std::string get_first_attribute(const std::string& attr) const;

      void add_attribute(const std::string& type, const std::string& value);
      void add_othername(const OID& oid, const std::string& value, ASN1_Tag type);

      const std::multimap<std::string, std::string>& get_attributes() const
         {
         return m_alt_info;
         }

      const std::multimap<OID, ASN1_String>& get_othernames() const
         {
         return m_othernames;
         }

      bool has_items() const;

      AlternativeName(const std::string& email_addr = "",
                      const std::string& uri = "",
                      const std::string& dns = "",
                      const std::string& ip_address = "");
   private:
      std::multimap<std::string, std::string> m_alt_info;
      std::multimap<OID, ASN1_String> m_othernames;
   };

}

#endif
