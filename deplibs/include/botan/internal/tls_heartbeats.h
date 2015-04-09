/*
* TLS Heartbeats
* (C) 2012,2015 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/

#ifndef BOTAN_TLS_HEARTBEATS_H__
#define BOTAN_TLS_HEARTBEATS_H__

#include <botan/secmem.h>

namespace Botan {

namespace TLS {

/**
* TLS Heartbeat message
*/
class Heartbeat_Message
   {
   public:
      enum Type { REQUEST = 1, RESPONSE = 2 };

      std::vector<byte> contents() const;

      const std::vector<byte>& payload() const { return m_payload; }

      bool is_request() const { return m_type == REQUEST; }

      Heartbeat_Message(const std::vector<byte>& buf);

      Heartbeat_Message(Type type, const byte payload[], size_t payload_len,
                        const std::vector<byte>& padding);
   private:
      Type m_type;
      std::vector<byte> m_payload, m_padding;
   };

}

}

#endif
