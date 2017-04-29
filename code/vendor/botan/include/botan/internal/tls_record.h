/*
* TLS Record Handling
* (C) 2004-2012 Jack Lloyd
*     2016 Matthias Gierlings
*
* Botan is released under the Simplified BSD License (see license.txt)
*/

#ifndef BOTAN_TLS_RECORDS_H__
#define BOTAN_TLS_RECORDS_H__

#include <botan/tls_magic.h>
#include <botan/tls_version.h>
#include <botan/aead.h>
#include <botan/block_cipher.h>
#include <botan/mac.h>
#include <vector>
#include <chrono>

namespace Botan {

namespace TLS {

class Ciphersuite;
class Session_Keys;

class Connection_Sequence_Numbers;

/**
* TLS Cipher State
*/
class Connection_Cipher_State
   {
   public:
      /**
      * Initialize a new cipher state
      */
      Connection_Cipher_State(Protocol_Version version,
                              Connection_Side which_side,
                              bool is_our_side,
                              const Ciphersuite& suite,
                              const Session_Keys& keys,
                              bool uses_encrypt_then_mac);

      AEAD_Mode* aead() { return m_aead.get(); }

      std::vector<uint8_t> aead_nonce(uint64_t seq, RandomNumberGenerator& rng);

      std::vector<uint8_t> aead_nonce(const uint8_t record[], size_t record_len, uint64_t seq);

      std::vector<uint8_t> format_ad(uint64_t seq, uint8_t type,
                                  Protocol_Version version,
                                  uint16_t ptext_length);

      size_t nonce_bytes_from_handshake() const { return m_nonce_bytes_from_handshake; }
      size_t nonce_bytes_from_record() const { return m_nonce_bytes_from_record; }
      bool cbc_nonce() const { return m_cbc_nonce; }

      std::chrono::seconds age() const
         {
         return std::chrono::duration_cast<std::chrono::seconds>(
            std::chrono::system_clock::now() - m_start_time);
         }

   private:
      std::chrono::system_clock::time_point m_start_time;
      std::unique_ptr<AEAD_Mode> m_aead;

      std::vector<uint8_t> m_nonce;
      size_t m_nonce_bytes_from_handshake;
      size_t m_nonce_bytes_from_record;
      bool m_cbc_nonce;
   };

class Record
   {
   public:
      Record(secure_vector<uint8_t>& data,
             uint64_t* sequence,
             Protocol_Version* protocol_version,
             Record_Type* type)
         : m_data(data), m_sequence(sequence), m_protocol_version(protocol_version),
           m_type(type), m_size(data.size()) {};

      secure_vector<uint8_t>& get_data() { return m_data; }

      Protocol_Version* get_protocol_version() { return m_protocol_version; }

      uint64_t* get_sequence() { return m_sequence; }

      Record_Type* get_type() { return m_type; }

      size_t& get_size() { return m_size; }

   private:
      secure_vector<uint8_t>& m_data;
      uint64_t* m_sequence;
      Protocol_Version* m_protocol_version;
      Record_Type* m_type;
      size_t m_size;
   };

class Record_Message
   {
   public:
      Record_Message(const uint8_t* data, size_t size)
         : m_type(0), m_sequence(0), m_data(data), m_size(size) {};
      Record_Message(uint8_t type, uint64_t sequence, const uint8_t* data, size_t size)
         : m_type(type), m_sequence(sequence), m_data(data),
           m_size(size) {};

      uint8_t& get_type() { return m_type; };
      uint64_t& get_sequence() { return m_sequence; };
      const uint8_t* get_data() { return m_data; };
      size_t& get_size() { return m_size; };

   private:
      uint8_t m_type;
      uint64_t m_sequence;
      const uint8_t* m_data;
      size_t m_size;
};

class Record_Raw_Input
   {
   public:
      Record_Raw_Input(const uint8_t* data, size_t size, size_t& consumed,
                       bool is_datagram)
         : m_data(data), m_size(size), m_consumed(consumed),
           m_is_datagram(is_datagram) {};

      const uint8_t*& get_data() { return m_data; };

      size_t& get_size() { return m_size; };

      size_t& get_consumed() { return m_consumed; };
      void set_consumed(size_t consumed) { m_consumed = consumed; }

      bool is_datagram() { return m_is_datagram; };

   private:
      const uint8_t* m_data;
      size_t m_size;
      size_t& m_consumed;
      bool m_is_datagram;
   };


/**
* Create a TLS record
* @param write_buffer the output record is placed here
* @param rec_msg is the plaintext message
* @param version is the protocol version
* @param msg_sequence is the sequence number
* @param cipherstate is the writing cipher state
* @param rng is a random number generator
*/
void write_record(secure_vector<uint8_t>& write_buffer,
                  Record_Message rec_msg,
                  Protocol_Version version,
                  uint64_t msg_sequence,
                  Connection_Cipher_State* cipherstate,
                  RandomNumberGenerator& rng);

// epoch -> cipher state
typedef std::function<std::shared_ptr<Connection_Cipher_State> (uint16_t)> get_cipherstate_fn;

/**
* Decode a TLS record
* @return zero if full message, else number of bytes still needed
*/
size_t read_record(secure_vector<uint8_t>& read_buffer,
                   Record_Raw_Input& raw_input,
                   Record& rec,
                   Connection_Sequence_Numbers* sequence_numbers,
                   get_cipherstate_fn get_cipherstate);

}

}

#endif
