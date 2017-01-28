/*
* ASN.1 OID
* (C) 1999-2007 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/

#ifndef BOTAN_ASN1_OID_H__
#define BOTAN_ASN1_OID_H__

#include <botan/asn1_obj.h>
#include <string>
#include <vector>

namespace Botan {

/**
* This class represents ASN.1 object identifiers.
*/
class BOTAN_DLL OID final : public ASN1_Object
   {
   public:
      void encode_into(class DER_Encoder&) const override;
      void decode_from(class BER_Decoder&) override;

      /**
      * Find out whether this OID is empty
      * @return true is no OID value is set
      */
      bool empty() const { return m_id.size() == 0; }

      /**
      * Get this OID as list (vector) of its components.
      * @return vector representing this OID
      */
      const std::vector<uint32_t>& get_id() const { return m_id; }

      /**
      * Get this OID as a string
      * @return string representing this OID
      */
      std::string as_string() const;

      /**
      * Compare two OIDs.
      * @return true if they are equal, false otherwise
      */
      bool operator==(const OID&) const;

      /**
      * Reset this instance to an empty OID.
      */
      void clear();

      /**
      * Add a component to this OID.
      * @param new_comp the new component to add to the end of this OID
      * @return reference to *this
      */
      OID& operator+=(uint32_t new_comp);

      /**
      * Construct an OID from a string.
      * @param str a string in the form "a.b.c" etc., where a,b,c are numbers
      */
      OID(const std::string& str = "");
   private:
      std::vector<uint32_t> m_id;
   };

/**
* Append another component onto the OID.
* @param oid the OID to add the new component to
* @param new_comp the new component to add
*/
OID BOTAN_DLL operator+(const OID& oid, uint32_t new_comp);

/**
* Compare two OIDs.
* @param a the first OID
* @param b the second OID
* @return true if a is not equal to b
*/
bool BOTAN_DLL operator!=(const OID& a, const OID& b);

/**
* Compare two OIDs.
* @param a the first OID
* @param b the second OID
* @return true if a is lexicographically smaller than b
*/
bool BOTAN_DLL operator<(const OID& a, const OID& b);

}

#endif
