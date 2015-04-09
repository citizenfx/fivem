/*
* TLS Channel
* (C) 2011,2012,2014,2015 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/

#ifndef BOTAN_TLS_CHANNEL_H__
#define BOTAN_TLS_CHANNEL_H__

#include <botan/tls_policy.h>
#include <botan/tls_session.h>
#include <botan/tls_alert.h>
#include <botan/tls_session_manager.h>
#include <botan/x509cert.h>
#include <vector>
#include <string>
#include <map>

namespace Botan {

namespace TLS {

class Connection_Cipher_State;
class Connection_Sequence_Numbers;
class Handshake_State;

/**
* Generic interface for TLS endpoint
*/
class BOTAN_DLL Channel
   {
   public:
      typedef std::function<void (const byte[], size_t)> output_fn;
      typedef std::function<void (const byte[], size_t)> data_cb;
      typedef std::function<void (Alert, const byte[], size_t)> alert_cb;
      typedef std::function<bool (const Session&)> handshake_cb;

      Channel(output_fn out,
              data_cb app_data_cb,
              alert_cb alert_cb,
              handshake_cb hs_cb,
              Session_Manager& session_manager,
              RandomNumberGenerator& rng,
              bool is_datagram,
              size_t reserved_io_buffer_size);

      Channel(const Channel&) = delete;

      Channel& operator=(const Channel&) = delete;

      virtual ~Channel();

      /**
      * Inject TLS traffic received from counterparty
      * @return a hint as the how many more bytes we need to process the
      *         current record (this may be 0 if on a record boundary)
      */
      size_t received_data(const byte buf[], size_t buf_size);

      /**
      * Inject TLS traffic received from counterparty
      * @return a hint as the how many more bytes we need to process the
      *         current record (this may be 0 if on a record boundary)
      */
      size_t received_data(const std::vector<byte>& buf);

      /**
      * Inject plaintext intended for counterparty
      * Throws an exception if is_active() is false
      */
      void send(const byte buf[], size_t buf_size);

      /**
      * Inject plaintext intended for counterparty
      * Throws an exception if is_active() is false
      */
      void send(const std::string& val);

      /**
      * Inject plaintext intended for counterparty
      * Throws an exception if is_active() is false
      */
      template<typename Alloc>
         void send(const std::vector<unsigned char, Alloc>& val)
         {
         send(&val[0], val.size());
         }

      /**
      * Send a TLS alert message. If the alert is fatal, the internal
      * state (keys, etc) will be reset.
      * @param alert the Alert to send
      */
      void send_alert(const Alert& alert);

      /**
      * Send a warning alert
      */
      void send_warning_alert(Alert::Type type) { send_alert(Alert(type, false)); }

      /**
      * Send a fatal alert
      */
      void send_fatal_alert(Alert::Type type) { send_alert(Alert(type, true)); }

      /**
      * Send a close notification alert
      */
      void close() { send_warning_alert(Alert::CLOSE_NOTIFY); }

      /**
      * @return true iff the connection is active for sending application data
      */
      bool is_active() const;

      /**
      * @return true iff the connection has been definitely closed
      */
      bool is_closed() const;


      /**
      * @return certificate chain of the peer (may be empty)
      */
      std::vector<X509_Certificate> peer_cert_chain() const;

      /**
      * Key material export (RFC 5705)
      * @param label a disambiguating label string
      * @param context a per-association context value
      * @param length the length of the desired key in bytes
      * @return key of length bytes
      */
      SymmetricKey key_material_export(const std::string& label,
                                       const std::string& context,
                                       size_t length) const;

      /**
      * Attempt to renegotiate the session
      * @param force_full_renegotiation if true, require a full renegotiation,
      * otherwise allow session resumption
      */
      void renegotiate(bool force_full_renegotiation = false);

      /**
      * @return true iff the counterparty supports the secure
      * renegotiation extensions.
      */
      bool secure_renegotiation_supported() const;

      /**
      * Perform a handshake timeout check. This does nothing unless
      * this is a DTLS channel with a pending handshake state, in
      * which case we check for timeout and potentially retransmit
      * handshake packets.
      */
      bool timeout_check();

      /**
      * @return true iff the peer supports heartbeat messages
      */
      bool peer_supports_heartbeats() const;

      /**
      * @return true iff we are allowed to send heartbeat messages
      */
      bool heartbeat_sending_allowed() const;

      /**
      * Attempt to send a heartbeat message (if negotiated with counterparty)
      * @param payload will be echoed back
      * @param payload_size size of payload in bytes
      * @param pad_bytes include 16 + pad_bytes extra bytes in the message (not echoed)
      */
      void heartbeat(const byte payload[], size_t payload_size, size_t pad_bytes = 0);

      /**
      * Attempt to send a heartbeat message (if negotiated with counterparty)
      */
      void heartbeat() { heartbeat(nullptr, 0); }
   protected:

      virtual void process_handshake_msg(const Handshake_State* active_state,
                                         Handshake_State& pending_state,
                                         Handshake_Type type,
                                         const std::vector<byte>& contents) = 0;

      virtual void initiate_handshake(Handshake_State& state,
                                      bool force_full_renegotiation) = 0;

      virtual std::vector<X509_Certificate>
         get_peer_cert_chain(const Handshake_State& state) const = 0;

      virtual Handshake_State* new_handshake_state(class Handshake_IO* io) = 0;

      Handshake_State& create_handshake_state(Protocol_Version version);

      void activate_session();

      void change_cipher_spec_reader(Connection_Side side);

      void change_cipher_spec_writer(Connection_Side side);

      /* secure renegotiation handling */

      void secure_renegotiation_check(const class Client_Hello* client_hello);
      void secure_renegotiation_check(const class Server_Hello* server_hello);

      std::vector<byte> secure_renegotiation_data_for_client_hello() const;
      std::vector<byte> secure_renegotiation_data_for_server_hello() const;

      RandomNumberGenerator& rng() { return m_rng; }

      Session_Manager& session_manager() { return m_session_manager; }

      bool save_session(const Session& session) const { return m_handshake_cb(session); }

   private:
      size_t maximum_fragment_size() const;

      void send_record(byte record_type, const std::vector<byte>& record);

      void send_record_under_epoch(u16bit epoch, byte record_type,
                                   const std::vector<byte>& record);

      void send_record_array(u16bit epoch, byte record_type,
                             const byte input[], size_t length);

      void write_record(Connection_Cipher_State* cipher_state,
                        u16bit epoch, byte type, const byte input[], size_t length);

      Connection_Sequence_Numbers& sequence_numbers() const;

      std::shared_ptr<Connection_Cipher_State> read_cipher_state_epoch(u16bit epoch) const;

      std::shared_ptr<Connection_Cipher_State> write_cipher_state_epoch(u16bit epoch) const;

      void reset_state();

      const Handshake_State* active_state() const { return m_active_state.get(); }

      const Handshake_State* pending_state() const { return m_pending_state.get(); }

      bool m_is_datagram;

      /* callbacks */
      handshake_cb m_handshake_cb;
      data_cb m_data_cb;
      alert_cb m_alert_cb;
      output_fn m_output_fn;

      /* external state */
      RandomNumberGenerator& m_rng;
      Session_Manager& m_session_manager;

      /* sequence number state */
      std::unique_ptr<Connection_Sequence_Numbers> m_sequence_numbers;

      /* pending and active connection states */
      std::unique_ptr<Handshake_State> m_active_state;
      std::unique_ptr<Handshake_State> m_pending_state;

      /* cipher states for each epoch */
      std::map<u16bit, std::shared_ptr<Connection_Cipher_State>> m_write_cipher_states;
      std::map<u16bit, std::shared_ptr<Connection_Cipher_State>> m_read_cipher_states;

      /* I/O buffers */
      secure_vector<byte> m_writebuf;
      secure_vector<byte> m_readbuf;
   };

}

}

#endif
