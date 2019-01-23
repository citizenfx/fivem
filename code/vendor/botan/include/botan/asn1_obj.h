/*
* ASN.1 Internals
* (C) 1999-2007,2018 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/

#ifndef BOTAN_ASN1_H_
#define BOTAN_ASN1_H_

#include <botan/secmem.h>
#include <botan/exceptn.h>

namespace Botan {

class BER_Decoder;
class DER_Encoder;

/**
* ASN.1 Type and Class Tags
*/
enum ASN1_Tag : uint32_t {
   UNIVERSAL        = 0x00,
   APPLICATION      = 0x40,
   CONTEXT_SPECIFIC = 0x80,

   CONSTRUCTED      = 0x20,

   PRIVATE          = CONSTRUCTED | CONTEXT_SPECIFIC,

   EOC              = 0x00,
   BOOLEAN          = 0x01,
   INTEGER          = 0x02,
   BIT_STRING       = 0x03,
   OCTET_STRING     = 0x04,
   NULL_TAG         = 0x05,
   OBJECT_ID        = 0x06,
   ENUMERATED       = 0x0A,
   SEQUENCE         = 0x10,
   SET              = 0x11,

   UTF8_STRING      = 0x0C,
   NUMERIC_STRING   = 0x12,
   PRINTABLE_STRING = 0x13,
   T61_STRING       = 0x14,
   IA5_STRING       = 0x16,
   VISIBLE_STRING   = 0x1A,
   UNIVERSAL_STRING = 0x1C,
   BMP_STRING       = 0x1E,

   UTC_TIME                = 0x17,
   GENERALIZED_TIME        = 0x18,
   UTC_OR_GENERALIZED_TIME = 0x19,

   NO_OBJECT        = 0xFF00,
   DIRECTORY_STRING = 0xFF01
};

std::string BOTAN_UNSTABLE_API asn1_tag_to_string(ASN1_Tag type);
std::string BOTAN_UNSTABLE_API asn1_class_to_string(ASN1_Tag type);

/**
* Basic ASN.1 Object Interface
*/
class BOTAN_PUBLIC_API(2,0) ASN1_Object
   {
   public:
      /**
      * Encode whatever this object is into to
      * @param to the DER_Encoder that will be written to
      */
      virtual void encode_into(DER_Encoder& to) const = 0;

      /**
      * Decode whatever this object is from from
      * @param from the BER_Decoder that will be read from
      */
      virtual void decode_from(BER_Decoder& from) = 0;

      /**
      * Return the encoding of this object. This is a convenience
      * method when just one object needs to be serialized. Use
      * DER_Encoder for complicated encodings.
      */
      std::vector<uint8_t> BER_encode() const;

      ASN1_Object() = default;
      ASN1_Object(const ASN1_Object&) = default;
      ASN1_Object & operator=(const ASN1_Object&) = default;
      virtual ~ASN1_Object() = default;
   };

/**
* BER Encoded Object
*/
class BOTAN_PUBLIC_API(2,0) BER_Object final
   {
   public:
      BER_Object() : type_tag(NO_OBJECT), class_tag(UNIVERSAL) {}

      BER_Object(const BER_Object& other) = default;

      BER_Object& operator=(const BER_Object& other) = default;

      BER_Object(BER_Object&& other) = default;

      BER_Object& operator=(BER_Object&& other) = default;

      bool is_set() const { return type_tag != NO_OBJECT; }

      ASN1_Tag tagging() const { return ASN1_Tag(type() | get_class()); }

      ASN1_Tag type() const { return type_tag; }
      ASN1_Tag get_class() const { return class_tag; }

      const uint8_t* bits() const { return value.data(); }

      size_t length() const { return value.size(); }

      void assert_is_a(ASN1_Tag type_tag, ASN1_Tag class_tag,
                       const std::string& descr = "object") const;

      bool is_a(ASN1_Tag type_tag, ASN1_Tag class_tag) const;

      bool is_a(int type_tag, ASN1_Tag class_tag) const;

   BOTAN_DEPRECATED_PUBLIC_MEMBER_VARIABLES:
      /*
      * The following member variables are public for historical reasons, but
      * will be made private in a future major release. Use the accessor
      * functions above.
      */
      ASN1_Tag type_tag, class_tag;
      secure_vector<uint8_t> value;

   private:

      friend class BER_Decoder;

      void set_tagging(ASN1_Tag type_tag, ASN1_Tag class_tag);

      uint8_t* mutable_bits(size_t length)
         {
         value.resize(length);
         return value.data();
         }
   };

/*
* ASN.1 Utility Functions
*/
class DataSource;

namespace ASN1 {

std::vector<uint8_t> put_in_sequence(const std::vector<uint8_t>& val);
std::vector<uint8_t> put_in_sequence(const uint8_t bits[], size_t len);
std::string to_string(const BER_Object& obj);

/**
* Heuristics tests; is this object possibly BER?
* @param src a data source that will be peeked at but not modified
*/
bool maybe_BER(DataSource& src);

}

/**
* General BER Decoding Error Exception
*/
class BOTAN_PUBLIC_API(2,0) BER_Decoding_Error : public Decoding_Error
   {
   public:
      explicit BER_Decoding_Error(const std::string&);
   };

/**
* Exception For Incorrect BER Taggings
*/
class BOTAN_PUBLIC_API(2,0) BER_Bad_Tag final : public BER_Decoding_Error
   {
   public:
      BER_Bad_Tag(const std::string& msg, ASN1_Tag tag);
      BER_Bad_Tag(const std::string& msg, ASN1_Tag tag1, ASN1_Tag tag2);
   };

}

#endif
