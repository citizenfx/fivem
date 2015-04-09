/*
* Public Key Interface
* (C) 1999-2010 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/

#ifndef BOTAN_PUBKEY_H__
#define BOTAN_PUBKEY_H__

#include <botan/pk_keys.h>
#include <botan/pk_ops.h>
#include <botan/symkey.h>
#include <botan/rng.h>
#include <botan/eme.h>
#include <botan/emsa.h>
#include <botan/kdf.h>

namespace Botan {

/**
* The two types of signature format supported by Botan.
*/
enum Signature_Format { IEEE_1363, DER_SEQUENCE };

/**
* Public Key Encryptor
*/
class BOTAN_DLL PK_Encryptor
   {
   public:

      /**
      * Encrypt a message.
      * @param in the message as a byte array
      * @param length the length of the above byte array
      * @param rng the random number source to use
      * @return encrypted message
      */
      std::vector<byte> encrypt(const byte in[], size_t length,
                                 RandomNumberGenerator& rng) const
         {
         return enc(in, length, rng);
         }

      /**
      * Encrypt a message.
      * @param in the message
      * @param rng the random number source to use
      * @return encrypted message
      */
      template<typename Alloc>
      std::vector<byte> encrypt(const std::vector<byte, Alloc>& in,
                                RandomNumberGenerator& rng) const
         {
         return enc(&in[0], in.size(), rng);
         }

      /**
      * Return the maximum allowed message size in bytes.
      * @return maximum message size in bytes
      */
      virtual size_t maximum_input_size() const = 0;

      PK_Encryptor() {}
      virtual ~PK_Encryptor() {}

      PK_Encryptor(const PK_Encryptor&) = delete;

      PK_Encryptor& operator=(const PK_Encryptor&) = delete;

   private:
      virtual std::vector<byte> enc(const byte[], size_t,
                                    RandomNumberGenerator&) const = 0;
   };

/**
* Public Key Decryptor
*/
class BOTAN_DLL PK_Decryptor
   {
   public:
      /**
      * Decrypt a ciphertext.
      * @param in the ciphertext as a byte array
      * @param length the length of the above byte array
      * @return decrypted message
      */
      secure_vector<byte> decrypt(const byte in[], size_t length) const
         {
         return dec(in, length);
         }

      /**
      * Decrypt a ciphertext.
      * @param in the ciphertext
      * @return decrypted message
      */
      template<typename Alloc>
      secure_vector<byte> decrypt(const std::vector<byte, Alloc>& in) const
         {
         return dec(&in[0], in.size());
         }

      PK_Decryptor() {}
      virtual ~PK_Decryptor() {}

      PK_Decryptor(const PK_Decryptor&) = delete;
      PK_Decryptor& operator=(const PK_Decryptor&) = delete;

   private:
      virtual secure_vector<byte> dec(const byte[], size_t) const = 0;
   };

/**
* Public Key Signer. Use the sign_message() functions for small
* messages. Use multiple calls update() to process large messages and
* generate the signature by finally calling signature().
*/
class BOTAN_DLL PK_Signer
   {
   public:
      /**
      * Sign a message.
      * @param in the message to sign as a byte array
      * @param length the length of the above byte array
      * @param rng the rng to use
      * @return signature
      */
      std::vector<byte> sign_message(const byte in[], size_t length,
                                     RandomNumberGenerator& rng)
         {
         this->update(in, length);
         return this->signature(rng);
         }

      /**
      * Sign a message.
      * @param in the message to sign
      * @param rng the rng to use
      * @return signature
      */
      std::vector<byte> sign_message(const std::vector<byte>& in,
                                     RandomNumberGenerator& rng)
         { return sign_message(&in[0], in.size(), rng); }

      std::vector<byte> sign_message(const secure_vector<byte>& in,
                                     RandomNumberGenerator& rng)
         { return sign_message(&in[0], in.size(), rng); }

      /**
      * Add a message part (single byte).
      * @param in the byte to add
      */
      void update(byte in) { update(&in, 1); }

      /**
      * Add a message part.
      * @param in the message part to add as a byte array
      * @param length the length of the above byte array
      */
      void update(const byte in[], size_t length);

      /**
      * Add a message part.
      * @param in the message part to add
      */
      void update(const std::vector<byte>& in) { update(&in[0], in.size()); }

      /**
      * Get the signature of the so far processed message (provided by the
      * calls to update()).
      * @param rng the rng to use
      * @return signature of the total message
      */
      std::vector<byte> signature(RandomNumberGenerator& rng);

      /**
      * Set the output format of the signature.
      * @param format the signature format to use
      */
      void set_output_format(Signature_Format format) { m_sig_format = format; }

      /**
      * Construct a PK Signer.
      * @param key the key to use inside this signer
      * @param emsa the EMSA to use
      * An example would be "EMSA1(SHA-224)".
      * @param format the signature format to use
      */
      PK_Signer(const Private_Key& key,
                const std::string& emsa,
                Signature_Format format = IEEE_1363);
   private:
      std::unique_ptr<PK_Ops::Signature> m_op;
      Signature_Format m_sig_format;
   };

