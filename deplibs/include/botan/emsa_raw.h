/*
* EMSA-Raw
* (C) 1999-2007 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/

#ifndef BOTAN_EMSA_RAW_H__
#define BOTAN_EMSA_RAW_H__

#include <botan/emsa.h>

namespace Botan {

/**
* EMSA-Raw - sign inputs directly
* Don't use this unless you know what you are doing.
*/
class BOTAN_DLL EMSA_Raw : public EMSA
   {
   private:
      void update(const byte[], size_t);
      secure_vector<byte> raw_data();

      secure_vector<byte> encoding_of(const secure_vector<byte>&, size_t,
                                     RandomNumberGenerator&);
      bool verify(const secure_vector<byte>&, const secure_vector<byte>&,
                  size_t);

      secure_vector<byte> message;
   };

}

#endif
