/*
* Cipher Modes
* (C) 2013 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/

#ifndef BOTAN_CIPHER_MODE_H__
#define BOTAN_CIPHER_MODE_H__

#include <botan/transform.h>
#include <botan/stream_cipher.h>

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

      /**
      * Return the size of the authentication tag used (in bytes)
      */
      virtual size_t tag_size() const { return 0; }
   };

/**
* The two possible directions for cipher filters, determining whether they
* actually perform encryption or decryption.
*/
enum Cipher_Dir { ENCRYPTION, DECRYPTION };

BOTAN_DLL Cipher_Mode* get_cipher_mode(const std::string& algo_spec, Cipher_Dir direction);

}

#endif