/**
* Public Key Verifier. Use the verify_message() functions for small
* messages. Use multiple calls update() to process large messages and
* verify the signature by finally calling check_signature().
*/
class BOTAN_DLL PK_Verifier
   {
   public:
      /**
      * Verify a signature.
      * @param msg the message that the signature belongs to, as a byte array
      * @param msg_length the length of the above byte array msg
      * @param sig the signature as a byte array
      * @param sig_length the length of the above byte array sig
      * @return true if the signature is valid
      */
      bool verify_message(const byte msg[], size_t msg_length,
                          const byte sig[], size_t sig_length);
      /**
      * Verify a signature.
      * @param msg the message that the signature belongs to
      * @param sig the signature
      * @return true if the signature is valid
      */
      template<typename Alloc, typename Alloc2>
      bool verify_message(const std::vector<byte, Alloc>& msg,
                          const std::vector<byte, Alloc2>& sig)
         {
         return verify_message(&msg[0], msg.size(),
                               &sig[0], sig.size());
         }

      /**
      * Add a message part (single byte) of the message corresponding to the
      * signature to be verified.
      * @param in the byte to add
      */
      void update(byte in) { update(&in, 1); }

      /**
      * Add a message part of the message corresponding to the
      * signature to be verified.
      * @param msg_part the new message part as a byte array
      * @param length the length of the above byte array
      */
      void update(const byte msg_part[], size_t length);

      /**
      * Add a message part of the message corresponding to the
      * signature to be verified.
      * @param in the new message part
      */
      void update(const std::vector<byte>& in)
         { update(&in[0], in.size()); }

      /**
      * Check the signature of the buffered message, i.e. the one build
      * by successive calls to update.
      * @param sig the signature to be verified as a byte array
      * @param length the length of the above byte array
      * @return true if the signature is valid, false otherwise
      */
      bool check_signature(const byte sig[], size_t length);

      /**
      * Check the signature of the buffered message, i.e. the one build
      * by successive calls to update.
      * @param sig the signature to be verified
      * @return true if the signature is valid, false otherwise
      */
      template<typename Alloc>
      bool check_signature(const std::vector<byte, Alloc>& sig)
         {
         return check_signature(&sig[0], sig.size());
         }

      /**
      * Set the format of the signatures fed to this verifier.
      * @param format the signature format to use
      */
      void set_input_format(Signature_Format format);

      /**
      * Construct a PK Verifier.
      * @param pub_key the public key to verify against
      * @param emsa the EMSA to use (eg "EMSA3(SHA-1)")
      * @param format the signature format to use
      */
      PK_Verifier(const Public_Key& pub_key,
                  const std::string& emsa,
                  Signature_Format format = IEEE_1363);
   private:
      std::unique_ptr<PK_Ops::Verification> m_op;
      Signature_Format m_sig_format;
   };

/**
* Key used for key agreement
*/
class BOTAN_DLL PK_Key_Agreement
   {
   public:

      /*
      * Perform Key Agreement Operation
      * @param key_len the desired key output size
      * @param in the other parties key
      * @param in_len the length of in in bytes
      * @param params extra derivation params
      * @param params_len the length of params in bytes
      */
      SymmetricKey derive_key(size_t key_len,
                              const byte in[],
                              size_t in_len,
                              const byte params[],
                              size_t params_len) const;

      /*
      * Perform Key Agreement Operation
      * @param key_len the desired key output size
      * @param in the other parties key
      * @param in_len the length of in in bytes
      * @param params extra derivation params
      * @param params_len the length of params in bytes
      */
      SymmetricKey derive_key(size_t key_len,
                              const std::vector<byte>& in,
                              const byte params[],
                              size_t params_len) const
         {
         return derive_key(key_len, &in[0], in.size(),
                           params, params_len);
         }

      /*
      * Perform Key Agreement Operation
      * @param key_len the desired key output size
      * @param in the other parties key
      * @param in_len the length of in in bytes
      * @param params extra derivation params
      */
      SymmetricKey derive_key(size_t key_len,
                              const byte in[], size_t in_len,
                              const std::string& params = "") const
         {
         return derive_key(key_len, in, in_len,
                           reinterpret_cast<const byte*>(params.data()),
                           params.length());
         }

      /*
      * Perform Key Agreement Operation
      * @param key_len the desired key output size
      * @param in the other parties key
      * @param params extra derivation params
      */
      SymmetricKey derive_key(size_t key_len,
                              const std::vector<byte>& in,
                              const std::string& params = "") const
         {
         return derive_key(key_len, &in[0], in.size(),
                           reinterpret_cast<const byte*>(params.data()),
                           params.length());
         }

      /**
      * Construct a PK Key Agreement.
      * @param key the key to use
      * @param kdf name of the KDF to use (or 'Raw' for no KDF)
      */
      PK_Key_Agreement(const Private_Key& key, const std::string& kdf);
   private:
      std::unique_ptr<PK_Ops::Key_Agreement> m_op;
   };

/**
* Encryption with an MR algorithm and an EME.
*/
class BOTAN_DLL PK_Encryptor_EME : public PK_Encryptor
   {
   public:
      size_t maximum_input_size() const;

      /**
      * Construct an instance.
      * @param key the key to use inside the decryptor
      * @param eme the EME to use
      */
      PK_Encryptor_EME(const Public_Key& key,
                       const std::string& eme);
   private:
      std::vector<byte> enc(const byte[], size_t,
                             RandomNumberGenerator& rng) const;

      std::unique_ptr<PK_Ops::Encryption> m_op;
   };

/**
* Decryption with an MR algorithm and an EME.
*/
class BOTAN_DLL PK_Decryptor_EME : public PK_Decryptor
   {
   public:
     /**
      * Construct an instance.
      * @param key the key to use inside the encryptor
      * @param eme the EME to use
      */
      PK_Decryptor_EME(const Private_Key& key,
                       const std::string& eme);
   private:
      secure_vector<byte> dec(const byte[], size_t) const;

      std::unique_ptr<PK_Ops::Decryption> m_op;
   };

}

#endif
