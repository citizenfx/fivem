/*
* TLS Messages
* (C) 2004-2011 Jack Lloyd
*
* Released under the terms of the Botan license
*/

#ifndef BOTAN_TLS_MESSAGES_H__
#define BOTAN_TLS_MESSAGES_H__

#include <botan/internal/tls_handshake_state.h>
#include <botan/internal/tls_extensions.h>
#include <botan/tls_handshake_msg.h>
#include <botan/tls_session.h>
#include <botan/tls_policy.h>
#include <botan/tls_ciphersuite.h>
#include <botan/bigint.h>
#include <botan/pkcs8.h>
#include <botan/x509cert.h>
#include <vector>
#include <string>

namespace Botan {

class Credentials_Manager;
class SRP6_Server_Session;

namespace TLS {

class Handshake_IO;

std::vector<byte> make_hello_random(RandomNumberGenerator& rng);

/**
* DTLS Hello Verify Request
*/
class Hello_Verify_Request : public Handshake_Message
   {
   public:
      std::vector<byte> serialize() const override;
      Handshake_Type type() const override { return HELLO_VERIFY_REQUEST; }

      std::vector<byte> cookie() const { return m_cookie; }

      Hello_Verify_Request(const std::vector<byte>& buf);

      Hello_Verify_Request(const std::vector<byte>& client_hello_bits,
                           const std::string& client_identity,
                           const SymmetricKey& secret_key);
   private:
      std::vector<byte> m_cookie;
   };

/**
* Client Hello Message
*/
class Client_Hello : public Handshake_Message
   {
   public:
      Handshake_Type type() const override { return CLIENT_HELLO; }

      Protocol_Version version() const { return m_version; }

      const std::vector<byte>& random() const { return m_random; }

      const std::vector<byte>& session_id() const { return m_session_id; }

      std::vector<u16bit> ciphersuites() const { return m_suites; }

      std::vector<byte> compression_methods() const { return m_comp_methods; }

      bool offered_suite(u16bit ciphersuite) const;

      std::vector<std::pair<std::string, std::string>> supported_algos() const
         {
         if(Signature_Algorithms* sigs = m_extensions.get<Signature_Algorithms>())
            return sigs->supported_signature_algorthms();
         return std::vector<std::pair<std::string, std::string>>();
         }

      std::vector<std::string> supported_ecc_curves() const
         {
         if(Supported_Elliptic_Curves* ecc = m_extensions.get<Supported_Elliptic_Curves>())
            return ecc->curves();
         return std::vector<std::string>();
         }

      std::string sni_hostname() const
         {
         if(Server_Name_Indicator* sni = m_extensions.get<Server_Name_Indicator>())
            return sni->host_name();
         return "";
         }

      std::string srp_identifier() const
         {
         if(SRP_Identifier* srp = m_extensions.get<SRP_Identifier>())
            return srp->identifier();
         return "";
         }

      bool secure_renegotiation() const
         {
         return m_extensions.get<Renegotiation_Extension>();
         }

      std::vector<byte> renegotiation_info() const
         {
         if(Renegotiation_Extension* reneg = m_extensions.get<Renegotiation_Extension>())
            return reneg->renegotiation_info();
         return std::vector<byte>();
         }

      bool next_protocol_notification() const
         {
         return m_extensions.get<Next_Protocol_Notification>();
         }

      size_t fragment_size() const
         {
         if(Maximum_Fragment_Length* frag = m_extensions.get<Maximum_Fragment_Length>())
            return frag->fragment_size();
         return 0;
         }

      bool supports_session_ticket() const
         {
         return m_extensions.get<Session_Ticket>();
         }

      std::vector<byte> session_ticket() const
         {
         if(Session_Ticket* ticket = m_extensions.get<Session_Ticket>())
            return ticket->contents();
         return std::vector<byte>();
         }

      bool supports_heartbeats() const
         {
         return m_extensions.get<Heartbeat_Support_Indicator>();
         }

