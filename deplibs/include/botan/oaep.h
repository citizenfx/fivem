/*
* OAEP
* (C) 1999-2007 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/

#ifndef BOTAN_OAEP_H__
#define BOTAN_OAEP_H__

#include <botan/eme.h>
#include <botan/hash.h>

namespace Botan {

/**
* OAEP (called EME1 in IEEE 1363 and in earlier versions of the library)
*/
class BOTAN_DLL OAEP : public EME
   {
   public:
      size_t maximum_input_size(size_t) const override;

      static OAEP* make(const Spec& spec);

      /**
      * @param hash object to use for hashing (takes ownership)
      * @param P an optional label. Normally empty.
      */
      OAEP(HashFunction* hash, const std::string& P = "");
   private:
      secure_vector<byte> pad(const byte[], size_t, size_t,
                             RandomNumberGenerator&) const override;
      secure_vector<byte> unpad(const byte[], size_t, size_t) const override;

      secure_vector<byte> m_Phash;
      std::unique_ptr<HashFunction> m_hash;
   };

}

#endif
