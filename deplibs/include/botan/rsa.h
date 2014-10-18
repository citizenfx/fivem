/*
* RSA
* (C) 1999-2008 Jack Lloyd
*
* Distributed under the terms of the Botan license
*/

#ifndef BOTAN_RSA_H__
#define BOTAN_RSA_H__

#include <botan/if_algo.h>
#include <botan/pk_ops.h>
#include <botan/reducer.h>
#include <botan/blinding.h>

namespace Botan {

/**
* RSA Public Key
*/
class BOTAN_DLL RSA_PublicKey : public virtual IF_Scheme_PublicKey
   {
   public:
      std::string algo_name() const { return "RSA"; }

      RSA_PublicKey(const AlgorithmIdentifier& alg_id,
                    const secure_vector<byte>& key_bits) :
         IF_Scheme_PublicKey(alg_id, key_bits)
         {}

      /**
      * Create a RSA_PublicKey
      * @arg n the modulus
      * @arg e the exponent
      */
      RSA_PublicKey(const BigInt& n, const BigInt& e) :
         IF_Scheme_PublicKey(n, e)
         {}

   protected:
      RSA_PublicKey() {}
   };

/**
* RSA Private Key
*/
class BOTAN_DLL RSA_PrivateKey : public RSA_PublicKey,
                                 public IF_Scheme_PrivateKey
   {
   public:
      bool check_key(RandomNumberGenerator& rng, bool) const;

      RSA_PrivateKey(const AlgorithmIdentifier& alg_id,
                     const secure_vector<byte>& key_bits,
                     RandomNumberGenerator& rng) :
         IF_Scheme_PrivateKey(rng, alg_id, key_bits) {}

      /**
      * Construct a private key from the specified parameters.
      * @param rng a random number generator
      * @param p the first prime
      * @param q the second prime
      * @param e the exponent
      * @param d if specified, this has to be d with
      * exp * d = 1 mod (p - 1, q - 1). Leave it as 0 if you wish to
      * the constructor to calculate it.
      * @param n if specified, this must be n = p * q. Leave it as 0
      * if you wish to the constructor to calculate it.
      */
      RSA_PrivateKey(RandomNumberGenerator& rng,
                     const BigInt& p, const BigInt& q,
                     const BigInt& e, const BigInt& d = 0,
                     const BigInt& n = 0) :
         IF_Scheme_PrivateKey(rng, p, q, e, d, n) {}

      /**
      * Create a new private key with the specified bit length
      * @param rng the random number generator to use
      * @param bits the desired bit length of the private key
      * @param exp the public exponent to be used
      */
      RSA_PrivateKey(RandomNumberGenerator& rng,
                     size_t bits, size_t exp = 65537);
   };

/**
* RSA private (decrypt/sign) operation
*/
class BOTAN_DLL RSA_Private_Operation : public PK_Ops::Signature,
                                        public PK_Ops::Decryption
   {
   public:
      RSA_Private_Operation(const RSA_PrivateKey& rsa,
                            RandomNumberGenerator& rng);

      size_t max_input_bits() const { return (n.bits() - 1); }

      secure_vector<byte> sign(const byte msg[], size_t msg_len,
                              RandomNumberGenerator& rng);

      secure_vector<byte> decrypt(const byte msg[], size_t msg_len);

   private:
      BigInt private_op(const BigInt& m) const;

      const BigInt& n;
      const BigInt& q;
      const BigInt& c;
      Fixed_Exponent_Power_Mod powermod_e_n, powermod_d1_p, powermod_d2_q;
      Modular_Reducer mod_p;
      Blinder blinder;
   };

/**
* RSA public (encrypt/verify) operation
*/
class BOTAN_DLL RSA_Public_Operation : public PK_Ops::Verification,
                                       public PK_Ops::Encryption
   {
   public:
      RSA_Public_Operation(const RSA_PublicKey& rsa) :
         n(rsa.get_n()), powermod_e_n(rsa.get_e(), rsa.get_n())
         {}

      size_t max_input_bits() const { return (n.bits() - 1); }
      bool with_recovery() const { return true; }

      secure_vector<byte> encrypt(const byte msg[], size_t msg_len,
                                 RandomNumberGenerator&)
         {
         BigInt m(msg, msg_len);
         return BigInt::encode_1363(public_op(m), n.bytes());
         }

      secure_vector<byte> verify_mr(const byte msg[], size_t msg_len)
         {
         BigInt m(msg, msg_len);
         return BigInt::encode_locked(public_op(m));
         }

   private:
      BigInt public_op(const BigInt& m) const
         {
         if(m >= n)
            throw Invalid_Argument("RSA public op - input is too large");
         return powermod_e_n(m);
         }

      const BigInt& n;
      Fixed_Exponent_Power_Mod powermod_e_n;
   };

}

#endif
