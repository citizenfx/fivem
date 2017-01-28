/*
* ASN.1 Time Representation
* (C) 1999-2007,2012 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/

#ifndef BOTAN_ASN1_TIME_H__
#define BOTAN_ASN1_TIME_H__

#include <botan/asn1_obj.h>
#include <chrono>

namespace Botan {

/**
* X.509 Time
*/
class BOTAN_DLL X509_Time final : public ASN1_Object
   {
   public:
      /// DER encode a X509_Time
      void encode_into(DER_Encoder&) const override;

      // Decode a BER encoded X509_Time
      void decode_from(BER_Decoder&) override;

      /// Return an internal string representation of the time
      std::string to_string() const;

      /// Returns a human friendly string replesentation of no particular formatting
      std::string readable_string() const;

      /// Return if the time has been set somehow
      bool time_is_set() const;

      ///  Compare this time against another
      int32_t cmp(const X509_Time& other) const;

      /// Create an invalid X509_Time
      X509_Time() {}

      /// Create a X509_Time from a time point
      explicit X509_Time(const std::chrono::system_clock::time_point& time);

      /// Create an X509_Time from string
      X509_Time(const std::string& t_spec, ASN1_Tag tag);

      /// Returns a STL timepoint object
      std::chrono::system_clock::time_point to_std_timepoint() const;

   private:
      void set_to(const std::string& t_spec, ASN1_Tag);
      bool passes_sanity_check() const;

      uint32_t m_year = 0;
      uint32_t m_month = 0;
      uint32_t m_day = 0;
      uint32_t m_hour = 0;
      uint32_t m_minute = 0;
      uint32_t m_second = 0;
      ASN1_Tag m_tag = NO_OBJECT;
   };

/*
* Comparison Operations
*/
bool BOTAN_DLL operator==(const X509_Time&, const X509_Time&);
bool BOTAN_DLL operator!=(const X509_Time&, const X509_Time&);
bool BOTAN_DLL operator<=(const X509_Time&, const X509_Time&);
bool BOTAN_DLL operator>=(const X509_Time&, const X509_Time&);
bool BOTAN_DLL operator<(const X509_Time&, const X509_Time&);
bool BOTAN_DLL operator>(const X509_Time&, const X509_Time&);

}

#endif
