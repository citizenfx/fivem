/*
* CRC24
* (C) 1999-2007 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/

#ifndef BOTAN_CRC24_H__
#define BOTAN_CRC24_H__

#include <botan/hash.h>

namespace Botan {

/**
* 24-bit cyclic redundancy check
*/
class BOTAN_DLL CRC24 final : public HashFunction
   {
   public:
      std::string name() const override { return "CRC24"; }
      size_t output_length() const override { return 3; }
      HashFunction* clone() const override { return new CRC24; }

      void clear() override { m_crc = 0xB704CE; }

      CRC24() { clear(); }
      ~CRC24() { clear(); }
   private:
      void add_data(const uint8_t[], size_t) override;
      void final_result(uint8_t[]) override;
      uint32_t m_crc;
   };

}

#endif