      bool peer_can_send_heartbeats() const
         {
         if(Heartbeat_Support_Indicator* hb = m_extensions.get<Heartbeat_Support_Indicator>())
            return hb->peer_allowed_to_send();
         return false;
         }

      void update_hello_cookie(const Hello_Verify_Request& hello_verify);

      std::set<Handshake_Extension_Type> extension_types() const
         { return m_extensions.extension_types(); }

      Client_Hello(Handshake_IO& io,
                   Handshake_Hash& hash,
                   Protocol_Version version,
                   const Policy& policy,
                   RandomNumberGenerator& rng,
                   const std::vector<byte>& reneg_info,
                   bool next_protocol = false,
                   const std::string& hostname = "",
                   const std::string& srp_identifier = "");

      Client_Hello(Handshake_IO& io,
                   Handshake_Hash& hash,
                   const Policy& policy,
                   RandomNumberGenerator& rng,
                   const std::vector<byte>& reneg_info,
                   const Session& resumed_session,
                   bool next_protocol = false);

      Client_Hello(const std::vector<byte>& buf,
                   Handshake_Type type);

   private:
      std::vector<byte> serialize() const override;
      void deserialize(const std::vector<byte>& buf);
      void deserialize_sslv2(const std::vector<byte>& buf);

      Protocol_Version m_version;
      std::vector<byte> m_session_id;
      std::vector<byte> m_random;
      std::vector<u16bit> m_suites;
      std::vector<byte> m_comp_methods;
      std::vector<byte> m_hello_cookie; // DTLS only

      Extensions m_extensions;
   };

/**
* Server Hello Message
*/
class Server_Hello : public Handshake_Message
   {
   public:
      Handshake_Type type() const override { return SERVER_HELLO; }

      Protocol_Version version() const { return m_version; }

      const std::vector<byte>& random() const { return m_random; }

      const std::vector<byte>& session_id() const { return m_session_id; }

      u16bit ciphersuite() const { return m_ciphersuite; }

      byte compression_method() const { return m_comp_method; }

      bool secure_renegotiation() const
         {
         return m_extensions.get<Renegotiation_Extension>();
         }

      std::vector<byte> renegotiation_info() const
         {
         if(Renegotiation_Extension* reneg = m_extensions.get<Renegotiation_Extension>())
            return reneg->renegotiation_info();
         return std::vector<byte>();
         }

      bool next_protocol_notification() const
         {
         return m_extensions.get<Next_Protocol_Notification>();
         }

      std::vector<std::string> next_protocols() const
         {
         if(Next_Protocol_Notification* npn = m_extensions.get<Next_Protocol_Notification>())
            return npn->protocols();
         return std::vector<std::string>();
         }

      size_t fragment_size() const
         {
         if(Maximum_Fragment_Length* frag = m_extensions.get<Maximum_Fragment_Length>())
            return frag->fragment_size();
         return 0;
         }

      bool supports_session_ticket() const
         {
         return m_extensions.get<Session_Ticket>();
         }

      bool supports_heartbeats() const
         {
         return m_extensions.get<Heartbeat_Support_Indicator>();
         }

      bool peer_can_send_heartbeats() const
         {
         if(Heartbeat_Support_Indicator* hb = m_extensions.get<Heartbeat_Support_Indicator>())
            return hb->peer_allowed_to_send();
         return false;
         }

      std::set<Handshake_Extension_Type> extension_types() const
         { return m_extensions.extension_types(); }

      Server_Hello(Handshake_IO& io,
                   Handshake_Hash& hash,
                   const Policy& policy,
                   const std::vector<byte>& session_id,
                   Protocol_Version ver,
                   u16bit ciphersuite,
                   byte compression,
                   size_t max_fragment_size,
                   bool client_has_secure_renegotiation,
                   const std::vector<byte>& reneg_info,
                   bool offer_session_ticket,
                   bool client_has_npn,
                   const std::vector<std::string>& next_protocols,
                   bool client_has_heartbeat,
                   RandomNumberGenerator& rng);

