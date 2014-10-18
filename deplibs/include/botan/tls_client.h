/*
* TLS Client
* (C) 2004-2011 Jack Lloyd
*
* Released under the terms of the Botan license
*/

#ifndef BOTAN_TLS_CLIENT_H__
#define BOTAN_TLS_CLIENT_H__

#include <botan/tls_channel.h>
#include <botan/credentials_manager.h>
#include <vector>

namespace Botan {

namespace TLS {

/**
* SSL/TLS Client
*/
class BOTAN_DLL Client : public Channel
   {
   public:
      /**
      * Set up a new TLS client session
      *
      * @param socket_output_fn is called with data for the outbound socket
      *
      * @param proc_cb is called when new application data is received
      *
      * @param alert_cb is called when a TLS alert is received
      *
      * @param handshake_cb is called when a handshake is completed
      *
      * @param session_manager manages session state
      *
      * @param creds manages application/user credentials
      *
      * @param policy specifies other connection policy information
      *
      * @param rng a random number generator
      *
      * @param server_info is identifying information about the TLS server
      *
      * @param offer_version specifies which version we will offer
      *        to the TLS server.
      *
      * @param next_protocol allows the client to specify what the next
      *        protocol will be. For more information read
      *        http://technotes.googlecode.com/git/nextprotoneg.html.
      *
      *        If the function is not empty, NPN will be negotiated
      *        and if the server supports NPN the function will be
      *        called with the list of protocols the server advertised;
      *        the client should return the protocol it would like to use.
      *
      * @param reserved_io_buffer_size This many bytes of memory will
      *        be preallocated for the read and write buffers. Smaller
      *        values just mean reallocations and copies are more likely.
      */
      Client(std::function<void (const byte[], size_t)> socket_output_fn,
             std::function<void (const byte[], size_t)> data_cb,
             std::function<void (Alert, const byte[], size_t)> alert_cb,
             std::function<bool (const Session&)> handshake_cb,
             Session_Manager& session_manager,
             Credentials_Manager& creds,
             const Policy& policy,
             RandomNumberGenerator& rng,
             const Server_Information& server_info = Server_Information(),
             const Protocol_Version offer_version = Protocol_Version::latest_tls_version(),
             std::function<std::string (std::vector<std::string>)> next_protocol =
                std::function<std::string (std::vector<std::string>)>(),
             size_t reserved_io_buffer_size = 16*1024
         );
   private:
      std::vector<X509_Certificate>
         get_peer_cert_chain(const Handshake_State& state) const override;

      void initiate_handshake(Handshake_State& state,
                              bool force_full_renegotiation) override;

      void send_client_hello(Handshake_State& state,
                             bool force_full_renegotiation,
                             Protocol_Version version,
                             const std::string& srp_identifier = "",
                             std::function<std::string (std::vector<std::string>)> next_protocol =
                                std::function<std::string (std::vector<std::string>)>());

      void process_handshake_msg(const Handshake_State* active_state,
                                 Handshake_State& pending_state,
                                 Handshake_Type type,
                                 const std::vector<byte>& contents) override;

      Handshake_State* new_handshake_state(Handshake_IO* io) override;

      const Policy& m_policy;
      Credentials_Manager& m_creds;
      const Server_Information m_info;
   };

}

}

#endif
