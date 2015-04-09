/*
* DER Encoder
* (C) 1999-2007 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/

#ifndef BOTAN_DER_ENCODER_H__
#define BOTAN_DER_ENCODER_H__

#include <botan/asn1_obj.h>
#include <vector>

namespace Botan {

class BigInt;
class ASN1_Object;

/**
* General DER Encoding Object
*/
class BOTAN_DLL DER_Encoder
   {
   public:
      secure_vector<byte> get_contents();

      std::vector<byte> get_contents_unlocked()
         { return unlock(get_contents()); }

      DER_Encoder& start_cons(ASN1_Tag type_tag,
                              ASN1_Tag class_tag = UNIVERSAL);
      DER_Encoder& end_cons();

      DER_Encoder& start_explicit(u16bit type_tag);
      DER_Encoder& end_explicit();

      DER_Encoder& raw_bytes(const byte val[], size_t len);
      DER_Encoder& raw_bytes(const secure_vector<byte>& val);
      DER_Encoder& raw_bytes(const std::vector<byte>& val);

      DER_Encoder& encode_null();
      DER_Encoder& encode(bool b);
      DER_Encoder& encode(size_t s);
      DER_Encoder& encode(const BigInt& n);
      DER_Encoder& encode(const secure_vector<byte>& v, ASN1_Tag real_type);
      DER_Encoder& encode(const std::vector<byte>& v, ASN1_Tag real_type);
      DER_Encoder& encode(const byte val[], size_t len, ASN1_Tag real_type);

      DER_Encoder& encode(bool b,
                          ASN1_Tag type_tag,
                          ASN1_Tag class_tag = CONTEXT_SPECIFIC);

      DER_Encoder& encode(size_t s,
                          ASN1_Tag type_tag,
                          ASN1_Tag class_tag = CONTEXT_SPECIFIC);

      DER_Encoder& encode(const BigInt& n,
                          ASN1_Tag type_tag,
                          ASN1_Tag class_tag = CONTEXT_SPECIFIC);

      DER_Encoder& encode(const std::vector<byte>& v,
                          ASN1_Tag real_type,
                          ASN1_Tag type_tag,
                          ASN1_Tag class_tag = CONTEXT_SPECIFIC);

      DER_Encoder& encode(const secure_vector<byte>& v,
                          ASN1_Tag real_type,
                          ASN1_Tag type_tag,
                          ASN1_Tag class_tag = CONTEXT_SPECIFIC);

      DER_Encoder& encode(const byte v[], size_t len,
                          ASN1_Tag real_type,
                          ASN1_Tag type_tag,
                          ASN1_Tag class_tag = CONTEXT_SPECIFIC);

      template<typename T>
      DER_Encoder& encode_optional(const T& value, const T& default_value)
         {
         if(value != default_value)
            encode(value);
         return (*this);
         }

      template<typename T>
      DER_Encoder& encode_list(const std::vector<T>& values)
         {
         for(size_t i = 0; i != values.size(); ++i)
            encode(values[i]);
         return (*this);
         }

      DER_Encoder& encode(const ASN1_Object& obj);
      DER_Encoder& encode_if(bool pred, DER_Encoder& enc);
      DER_Encoder& encode_if(bool pred, const ASN1_Object& obj);

      DER_Encoder& add_object(ASN1_Tag type_tag, ASN1_Tag class_tag,
                              const byte rep[], size_t length);

      DER_Encoder& add_object(ASN1_Tag type_tag, ASN1_Tag class_tag,
                              const std::vector<byte>& rep)
         {
         return add_object(type_tag, class_tag, &rep[0], rep.size());
         }

      DER_Encoder& add_object(ASN1_Tag type_tag, ASN1_Tag class_tag,
                              const secure_vector<byte>& rep)
         {
         return add_object(type_tag, class_tag, &rep[0], rep.size());
         }

      DER_Encoder& add_object(ASN1_Tag type_tag, ASN1_Tag class_tag,
                              const std::string& str);

      DER_Encoder& add_object(ASN1_Tag type_tag, ASN1_Tag class_tag,
                              byte val);

   private:
      class DER_Sequence
         {
         public:
            ASN1_Tag tag_of() const;
            secure_vector<byte> get_contents();
            void add_bytes(const byte[], size_t);
            DER_Sequence(ASN1_Tag, ASN1_Tag);
         private:
            ASN1_Tag type_tag, class_tag;
            secure_vector<byte> contents;
            std::vector< secure_vector<byte> > set_contents;
         };

      secure_vector<byte> contents;
      std::vector<DER_Sequence> subsequences;
   };

}

#endif