      Server_Hello(const std::vector<byte>& buf);
   private:
      std::vector<byte> serialize() const override;

      Protocol_Version m_version;
      std::vector<byte> m_session_id, m_random;
      u16bit m_ciphersuite;
      byte m_comp_method;

      Extensions m_extensions;
   };

/**
* Client Key Exchange Message
*/
class Client_Key_Exchange : public Handshake_Message
   {
   public:
      Handshake_Type type() const override { return CLIENT_KEX; }

      const secure_vector<byte>& pre_master_secret() const
         { return m_pre_master; }

      Client_Key_Exchange(Handshake_IO& io,
                          Handshake_State& state,
                          const Policy& policy,
                          Credentials_Manager& creds,
                          const Public_Key* server_public_key,
                          const std::string& hostname,
                          RandomNumberGenerator& rng);

      Client_Key_Exchange(const std::vector<byte>& buf,
                          const Handshake_State& state,
                          const Private_Key* server_rsa_kex_key,
                          Credentials_Manager& creds,
                          const Policy& policy,
                          RandomNumberGenerator& rng);

   private:
      std::vector<byte> serialize() const override
         { return m_key_material; }

      std::vector<byte> m_key_material;
      secure_vector<byte> m_pre_master;
   };

/**
* Certificate Message
*/
class Certificate : public Handshake_Message
   {
   public:
      Handshake_Type type() const override { return CERTIFICATE; }
      const std::vector<X509_Certificate>& cert_chain() const { return m_certs; }

      size_t count() const { return m_certs.size(); }
      bool empty() const { return m_certs.empty(); }

      Certificate(Handshake_IO& io,
                  Handshake_Hash& hash,
                  const std::vector<X509_Certificate>& certs);

      Certificate(const std::vector<byte>& buf);
   private:
      std::vector<byte> serialize() const override;

      std::vector<X509_Certificate> m_certs;
   };

/**
* Certificate Request Message
*/
class Certificate_Req : public Handshake_Message
   {
   public:
      Handshake_Type type() const override { return CERTIFICATE_REQUEST; }

      const std::vector<std::string>& acceptable_cert_types() const
         { return m_cert_key_types; }

      std::vector<X509_DN> acceptable_CAs() const { return m_names; }

      std::vector<std::pair<std::string, std::string> > supported_algos() const
         { return m_supported_algos; }

      Certificate_Req(Handshake_IO& io,
                      Handshake_Hash& hash,
                      const Policy& policy,
                      const std::vector<X509_DN>& allowed_cas,
                      Protocol_Version version);

      Certificate_Req(const std::vector<byte>& buf,
                      Protocol_Version version);
   private:
      std::vector<byte> serialize() const override;

      std::vector<X509_DN> m_names;
      std::vector<std::string> m_cert_key_types;

      std::vector<std::pair<std::string, std::string> > m_supported_algos;
   };

/**
* Certificate Verify Message
*/
class Certificate_Verify : public Handshake_Message
   {
   public:
      Handshake_Type type() const override { return CERTIFICATE_VERIFY; }

      /**
      * Check the signature on a certificate verify message
      * @param cert the purported certificate
      * @param state the handshake state
      */
      bool verify(const X509_Certificate& cert,
                  const Handshake_State& state) const;

      Certificate_Verify(Handshake_IO& io,
                         Handshake_State& state,
                         const Policy& policy,
                         RandomNumberGenerator& rng,
                         const Private_Key* key);

      Certificate_Verify(const std::vector<byte>& buf,
                         Protocol_Version version);
   private:
      std::vector<byte> serialize() const override;

      std::string m_sig_algo; // sig algo used to create signature
      std::string m_hash_algo; // hash used to create signature
      std::vector<byte> m_signature;
   };

/**
* Finished Message
*/
class Finished : public Handshake_Message
   {
   public:
      Handshake_Type type() const override { return FINISHED; }

      std::vector<byte> verify_data() const
         { return m_verification_data; }

