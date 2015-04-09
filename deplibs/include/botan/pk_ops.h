/*
* PK Operation Types
* (C) 2010,2015 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/

#ifndef BOTAN_PK_OPERATIONS_H__
#define BOTAN_PK_OPERATIONS_H__

#include <botan/pk_keys.h>
#include <botan/secmem.h>
#include <botan/rng.h>

namespace Botan {

class EME;
class KDF;
class EMSA;

namespace PK_Ops {

template<typename Key>
class PK_Spec
   {
   public:
      PK_Spec(const Key& key, const std::string& pad) :
         m_key(key), m_pad(pad) {}

      std::string algo_name() const { return m_key.algo_name(); }

      std::string as_string() const { return algo_name() + "/" + padding(); }

      const Key& key() const { return m_key; }
      const std::string& padding() const { return m_pad; }
   private:
      const Key& m_key;
      const std::string m_pad;
   };

typedef PK_Spec<Public_Key> PK_Spec_Public_Key;
typedef PK_Spec<Private_Key> PK_Spec_Private_Key;

/**
* Public key encryption interface
*/
class BOTAN_DLL Encryption
   {
   public:
      virtual size_t max_input_bits() const = 0;

      virtual secure_vector<byte> encrypt(const byte msg[], size_t msg_len, RandomNumberGenerator& rng) = 0;

      typedef PK_Spec_Public_Key Spec;

      virtual ~Encryption() {}
   };

/**
* Public key decryption interface
*/
class BOTAN_DLL Decryption
   {
   public:
      typedef PK_Spec_Private_Key Spec;

      virtual size_t max_input_bits() const = 0;

      virtual secure_vector<byte> decrypt(const byte msg[],  size_t msg_len) = 0;

      virtual ~Decryption() {}
   };

/**
* Public key signature verification interface
*/
class BOTAN_DLL Verification
   {
   public:
      typedef PK_Spec_Public_Key Spec;

      /*
      * Add more data to the message currently being signed
      * @param msg the message
      * @param msg_len the length of msg in bytes
      */
      virtual void update(const byte msg[], size_t msg_len) = 0;

      /*
      * Perform a signature operation
      * @param rng a random number generator
      */
      virtual bool is_valid_signature(const byte sig[], size_t sig_len) = 0;

      /**
      * Get the maximum message size in bits supported by this public key.
      * @return maximum message in bits
      */
      virtual size_t max_input_bits() const = 0;

      /**
      * Find out the number of message parts supported by this scheme.
      * @return number of message parts
      */
      virtual size_t message_parts() const { return 1; }

      /**
      * Find out the message part size supported by this scheme/key.
      * @return size of the message parts
      */
      virtual size_t message_part_size() const { return 0; }

      virtual ~Verification() {}
   };

/**
* Public key signature creation interface
*/
class BOTAN_DLL Signature
   {
   public:
      typedef PK_Spec_Private_Key Spec;

      /**
      * Find out the number of message parts supported by this scheme.
      * @return number of message parts
      */
      virtual size_t message_parts() const { return 1; }

      /**
      * Find out the message part size supported by this scheme/key.
      * @return size of the message parts
      */
      virtual size_t message_part_size() const { return 0; }

      /*
      * Add more data to the message currently being signed
      * @param msg the message
      * @param msg_len the length of msg in bytes
      */
      virtual void update(const byte msg[], size_t msg_len) = 0;

      /*
      * Perform a signature operation
      * @param rng a random number generator
      */
      virtual secure_vector<byte> sign(RandomNumberGenerator& rng) = 0;

      virtual ~Signature() {}
   };

/**
* A generic key agreement operation (eg DH or ECDH)
*/
class BOTAN_DLL Key_Agreement
   {
   public:
      typedef PK_Spec_Private_Key Spec;

      virtual secure_vector<byte> agree(size_t key_len,
                                        const byte other_key[], size_t other_key_len,
                                        const byte salt[], size_t salt_len) = 0;

      virtual ~Key_Agreement() {}
   };

}

}

#endif
