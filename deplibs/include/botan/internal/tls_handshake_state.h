/*
* TLS Handshake State
* (C) 2004-2006,2011,2012 Jack Lloyd
*
* Released under the terms of the Botan license
*/

#ifndef BOTAN_TLS_HANDSHAKE_STATE_H__
#define BOTAN_TLS_HANDSHAKE_STATE_H__

#include <botan/internal/tls_handshake_hash.h>
#include <botan/internal/tls_handshake_io.h>
#include <botan/internal/tls_session_key.h>
#include <botan/tls_ciphersuite.h>
#include <botan/tls_exceptn.h>
#include <botan/tls_handshake_msg.h>
#include <botan/pk_keys.h>
#include <botan/pubkey.h>
#include <functional>

namespace Botan {

class KDF;

namespace TLS {

class Policy;

class Hello_Verify_Request;
class Client_Hello;
class Server_Hello;
class Certificate;
class Server_Key_Exchange;
class Certificate_Req;
class Server_Hello_Done;
class Certificate;
class Client_Key_Exchange;
class Certificate_Verify;
class Next_Protocol;
class New_Session_Ticket;
class Finished;

/**
* SSL/TLS Handshake State
*/
class Handshake_State
   {
   public:
      Handshake_State(Handshake_IO* io,
                      std::function<void (const Handshake_Message&)> msg_callback =
                         std::function<void (const Handshake_Message&)>());

      virtual ~Handshake_State();

      Handshake_State(const Handshake_State&) = delete;
      Handshake_State& operator=(const Handshake_State&) = delete;

      Handshake_IO& handshake_io() { return *m_handshake_io; }

      /**
      * Return true iff we have received a particular message already
      * @param msg_type the message type
      */
      bool received_handshake_msg(Handshake_Type msg_type) const;

      /**
      * Confirm that we were expecting this message type
      * @param msg_type the message type
      */
      void confirm_transition_to(Handshake_Type msg_type);

      /**
      * Record that we are expecting a particular message type next
      * @param msg_type the message type
      */
      void set_expected_next(Handshake_Type msg_type);

      std::pair<Handshake_Type, std::vector<byte>>
         get_next_handshake_msg();

      std::vector<byte> session_ticket() const;

      std::pair<std::string, Signature_Format>
         understand_sig_format(const Public_Key& key,
                               std::string hash_algo,
                               std::string sig_algo,
                               bool for_client_auth) const;

      std::pair<std::string, Signature_Format>
         choose_sig_format(const Private_Key& key,
                           std::string& hash_algo,
                           std::string& sig_algo,
                           bool for_client_auth,
                           const Policy& policy) const;

      std::string srp_identifier() const;

      KDF* protocol_specific_prf() const;

      Protocol_Version version() const { return m_version; }

      void set_version(const Protocol_Version& version);

      void hello_verify_request(const Hello_Verify_Request& hello_verify);

      void client_hello(Client_Hello* client_hello);
      void server_hello(Server_Hello* server_hello);
      void server_certs(Certificate* server_certs);
      void server_kex(Server_Key_Exchange* server_kex);
      void cert_req(Certificate_Req* cert_req);
      void server_hello_done(Server_Hello_Done* server_hello_done);
      void client_certs(Certificate* client_certs);
      void client_kex(Client_Key_Exchange* client_kex);
      void client_verify(Certificate_Verify* client_verify);
      void next_protocol(Next_Protocol* next_protocol);
      void new_session_ticket(New_Session_Ticket* new_session_ticket);
      void server_finished(Finished* server_finished);
      void client_finished(Finished* client_finished);

      const Client_Hello* client_hello() const
         { return m_client_hello.get(); }

      const Server_Hello* server_hello() const
         { return m_server_hello.get(); }

      const Certificate* server_certs() const
         { return m_server_certs.get(); }

      const Server_Key_Exchange* server_kex() const
         { return m_server_kex.get(); }

      const Certificate_Req* cert_req() const
         { return m_cert_req.get(); }

      const Server_Hello_Done* server_hello_done() const
         { return m_server_hello_done.get(); }

      const Certificate* client_certs() const
         { return m_client_certs.get(); }

      const Client_Key_Exchange* client_kex() const
         { return m_client_kex.get(); }

      const Certificate_Verify* client_verify() const
         { return m_client_verify.get(); }

      const Next_Protocol* next_protocol() const
         { return m_next_protocol.get(); }

      const New_Session_Ticket* new_session_ticket() const
         { return m_new_session_ticket.get(); }

      const Finished* server_finished() const
         { return m_server_finished.get(); }

      const Finished* client_finished() const
         { return m_client_finished.get(); }

      const Ciphersuite& ciphersuite() const { return m_ciphersuite; }

      const Session_Keys& session_keys() const { return m_session_keys; }

      void compute_session_keys();

      void compute_session_keys(const secure_vector<byte>& resume_master_secret);

      Handshake_Hash& hash() { return m_handshake_hash; }

      const Handshake_Hash& hash() const { return m_handshake_hash; }

      void note_message(const Handshake_Message& msg)
         {
         if(m_msg_callback)
            m_msg_callback(msg);
         }

   private:

      std::function<void (const Handshake_Message&)> m_msg_callback;

      std::unique_ptr<Handshake_IO> m_handshake_io;

      u32bit m_hand_expecting_mask = 0;
      u32bit m_hand_received_mask = 0;
      Protocol_Version m_version;
      Ciphersuite m_ciphersuite;
      Session_Keys m_session_keys;
      Handshake_Hash m_handshake_hash;

      std::unique_ptr<Client_Hello> m_client_hello;
      std::unique_ptr<Server_Hello> m_server_hello;
      std::unique_ptr<Certificate> m_server_certs;
      std::unique_ptr<Server_Key_Exchange> m_server_kex;
      std::unique_ptr<Certificate_Req> m_cert_req;
      std::unique_ptr<Server_Hello_Done> m_server_hello_done;
      std::unique_ptr<Certificate> m_client_certs;
      std::unique_ptr<Client_Key_Exchange> m_client_kex;
      std::unique_ptr<Certificate_Verify> m_client_verify;
      std::unique_ptr<Next_Protocol> m_next_protocol;
      std::unique_ptr<New_Session_Ticket> m_new_session_ticket;
      std::unique_ptr<Finished> m_server_finished;
      std::unique_ptr<Finished> m_client_finished;
   };

}

}

#endif
