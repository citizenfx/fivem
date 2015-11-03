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
class Win32_CAPI_EntropySource : public EntropySource
   {
   public:
      std::string name() const override { return "Win32 CryptoGenRandom"; }

      void poll(Entropy_Accumulator& accum) override;

     /**
     * Win32_Capi_Entropysource Constructor
     * @param provs list of providers, separated by ':'
     */
      Win32_CAPI_EntropySource(const std::string& provs = "");
   private:
      std::vector<u64bit> prov_types;
      secure_vector<byte> m_buf;
   };

}

#endif
