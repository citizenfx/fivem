/*
* TLS Sequence Number Handling
* (C) 2012 Jack Lloyd
*
* Released under the terms of the Botan license
*/

#ifndef BOTAN_TLS_SEQ_NUMBERS_H__
#define BOTAN_TLS_SEQ_NUMBERS_H__

#include <botan/types.h>
#include <stdexcept>

namespace Botan {

namespace TLS {

class Connection_Sequence_Numbers
   {
   public:
      virtual void new_read_cipher_state() = 0;
      virtual void new_write_cipher_state() = 0;

      virtual u16bit current_read_epoch() const = 0;
      virtual u16bit current_write_epoch() const = 0;

      virtual u64bit next_write_sequence(u16bit) = 0;
      virtual u64bit next_read_sequence() = 0;

      virtual bool already_seen(u64bit seq) const = 0;
      virtual void read_accept(u64bit seq) = 0;
   };

class Stream_Sequence_Numbers : public Connection_Sequence_Numbers
   {
   public:
      void new_read_cipher_state() override { m_read_seq_no = 0; m_read_epoch += 1; }
      void new_write_cipher_state() override { m_write_seq_no = 0; m_write_epoch += 1; }

      u16bit current_read_epoch() const override { return m_read_epoch; }
      u16bit current_write_epoch() const override { return m_write_epoch; }

      u64bit next_write_sequence(u16bit) override { return m_write_seq_no++; }
      u64bit next_read_sequence() override { return m_read_seq_no; }

      bool already_seen(u64bit) const override { return false; }
      void read_accept(u64bit) override { m_read_seq_no++; }
   private:
      u64bit m_write_seq_no = 0;
      u64bit m_read_seq_no = 0;
      u16bit m_read_epoch = 0;
      u16bit m_write_epoch = 0;
   };

class Datagram_Sequence_Numbers : public Connection_Sequence_Numbers
   {
   public:
      Datagram_Sequence_Numbers() { m_write_seqs[0] = 0; }

      void new_read_cipher_state() override { m_read_epoch += 1; }

      void new_write_cipher_state() override
         {
         m_write_epoch += 1;
         m_write_seqs[m_write_epoch] = 0;
         }

      u16bit current_read_epoch() const override { return m_read_epoch; }
      u16bit current_write_epoch() const override { return m_write_epoch; }

      u64bit next_write_sequence(u16bit epoch) override
         {
         auto i = m_write_seqs.find(epoch);
         BOTAN_ASSERT(i != m_write_seqs.end(), "Found epoch");
         return (static_cast<u64bit>(epoch) << 48) | i->second++;
         }

      u64bit next_read_sequence() override
         {
         throw std::runtime_error("DTLS uses explicit sequence numbers");
         }

      bool already_seen(u64bit sequence) const override
         {
         const size_t window_size = sizeof(m_window_bits) * 8;

         if(sequence > m_window_highest)
            return false;

         const u64bit offset = m_window_highest - sequence;

         if(offset >= window_size)
            return true; // really old?

         return (((m_window_bits >> offset) & 1) == 1);
         }

      void read_accept(u64bit sequence) override
         {
         const size_t window_size = sizeof(m_window_bits) * 8;

         if(sequence > m_window_highest)
            {
            const size_t offset = sequence - m_window_highest;
            m_window_highest += offset;

            if(offset >= window_size)
               m_window_bits = 0;
            else
               m_window_bits <<= offset;

            m_window_bits |= 0x01;
            }
         else
            {
            const u64bit offset = m_window_highest - sequence;
            m_window_bits |= (static_cast<u64bit>(1) << offset);
            }
         }

   private:
      std::map<u16bit, u64bit> m_write_seqs;
      u16bit m_write_epoch = 0;
      u16bit m_read_epoch = 0;
      u64bit m_window_highest = 0;
      u64bit m_window_bits = 0;
   };

}

}

#endif
