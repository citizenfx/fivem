/*
* TLS Server
* (C) 2004-2011 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
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
      typedef std::function<std::string (std::vector<std::string>)> next_protocol_fn;

      /**
      * Server initialization
      */
      Server(output_fn output,
             data_cb data_cb,
             alert_cb alert_cb,
             handshake_cb handshake_cb,
             Session_Manager& session_manager,
             Credentials_Manager& creds,
             const Policy& policy,
             RandomNumberGenerator& rng,
             next_protocol_fn next_proto = next_protocol_fn(),
             bool is_datagram = false,
             size_t reserved_io_buffer_size = 16*1024
         );

      /**
      * Return the protocol notification set by the client (using the
      * NPN extension) for this connection, if any. This value is not
      * tied to the session and a later renegotiation of the same
      * session can choose a new protocol.
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

      next_protocol_fn m_choose_next_protocol;
      std::string m_next_protocol;
   };

}

}

#endif
