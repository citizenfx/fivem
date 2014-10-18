/*
* Cipher Modes
* (C) 2013 Jack Lloyd
*
* Distributed under the terms of the Botan license
*/

#ifndef BOTAN_CIPHER_MODE_H__
#define BOTAN_CIPHER_MODE_H__

#include <botan/transform.h>

namespace Botan {

/**
* Interface for cipher modes
*/
class BOTAN_DLL Cipher_Mode : public Keyed_Transform
   {
   public:
      /**
      * Returns true iff this mode provides authentication as well as
      * confidentiality.
      */
      virtual bool authenticated() const { return false; }
   };

}

#endif
