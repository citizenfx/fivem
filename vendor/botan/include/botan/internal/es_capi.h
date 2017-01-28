/*
* Win32 CAPI EntropySource
* (C) 1999-2007 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/

#ifndef BOTAN_ENTROPY_SRC_WIN32_CAPI_H__
#define BOTAN_ENTROPY_SRC_WIN32_CAPI_H__

#include <botan/entropy_src.h>
#include <vector>

namespace Botan {

/**
* Win32 CAPI Entropy Source
*/
class Win32_CAPI_EntropySource final : public Entropy_Source
   {
   public:
      std::string name() const override { return "win32_cryptoapi"; }

      size_t poll(RandomNumberGenerator& rng) override;

      /**
      * Win32_Capi_Entropysource Constructor
      * @param provs list of providers, separated by ':'
      */
      explicit Win32_CAPI_EntropySource(const std::string& provs = "");

      class CSP_Handle
         {
         public:
            virtual size_t gen_random(uint8_t out[], size_t n) const = 0;
         };
   private:
      std::vector<std::unique_ptr<CSP_Handle>> m_csp_provs;
   };

}

#endif
