/*
* TLS Session Key
* (C) 2004-2006,2011 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/

#ifndef BOTAN_TLS_SESSION_KEYS_H_
#define BOTAN_TLS_SESSION_KEYS_H_

#include <botan/symkey.h>

namespace Botan {

namespace TLS {

class Handshake_State;

/**
* TLS Session Keys
*/
class Session_Keys final
   {
   public:
      /**
      * @return client encipherment key
      */
      const SymmetricKey& client_cipher_key() const { return m_c_cipher; }

      /**
      * @return client encipherment key
      */
      const SymmetricKey& server_cipher_key() const { return m_s_cipher; }

      /**
      * @return client MAC key
      */
      const SymmetricKey& client_mac_key() const { return m_c_mac; }

      /**
      * @return server MAC key
      */
      const SymmetricKey& server_mac_key() const { return m_s_mac; }

      /**
      * @return client IV
      */
      const InitializationVector& client_iv() const { return m_c_iv; }

      /**
      * @return server IV
      */
      const InitializationVector& server_iv() const { return m_s_iv; }

      /**
      * @return TLS master secret
      */
      const secure_vector<uint8_t>& master_secret() const { return m_master_sec; }

      Session_Keys() = default;

      /**
      * @param state state the handshake state
      * @param pre_master_secret the pre-master secret
      * @param resuming whether this TLS session is resumed
      */
      Session_Keys(const Handshake_State* state,
                   const secure_vector<uint8_t>& pre_master_secret,
                   bool resuming);

   private:
      secure_vector<uint8_t> m_master_sec;
      SymmetricKey m_c_cipher, m_s_cipher, m_c_mac, m_s_mac;
      InitializationVector m_c_iv, m_s_iv;
   };

}

}

#endif
