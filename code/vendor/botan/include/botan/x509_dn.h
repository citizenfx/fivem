/*
* X.509 Distinguished Name
* (C) 1999-2010 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/

#ifndef BOTAN_X509_DN_H__
#define BOTAN_X509_DN_H__

#include <botan/asn1_obj.h>
#include <botan/asn1_oid.h>
#include <botan/asn1_str.h>
#include <map>
#include <iosfwd>

namespace Botan {

/**
* Distinguished Name
*/
class BOTAN_DLL X509_DN final : public ASN1_Object
   {
   public:
      void encode_into(class DER_Encoder&) const override;
      void decode_from(class BER_Decoder&) override;

      std::multimap<OID, std::string> get_attributes() const;
      std::vector<std::string> get_attribute(const std::string&) const;

      std::multimap<std::string, std::string> contents() const;

      void add_attribute(const std::string&, const std::string&);
      void add_attribute(const OID&, const std::string&);

      static std::string deref_info_field(const std::string&);

      std::vector<uint8_t> get_bits() const;

      bool empty() const { return m_dn_info.empty(); }

      X509_DN();
      explicit X509_DN(const std::multimap<OID, std::string>&);
      explicit X509_DN(const std::multimap<std::string, std::string>&);
   private:
      std::multimap<OID, ASN1_String> m_dn_info;
      std::vector<uint8_t> m_dn_bits;
   };

bool BOTAN_DLL operator==(const X509_DN&, const X509_DN&);
bool BOTAN_DLL operator!=(const X509_DN&, const X509_DN&);
bool BOTAN_DLL operator<(const X509_DN&, const X509_DN&);

BOTAN_DLL std::ostream& operator<<(std::ostream& out, const X509_DN& dn);
BOTAN_DLL std::istream& operator>>(std::istream& in, X509_DN& dn);

}

#endif
