/*
* HKDF
* (C) 2013 Jack Lloyd
*
* Distributed under the terms of the Botan license
*/

#ifndef BOTAN_HKDF_H__
#define BOTAN_HKDF_H__

#include <botan/mac.h>
#include <botan/hash.h>

namespace Botan {

/**
* HKDF, see @rfc 5869 for details
*/
class BOTAN_DLL HKDF
   {
   public:
      HKDF(MessageAuthenticationCode* extractor,
           MessageAuthenticationCode* prf) :
         m_extractor(extractor), m_prf(prf) {}

      HKDF(MessageAuthenticationCode* prf) :
         m_extractor(prf), m_prf(m_extractor->clone()) {}

      void start_extract(const byte salt[], size_t salt_len);
      void extract(const byte input[], size_t input_len);
      void finish_extract();

      /**
      * Only call after extract
      * @param output_len must be less than 256*hashlen
      */
      void expand(byte output[], size_t output_len,
                  const byte info[], size_t info_len);

      std::string name() const;

      void clear();
   private:
      std::unique_ptr<MessageAuthenticationCode> m_extractor;
      std::unique_ptr<MessageAuthenticationCode> m_prf;
   };

}

#endif
