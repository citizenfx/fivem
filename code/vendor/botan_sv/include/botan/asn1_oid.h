/*
* ASN.1 OID
* (C) 1999-2007,2019 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/

#ifndef BOTAN_ASN1_OID_H_
#define BOTAN_ASN1_OID_H_

#include <botan/asn1_obj.h>
#include <string>
#include <vector>

namespace Botan {

/**
* This class represents ASN.1 object identifiers.
*/
class BOTAN_PUBLIC_API(2,0) OID final : public ASN1_Object
   {
   public:

      /**
      * Create an uninitialied OID object
      */
      explicit OID() {}

      /**
      * Construct an OID from a string.
      * @param str a string in the form "a.b.c" etc., where a,b,c are numbers
      */
      explicit OID(const std::string& str);

      /**
      * Initialize an OID from a sequence of integer values
      */
      explicit OID(std::initializer_list<uint32_t> init) : m_id(init) {}

      /**
      * Initialize an OID from a vector of integer values
      */
      explicit OID(std::vector<uint32_t>&& init) : m_id(init) {}

      /**
      * Construct an OID from a string.
      * @param str a string in the form "a.b.c" etc., where a,b,c are numbers
      *        or any known OID name (for example "RSA" or "X509v3.SubjectKeyIdentifier")
      */
      static OID from_string(const std::string& str);

      void encode_into(class DER_Encoder&) const override;
      void decode_from(class BER_Decoder&) override;

      /**
      * Find out whether this OID is empty
      * @return true is no OID value is set
      */
      bool empty() const { return m_id.empty(); }

      /**
      * Find out whether this OID has a value
      * @return true is this OID has a value
      */
      bool has_value() const { return (m_id.empty() == false); }

      /**
      * Get this OID as list (vector) of its components.
      * @return vector representing this OID
      */
      const std::vector<uint32_t>& get_components() const { return m_id; }

      const std::vector<uint32_t>& get_id() const { return get_components(); }

      /**
      * Get this OID as a string
      * @return string representing this OID
      */
      std::string BOTAN_DEPRECATED("Use OID::to_string") as_string() const
         {
         return this->to_string();
         }

      /**
      * Get this OID as a dotted-decimal string
      * @return string representing this OID
      */
      std::string to_string() const;

      /**
      * If there is a known name associated with this OID, return that.
      * Otherwise return the result of to_string
      */
      std::string to_formatted_string() const;

      /**
      * Compare two OIDs.
      * @return true if they are equal, false otherwise
      */
      bool operator==(const OID& other) const
         {
         return m_id == other.m_id;
         }

      /**
      * Reset this instance to an empty OID.
      */
      void BOTAN_DEPRECATED("Avoid mutation of OIDs") clear() { m_id.clear(); }

      /**
      * Add a component to this OID.
      * @param new_comp the new component to add to the end of this OID
      * @return reference to *this
      */
      BOTAN_DEPRECATED("Avoid mutation of OIDs") OID& operator+=(uint32_t new_comp)
         {
         m_id.push_back(new_comp);
         return (*this);
         }

   private:
      std::vector<uint32_t> m_id;
   };

/**
* Append another component onto the OID.
* @param oid the OID to add the new component to
* @param new_comp the new component to add
*/
OID BOTAN_PUBLIC_API(2,0) operator+(const OID& oid, uint32_t new_comp);

/**
* Compare two OIDs.
* @param a the first OID
* @param b the second OID
* @return true if a is not equal to b
*/
inline bool operator!=(const OID& a, const OID& b)
   {
   return !(a == b);
   }

/**
* Compare two OIDs.
* @param a the first OID
* @param b the second OID
* @return true if a is lexicographically smaller than b
*/
bool BOTAN_PUBLIC_API(2,0) operator<(const OID& a, const OID& b);

}

#endif
