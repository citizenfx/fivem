/*
* Symmetric Key Length Specification
* (C) 2010 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/

#ifndef BOTAN_KEY_LEN_SPECIFICATION_H__
#define BOTAN_KEY_LEN_SPECIFICATION_H__

#include <botan/types.h>

namespace Botan {

/**
* Represents the length requirements on an algorithm key
*/
class BOTAN_DLL Key_Length_Specification
   {
   public:
      /**
      * Constructor for fixed length keys
      * @param keylen the supported key length
      */
      Key_Length_Specification(size_t keylen) :
         min_keylen(keylen),
         max_keylen(keylen),
         keylen_mod(1)
         {
         }

      /**
      * Constructor for variable length keys
      * @param min_k the smallest supported key length
      * @param max_k the largest supported key length
      * @param k_mod the number of bytes the key must be a multiple of
      */
      Key_Length_Specification(size_t min_k,
                               size_t max_k,
                               size_t k_mod = 1) :
         min_keylen(min_k),
         max_keylen(max_k ? max_k : min_k),
         keylen_mod(k_mod)
         {
         }

      /**
      * @param length is a key length in bytes
      * @return true iff this length is a valid length for this algo
      */
      bool valid_keylength(size_t length) const
         {
         return ((length >= min_keylen) &&
                 (length <= max_keylen) &&
                 (length % keylen_mod == 0));
         }

      /**
      * @return minimum key length in bytes
      */
      size_t minimum_keylength() const
         {
         return min_keylen;
         }

      /**
      * @return maximum key length in bytes
      */
      size_t maximum_keylength() const
         {
         return max_keylen;
         }

      /**
      * @return key length multiple in bytes
      */
      size_t keylength_multiple() const
         {
         return keylen_mod;
         }

      Key_Length_Specification multiple(size_t n) const
         {
         return Key_Length_Specification(n * min_keylen,
                                         n * max_keylen,
                                         n * keylen_mod);
         }

   private:
      size_t min_keylen, max_keylen, keylen_mod;
   };

}

#endif
