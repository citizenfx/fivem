/*
* CRC32
* (C) 1999-2007 Jack Lloyd
*
* Distributed under the terms of the Botan license
*/

#ifndef BOTAN_CRC32_H__
#define BOTAN_CRC32_H__

#include <botan/hash.h>

namespace Botan {

/**
* 32-bit cyclic redundancy check
*/
class BOTAN_DLL CRC32 : public HashFunction
   {
   public:
      std::string name() const { return "CRC32"; }
      size_t output_length() const { return 4; }
      HashFunction* clone() const { return new CRC32; }

      void clear() { crc = 0xFFFFFFFF; }

      CRC32() { clear(); }
      ~CRC32() { clear(); }
   private:
      void add_data(const byte[], size_t);
      void final_result(byte[]);
      u32bit crc;
   };

}

#endif
