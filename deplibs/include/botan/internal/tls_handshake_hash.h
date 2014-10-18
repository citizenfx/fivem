/*
* TLS Handshake Hash
* (C) 2004-2006,2011,2012 Jack Lloyd
*
* Released under the terms of the Botan license
*/

#ifndef BOTAN_TLS_HANDSHAKE_HASH_H__
#define BOTAN_TLS_HANDSHAKE_HASH_H__

#include <botan/secmem.h>
#include <botan/tls_version.h>
#include <botan/tls_magic.h>

namespace Botan {

namespace TLS {

using namespace Botan;

/**
* TLS Handshake Hash
*/
class Handshake_Hash
   {
   public:
      void update(const byte in[], size_t length)
         { data += std::make_pair(in, length); }

      void update(const std::vector<byte>& in)
         { data += in; }

      secure_vector<byte> final(Protocol_Version version,
                                const std::string& mac_algo) const;

      secure_vector<byte> final_ssl3(const secure_vector<byte>& master_secret) const;

      const std::vector<byte>& get_contents() const
         { return data; }

      void reset() { data.clear(); }
   private:
      std::vector<byte> data;
   };

}

}

#endif
