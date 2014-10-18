/*
* TLS Server
* (C) 2004-2011 Jack Lloyd
*
* Released under the terms of the Botan license
*/

#ifndef BOTAN_TLS_SERVER_H__
#define BOTAN_TLS_SERVER_H__

#include <botan/tls_channel.h>
#include <botan/credentials_manager.h>
#include <vector>

namespace Botan {

namespace TLS {

/**
* TLS Server
*/
class BOTAN_DLL Server : public Channel
   {
   public:
      /**
      * Server initialization
      */
      Server(std::function<void (const byte[], size_t)> socket_output_fn,
             std::function<void (const byte[], size_t)> data_cb,
             std::function<void (Alert, const byte[], size_t)> alert_cb,
             std::function<bool (const Session&)> handshake_cb,
             Session_Manager& session_manager,
             Credentials_Manager& creds,
             const Policy& policy,
             RandomNumberGenerator& rng,
             const std::vector<std::string>& protocols = std::vector<std::string>(),
             size_t reserved_io_buffer_size = 16*1024
         );

      /**
      * Return the protocol notification set by the client (using the
      * NPN extension) for this connection, if any
      */
      std::string next_protocol() const { return m_next_protocol; }

   private:
      std::vector<X509_Certificate>
         get_peer_cert_chain(const Handshake_State& state) const override;

      void initiate_handshake(Handshake_State& state,
                              bool force_full_renegotiation) override;

      void process_handshake_msg(const Handshake_State* active_state,
                                 Handshake_State& pending_state,
                                 Handshake_Type type,
                                 const std::vector<byte>& contents) override;

      Handshake_State* new_handshake_state(Handshake_IO* io) override;

      const Policy& m_policy;
      Credentials_Manager& m_creds;

      std::vector<std::string> m_possible_protocols;
      std::string m_next_protocol;
   };

}

}

#endif
