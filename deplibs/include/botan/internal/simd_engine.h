/*
* SIMD Assembly Engine
* (C) 1999-2009 Jack Lloyd
*
* Distributed under the terms of the Botan license
*/

#ifndef BOTAN_SIMD_ENGINE_H__
#define BOTAN_SIMD_ENGINE_H__

#include <botan/engine.h>

namespace Botan {

/**
* Engine for implementations that use some kind of SIMD
*/
class SIMD_Engine : public Engine
   {
   public:
      std::string provider_name() const { return "simd"; }

      BlockCipher* find_block_cipher(const SCAN_Name&,
                                     Algorithm_Factory&) const;

      HashFunction* find_hash(const SCAN_Name& request,
                              Algorithm_Factory&) const;
   };

}

#endif
