/*
* DSA
* (C) 1999-2010 Jack Lloyd
*
* Distributed under the terms of the Botan license
*/

#ifndef BOTAN_DSA_H__
#define BOTAN_DSA_H__

#include <botan/dl_algo.h>
#include <botan/pk_ops.h>
#include <botan/reducer.h>
#include <botan/pow_mod.h>

namespace Botan {

/**
* DSA Public Key
*/
class BOTAN_DLL DSA_PublicKey : public virtual DL_Scheme_PublicKey
   {
   public:
      std::string algo_name() const { return "DSA"; }

      DL_Group::Format group_format() const { return DL_Group::ANSI_X9_57; }
      size_t message_parts() const { return 2; }
      size_t message_part_size() const { return group_q().bytes(); }
      size_t max_input_bits() const { return group_q().bits(); }

      DSA_PublicKey(const AlgorithmIdentifier& alg_id,
                    const secure_vector<byte>& key_bits) :
         DL_Scheme_PublicKey(alg_id, key_bits, DL_Group::ANSI_X9_57)
         {
         }

      DSA_PublicKey(const DL_Group& group, const BigInt& y);
   protected:
      DSA_PublicKey() {}
   };

/**
* DSA Private Key
*/
class BOTAN_DLL DSA_PrivateKey : public DSA_PublicKey,
                                 public virtual DL_Scheme_PrivateKey
   {
   public:
      DSA_PrivateKey(const AlgorithmIdentifier& alg_id,
                     const secure_vector<byte>& key_bits,
                     RandomNumberGenerator& rng);

      DSA_PrivateKey(RandomNumberGenerator& rng,
                     const DL_Group& group,
                     const BigInt& private_key = 0);

      bool check_key(RandomNumberGenerator& rng, bool strong) const;
   };

/**
* Object that can create a DSA signature
*/
class BOTAN_DLL DSA_Signature_Operation : public PK_Ops::Signature
   {
   public:
      DSA_Signature_Operation(const DSA_PrivateKey& dsa);

      size_t message_parts() const { return 2; }
      size_t message_part_size() const { return q.bytes(); }
      size_t max_input_bits() const { return q.bits(); }

      secure_vector<byte> sign(const byte msg[], size_t msg_len,
                              RandomNumberGenerator& rng);
   private:
      const BigInt& q;
      const BigInt& x;
      Fixed_Base_Power_Mod powermod_g_p;
      Modular_Reducer mod_q;
   };

/**
* Object that can verify a DSA signature
*/
class BOTAN_DLL DSA_Verification_Operation : public PK_Ops::Verification
   {
   public:
      DSA_Verification_Operation(const DSA_PublicKey& dsa);

      size_t message_parts() const { return 2; }
      size_t message_part_size() const { return q.bytes(); }
      size_t max_input_bits() const { return q.bits(); }

      bool with_recovery() const { return false; }

      bool verify(const byte msg[], size_t msg_len,
                  const byte sig[], size_t sig_len);
   private:
      const BigInt& q;
      const BigInt& y;

      Fixed_Base_Power_Mod powermod_g_p, powermod_y_p;
      Modular_Reducer mod_p, mod_q;
   };

}

#endif