      bool verify(const Handshake_State& state,
                  Connection_Side side) const;

      Finished(Handshake_IO& io,
               Handshake_State& state,
               Connection_Side side);

      Finished(const std::vector<byte>& buf);
   private:
      std::vector<byte> serialize() const override;

      std::vector<byte> m_verification_data;
   };

/**
* Hello Request Message
*/
class Hello_Request : public Handshake_Message
   {
   public:
      Handshake_Type type() const override { return HELLO_REQUEST; }

      Hello_Request(Handshake_IO& io);
      Hello_Request(const std::vector<byte>& buf);
   private:
      std::vector<byte> serialize() const override;
   };

/**
* Server Key Exchange Message
*/
class Server_Key_Exchange : public Handshake_Message
   {
   public:
      Handshake_Type type() const override { return SERVER_KEX; }

      const std::vector<byte>& params() const { return m_params; }

      bool verify(const Public_Key& server_key,
                  const Handshake_State& state) const;

      // Only valid for certain kex types
      const Private_Key& server_kex_key() const;

      // Only valid for SRP negotiation
      SRP6_Server_Session& server_srp_params() const;

      Server_Key_Exchange(Handshake_IO& io,
                          Handshake_State& state,
                          const Policy& policy,
                          Credentials_Manager& creds,
                          RandomNumberGenerator& rng,
                          const Private_Key* signing_key = nullptr);

      Server_Key_Exchange(const std::vector<byte>& buf,
                          const std::string& kex_alg,
                          const std::string& sig_alg,
                          Protocol_Version version);

      ~Server_Key_Exchange();
   private:
      std::vector<byte> serialize() const override;

      std::unique_ptr<Private_Key> m_kex_key;
      std::unique_ptr<SRP6_Server_Session> m_srp_params;

      std::vector<byte> m_params;

      std::string m_sig_algo; // sig algo used to create signature
      std::string m_hash_algo; // hash used to create signature
      std::vector<byte> m_signature;
   };

/**
* Server Hello Done Message
*/
class Server_Hello_Done : public Handshake_Message
   {
   public:
      Handshake_Type type() const override { return SERVER_HELLO_DONE; }

      Server_Hello_Done(Handshake_IO& io, Handshake_Hash& hash);
      Server_Hello_Done(const std::vector<byte>& buf);
   private:
      std::vector<byte> serialize() const override;
   };

/**
* Next Protocol Message
*/
class Next_Protocol : public Handshake_Message
   {
   public:
      Handshake_Type type() const override { return NEXT_PROTOCOL; }

      std::string protocol() const { return m_protocol; }

      Next_Protocol(Handshake_IO& io,
                    Handshake_Hash& hash,
                    const std::string& protocol);

      Next_Protocol(const std::vector<byte>& buf);
   private:
      std::vector<byte> serialize() const override;

      std::string m_protocol;
   };

/**
* New Session Ticket Message
*/
class New_Session_Ticket : public Handshake_Message
   {
   public:
      Handshake_Type type() const override { return NEW_SESSION_TICKET; }

      u32bit ticket_lifetime_hint() const { return m_ticket_lifetime_hint; }
      const std::vector<byte>& ticket() const { return m_ticket; }

      New_Session_Ticket(Handshake_IO& io,
                         Handshake_Hash& hash,
                         const std::vector<byte>& ticket,
                         u32bit lifetime);

      New_Session_Ticket(Handshake_IO& io,
                         Handshake_Hash& hash);

      New_Session_Ticket(const std::vector<byte>& buf);
   private:
      std::vector<byte> serialize() const override;

      u32bit m_ticket_lifetime_hint = 0;
      std::vector<byte> m_ticket;
   };

/**
* Change Cipher Spec
*/
class Change_Cipher_Spec : public Handshake_Message
   {
   public:
      Handshake_Type type() const override { return HANDSHAKE_CCS; }

      std::vector<byte> serialize() const override
         { return std::vector<byte>(1, 1); }
   };

}

}

#endif
