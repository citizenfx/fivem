/*
* TLS Handshake Serialization
* (C) 2012,2014 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/

#ifndef BOTAN_TLS_HANDSHAKE_IO_H__
#define BOTAN_TLS_HANDSHAKE_IO_H__

#include <botan/tls_magic.h>
#include <botan/tls_version.h>
#include <botan/loadstor.h>
#include <functional>
#include <vector>
#include <deque>
#include <map>
#include <set>
#include <utility>

namespace Botan {

namespace TLS {

class Handshake_Message;

/**
* Handshake IO Interface
*/
class Handshake_IO
   {
   public:
      virtual Protocol_Version initial_record_version() const = 0;

      virtual std::vector<byte> send(const Handshake_Message& msg) = 0;

      virtual bool timeout_check() = 0;

      virtual std::vector<byte> format(
         const std::vector<byte>& handshake_msg,
         Handshake_Type handshake_type) const = 0;

      virtual void add_record(const std::vector<byte>& record,
                              Record_Type type,
                              u64bit sequence_number) = 0;

      /**
      * Returns (HANDSHAKE_NONE, std::vector<>()) if no message currently available
      */
      virtual std::pair<Handshake_Type, std::vector<byte>>
         get_next_record(bool expecting_ccs) = 0;

      Handshake_IO() {}

      Handshake_IO(const Handshake_IO&) = delete;

      Handshake_IO& operator=(const Handshake_IO&) = delete;

      virtual ~Handshake_IO() {}
   };

/**
* Handshake IO for stream-based handshakes
*/
class Stream_Handshake_IO : public Handshake_IO
   {
   public:
      typedef std::function<void (byte, const std::vector<byte>&)> writer_fn;

      Stream_Handshake_IO(writer_fn writer) : m_send_hs(writer) {}

      Protocol_Version initial_record_version() const override;

      bool timeout_check() override { return false; }

      std::vector<byte> send(const Handshake_Message& msg) override;

      std::vector<byte> format(
         const std::vector<byte>& handshake_msg,
         Handshake_Type handshake_type) const override;

      void add_record(const std::vector<byte>& record,
                      Record_Type type,
                      u64bit sequence_number) override;

      std::pair<Handshake_Type, std::vector<byte>>
         get_next_record(bool expecting_ccs) override;
   private:
      std::deque<byte> m_queue;
      writer_fn m_send_hs;
   };

/**
* Handshake IO for datagram-based handshakes
*/
class Datagram_Handshake_IO : public Handshake_IO
   {
   public:
      typedef std::function<void (u16bit, byte, const std::vector<byte>&)> writer_fn;

      Datagram_Handshake_IO(writer_fn writer,
                            class Connection_Sequence_Numbers& seq,
                            u16bit mtu, u64bit initial_timeout_ms, u64bit max_timeout_ms) :
         m_seqs(seq),
         m_flights(1),
         m_initial_timeout(initial_timeout_ms),
         m_max_timeout(max_timeout_ms),
         m_send_hs(writer),
         m_mtu(mtu)
         {}

      Protocol_Version initial_record_version() const override;

      bool timeout_check() override;

      std::vector<byte> send(const Handshake_Message& msg) override;

      std::vector<byte> format(
         const std::vector<byte>& handshake_msg,
         Handshake_Type handshake_type) const override;

      void add_record(const std::vector<byte>& record,
                      Record_Type type,
                      u64bit sequence_number) override;

      std::pair<Handshake_Type, std::vector<byte>>
         get_next_record(bool expecting_ccs) override;
   private:
      void retransmit_flight(size_t flight);
      void retransmit_last_flight();

      std::vector<byte> format_fragment(
         const byte fragment[],
         size_t fragment_len,
         u16bit frag_offset,
         u16bit msg_len,
         Handshake_Type type,
         u16bit msg_sequence) const;

      std::vector<byte> format_w_seq(
         const std::vector<byte>& handshake_msg,
         Handshake_Type handshake_type,
         u16bit msg_sequence) const;

      std::vector<byte> send_message(u16bit msg_seq, u16bit epoch,
                                     Handshake_Type msg_type,
                                     const std::vector<byte>& msg);

      class Handshake_Reassembly
         {
         public:
            void add_fragment(const byte fragment[],
                              size_t fragment_length,
                              size_t fragment_offset,
                              u16bit epoch,
                              byte msg_type,
                              size_t msg_length);

            bool complete() const;

            u16bit epoch() const { return m_epoch; }

            std::pair<Handshake_Type, std::vector<byte>> message() const;
         private:
            byte m_msg_type = HANDSHAKE_NONE;
            size_t m_msg_length = 0;
            u16bit m_epoch = 0;

            // vector<bool> m_seen;
            // vector<byte> m_fragments
            std::map<size_t, byte> m_fragments;
            std::vector<byte> m_message;
         };

      struct Message_Info
         {
         Message_Info(u16bit e, Handshake_Type mt, const std::vector<byte>& msg) :
            epoch(e), msg_type(mt), msg_bits(msg) {}

         Message_Info(const Message_Info& other) = default;

         Message_Info() : epoch(0xFFFF), msg_type(HANDSHAKE_NONE) {}

         u16bit epoch;
         Handshake_Type msg_type;
         std::vector<byte> msg_bits;
         };

      class Connection_Sequence_Numbers& m_seqs;
      std::map<u16bit, Handshake_Reassembly> m_messages;
      std::set<u16bit> m_ccs_epochs;
      std::vector<std::vector<u16bit>> m_flights;
      std::map<u16bit, Message_Info> m_flight_data;

      u64bit m_initial_timeout = 0;
      u64bit m_max_timeout = 0;

      u64bit m_last_write = 0;
      u64bit m_next_timeout = 0;

      u16bit m_in_message_seq = 0;
      u16bit m_out_message_seq = 0;

      writer_fn m_send_hs;
      u16bit m_mtu;
   };

}

}

#endif
