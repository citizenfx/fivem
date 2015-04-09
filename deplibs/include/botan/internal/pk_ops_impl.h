/*
* (C) 2015 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/

#ifndef BOTAN_PK_OPERATION_IMPL_H__
#define BOTAN_PK_OPERATION_IMPL_H__

#include <botan/pk_ops.h>

namespace Botan {

namespace PK_Ops {

class Encryption_with_EME : public Encryption
   {
   public:
      size_t max_input_bits() const override;

      secure_vector<byte> encrypt(const byte msg[], size_t msg_len,
                                  RandomNumberGenerator& rng) override;

      ~Encryption_with_EME();
   protected:
      Encryption_with_EME(const std::string& eme);
   private:
      virtual size_t max_raw_input_bits() const = 0;

      virtual secure_vector<byte> raw_encrypt(const byte msg[], size_t len,
                                              RandomNumberGenerator& rng) = 0;
      std::unique_ptr<EME> m_eme;
   };

class Decryption_with_EME : public Decryption
   {
   public:
      size_t max_input_bits() const override;

      secure_vector<byte> decrypt(const byte msg[], size_t msg_len) override;

      ~Decryption_with_EME();
   protected:
      Decryption_with_EME(const std::string& eme);
   private:
      virtual size_t max_raw_input_bits() const = 0;
      virtual secure_vector<byte> raw_decrypt(const byte msg[], size_t len) = 0;
      std::unique_ptr<EME> m_eme;
   };

class Verification_with_EMSA : public Verification
   {
   public:
      void update(const byte msg[], size_t msg_len) override;
      bool is_valid_signature(const byte sig[], size_t sig_len) override;

      bool do_check(const secure_vector<byte>& msg,
                    const byte sig[], size_t sig_len);

   protected:

      Verification_with_EMSA(const std::string& emsa);
      ~Verification_with_EMSA();

      /**
      * @return boolean specifying if this key type supports message
      * recovery and thus if you need to call verify() or verify_mr()
      */
      virtual bool with_recovery() const = 0;

      /*
      * Perform a signature check operation
      * @param msg the message
      * @param msg_len the length of msg in bytes
      * @param sig the signature
      * @param sig_len the length of sig in bytes
      * @returns if signature is a valid one for message
      */
      virtual bool verify(const byte[], size_t,
                          const byte[], size_t)
         {
         throw Invalid_State("Message recovery required");
         }

      /*
      * Perform a signature operation (with message recovery)
      * Only call this if with_recovery() returns true
      * @param msg the message
      * @param msg_len the length of msg in bytes
      * @returns recovered message
      */
      virtual secure_vector<byte> verify_mr(const byte[], size_t)
         {
         throw Invalid_State("Message recovery not supported");
         }

   private:
      std::unique_ptr<EMSA> m_emsa;
   };

class Signature_with_EMSA : public Signature
   {
   public:
      void update(const byte msg[], size_t msg_len) override;

      secure_vector<byte> sign(RandomNumberGenerator& rng) override;
   protected:
      Signature_with_EMSA(const std::string& emsa);
      ~Signature_with_EMSA();
   private:

      /**
      * Get the maximum message size in bits supported by this public key.
      * @return maximum message in bits
      */
      virtual size_t max_input_bits() const = 0;

      bool self_test_signature(const std::vector<byte>& msg,
                               const std::vector<byte>& sig) const;

      virtual secure_vector<byte> raw_sign(const byte msg[], size_t msg_len,
                                           RandomNumberGenerator& rng) = 0;

      std::unique_ptr<EMSA> m_emsa;
   };

class Key_Agreement_with_KDF : public Key_Agreement
   {
   public:
      secure_vector<byte> agree(size_t key_len,
                                const byte other_key[], size_t other_key_len,
                                const byte salt[], size_t salt_len) override;

   protected:
      Key_Agreement_with_KDF(const std::string& kdf);
      ~Key_Agreement_with_KDF();
   private:
      virtual secure_vector<byte> raw_agree(const byte w[], size_t w_len) = 0;
      std::unique_ptr<KDF> m_kdf;
   };

}

}

#endif
