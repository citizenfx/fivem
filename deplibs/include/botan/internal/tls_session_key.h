/*
* TLS Session Key
* (C) 2004-2006,2011 Jack Lloyd
*
* Released under the terms of the Botan license
*/

#ifndef BOTAN_TLS_SESSION_KEYS_H__
#define BOTAN_TLS_SESSION_KEYS_H__

#include <botan/symkey.h>

namespace Botan {

namespace TLS {

/**
* TLS Session Keys
*/
class Session_Keys
   {
   public:
      SymmetricKey client_cipher_key() const { return c_cipher; }
      SymmetricKey server_cipher_key() const { return s_cipher; }

      SymmetricKey client_mac_key() const { return c_mac; }
      SymmetricKey server_mac_key() const { return s_mac; }

      InitializationVector client_iv() const { return c_iv; }
      InitializationVector server_iv() const { return s_iv; }

      const secure_vector<byte>& master_secret() const { return master_sec; }

      Session_Keys() {}

      Session_Keys(const class Handshake_State* state,
                   const secure_vector<byte>& pre_master,
                   bool resuming);

   private:
      secure_vector<byte> master_sec;
      SymmetricKey c_cipher, s_cipher, c_mac, s_mac;
      InitializationVector c_iv, s_iv;
   };

}

}

#endif
