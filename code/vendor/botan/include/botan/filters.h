/*
* Filters
* (C) 1999-2007,2015 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/

#ifndef BOTAN_FILTERS_H_
#define BOTAN_FILTERS_H_

#include <botan/basefilt.h>
#include <botan/key_filt.h>
#include <botan/data_snk.h>
#include <botan/pipe.h>

#if defined(BOTAN_HAS_STREAM_CIPHER)
   #include <botan/stream_cipher.h>
#endif

#if defined(BOTAN_HAS_HASH)
   #include <botan/hash.h>
#endif

#if defined(BOTAN_HAS_MAC)
   #include <botan/mac.h>
#endif

#if defined(BOTAN_HAS_CODEC_FILTERS)
  #include <botan/b64_filt.h>
  #include <botan/hex_filt.h>
#endif

namespace Botan {

#if defined(BOTAN_HAS_STREAM_CIPHER)

/**
* Stream Cipher Filter
*/
class BOTAN_PUBLIC_API(2,0) StreamCipher_Filter final : public Keyed_Filter
   {
   public:

      std::string name() const override { return m_cipher->name(); }

      /**
      * Write input data
      * @param input data
      * @param input_len length of input in bytes
      */
      void write(const uint8_t input[], size_t input_len) override;

      bool valid_iv_length(size_t iv_len) const override
         { return m_cipher->valid_iv_length(iv_len); }

      /**
      * Set the initialization vector for this filter.
      * @param iv the initialization vector to set
      */
      void set_iv(const InitializationVector& iv) override
         {
         m_cipher->set_iv(iv.begin(), iv.length());
         }

      /**
      * Set the key of this filter.
      * @param key the key to set
      */
      void set_key(const SymmetricKey& key) override { m_cipher->set_key(key); }

      Key_Length_Specification key_spec() const override { return m_cipher->key_spec(); }

      /**
      * Construct a stream cipher filter.
      * @param cipher a cipher object to use
      */
      explicit StreamCipher_Filter(StreamCipher* cipher);

      /**
      * Construct a stream cipher filter.
      * @param cipher a cipher object to use
      * @param key the key to use inside this filter
      */
      StreamCipher_Filter(StreamCipher* cipher, const SymmetricKey& key);

      /**
      * Construct a stream cipher filter.
      * @param cipher the name of the desired cipher
      */
      explicit StreamCipher_Filter(const std::string& cipher);

      /**
      * Construct a stream cipher filter.
      * @param cipher the name of the desired cipher
      * @param key the key to use inside this filter
      */
      StreamCipher_Filter(const std::string& cipher, const SymmetricKey& key);
   private:
      secure_vector<uint8_t> m_buffer;
      std::unique_ptr<StreamCipher> m_cipher;
   };
#endif

#if defined(BOTAN_HAS_HASH)

/**
* Hash Filter.
*/
class BOTAN_PUBLIC_API(2,0) Hash_Filter final : public Filter
   {
   public:
      void write(const uint8_t input[], size_t len) override { m_hash->update(input, len); }
      void end_msg() override;

      std::string name() const override { return m_hash->name(); }

      /**
      * Construct a hash filter.
      * @param hash the hash function to use
      * @param len the output length of this filter. Leave the default
      * value 0 if you want to use the full output of the hashfunction
      * hash. Otherwise, specify a smaller value here so that the
      * output of the hash algorithm will be cut off.
      */
      Hash_Filter(HashFunction* hash, size_t len = 0) :
         m_hash(hash), m_out_len(len) {}

      /**
      * Construct a hash filter.
      * @param request the name of the hash algorithm to use
      * @param len the output length of this filter. Leave the default
      * value 0 if you want to use the full output of the hashfunction
      * hash. Otherwise, specify a smaller value here so that the
      * output of the hash algorithm will be cut off.
      */
      Hash_Filter(const std::string& request, size_t len = 0);

   private:
      std::unique_ptr<HashFunction> m_hash;
      const size_t m_out_len;
   };
#endif

#if defined(BOTAN_HAS_MAC)

/**
* MessageAuthenticationCode Filter.
*/
class BOTAN_PUBLIC_API(2,0) MAC_Filter final : public Keyed_Filter
   {
   public:
      void write(const uint8_t input[], size_t len) override { m_mac->update(input, len); }
      void end_msg() override;

      std::string name() const override { return m_mac->name(); }

      /**
      * Set the key of this filter.
      * @param key the key to set
      */
      void set_key(const SymmetricKey& key) override { m_mac->set_key(key); }

      Key_Length_Specification key_spec() const override { return m_mac->key_spec(); }

      /**
      * Construct a MAC filter. The MAC key will be left empty.
      * @param mac the MAC to use
      * @param out_len the output length of this filter. Leave the default
      * value 0 if you want to use the full output of the
      * MAC. Otherwise, specify a smaller value here so that the
      * output of the MAC will be cut off.
      */
      MAC_Filter(MessageAuthenticationCode* mac,
                 size_t out_len = 0) :
         m_mac(mac),
         m_out_len(out_len)
         {
         }

      /**
      * Construct a MAC filter.
      * @param mac the MAC to use
      * @param key the MAC key to use
      * @param out_len the output length of this filter. Leave the default
      * value 0 if you want to use the full output of the
      * MAC. Otherwise, specify a smaller value here so that the
      * output of the MAC will be cut off.
      */
      MAC_Filter(MessageAuthenticationCode* mac,
                 const SymmetricKey& key,
                 size_t out_len = 0) :
         m_mac(mac),
         m_out_len(out_len)
         {
         m_mac->set_key(key);
         }

      /**
      * Construct a MAC filter. The MAC key will be left empty.
      * @param mac the name of the MAC to use
      * @param len the output length of this filter. Leave the default
      * value 0 if you want to use the full output of the
      * MAC. Otherwise, specify a smaller value here so that the
      * output of the MAC will be cut off.
      */
      MAC_Filter(const std::string& mac, size_t len = 0);

      /**
      * Construct a MAC filter.
      * @param mac the name of the MAC to use
      * @param key the MAC key to use
      * @param len the output length of this filter. Leave the default
      * value 0 if you want to use the full output of the
      * MAC. Otherwise, specify a smaller value here so that the
      * output of the MAC will be cut off.
      */
      MAC_Filter(const std::string& mac, const SymmetricKey& key,
                 size_t len = 0);
   private:
      std::unique_ptr<MessageAuthenticationCode> m_mac;
      const size_t m_out_len;
   };
#endif

}

#endif
