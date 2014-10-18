/*
* ElGamal
* (C) 1999-2007 Jack Lloyd
*
* Distributed under the terms of the Botan license
*/

#ifndef BOTAN_ELGAMAL_H__
#define BOTAN_ELGAMAL_H__

#include <botan/dl_algo.h>
#include <botan/numthry.h>
#include <botan/reducer.h>
#include <botan/blinding.h>
#include <botan/pk_ops.h>

namespace Botan {

/**
* ElGamal Public Key
*/
class BOTAN_DLL ElGamal_PublicKey : public virtual DL_Scheme_PublicKey
   {
   public:
      std::string algo_name() const { return "ElGamal"; }
      DL_Group::Format group_format() const { return DL_Group::ANSI_X9_42; }

      size_t max_input_bits() const { return (group_p().bits() - 1); }

      ElGamal_PublicKey(const AlgorithmIdentifier& alg_id,
                        const secure_vector<byte>& key_bits) :
         DL_Scheme_PublicKey(alg_id, key_bits, DL_Group::ANSI_X9_42)
         {}

      ElGamal_PublicKey(const DL_Group& group, const BigInt& y);
   protected:
      ElGamal_PublicKey() {}
   };

/**
* ElGamal Private Key
*/
class BOTAN_DLL ElGamal_PrivateKey : public ElGamal_PublicKey,
                                     public virtual DL_Scheme_PrivateKey
   {
   public:
      bool check_key(RandomNumberGenerator& rng, bool) const;

      ElGamal_PrivateKey(const AlgorithmIdentifier& alg_id,
                         const secure_vector<byte>& key_bits,
                         RandomNumberGenerator& rng);

      ElGamal_PrivateKey(RandomNumberGenerator& rng,
                         const DL_Group& group,
                         const BigInt& priv_key = 0);
   };

/**
* ElGamal encryption operation
*/
class BOTAN_DLL ElGamal_Encryption_Operation : public PK_Ops::Encryption
   {
   public:
      size_t max_input_bits() const { return mod_p.get_modulus().bits() - 1; }

      ElGamal_Encryption_Operation(const ElGamal_PublicKey& key);

      secure_vector<byte> encrypt(const byte msg[], size_t msg_len,
                                 RandomNumberGenerator& rng);

   private:
      Fixed_Base_Power_Mod powermod_g_p, powermod_y_p;
      Modular_Reducer mod_p;
   };

/**
* ElGamal decryption operation
*/
class BOTAN_DLL ElGamal_Decryption_Operation : public PK_Ops::Decryption
   {
   public:
      size_t max_input_bits() const { return mod_p.get_modulus().bits() - 1; }

      ElGamal_Decryption_Operation(const ElGamal_PrivateKey& key,
                                   RandomNumberGenerator& rng);

      secure_vector<byte> decrypt(const byte msg[], size_t msg_len);
   private:
      Fixed_Exponent_Power_Mod powermod_x_p;
      Modular_Reducer mod_p;
      Blinder blinder;
   };

}

#endif
