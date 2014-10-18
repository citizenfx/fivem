/*
* TLS Handshake Message
* (C) 2012 Jack Lloyd
*
* Released under the terms of the Botan license
*/

#ifndef BOTAN_TLS_HANDSHAKE_MSG_H__
#define BOTAN_TLS_HANDSHAKE_MSG_H__

#include <botan/tls_magic.h>
#include <vector>
#include <string>

namespace Botan {

namespace TLS {

/**
* TLS Handshake Message Base Class
*/
class BOTAN_DLL Handshake_Message
   {
   public:
      virtual Handshake_Type type() const = 0;

      virtual std::vector<byte> serialize() const = 0;

      virtual ~Handshake_Message() {}
   };

}

}

#endif
