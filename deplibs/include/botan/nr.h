/*
* Nyberg-Rueppel
* (C) 1999-2010 Jack Lloyd
*
* Distributed under the terms of the Botan license
*/

#ifndef BOTAN_NYBERG_RUEPPEL_H__
#define BOTAN_NYBERG_RUEPPEL_H__

#include <botan/dl_algo.h>
#include <botan/pk_ops.h>
#include <botan/numthry.h>
#include <botan/reducer.h>

namespace Botan {

/**
* Nyberg-Rueppel Public Key
*/
class BOTAN_DLL NR_PublicKey : public virtual DL_Scheme_PublicKey
   {
   public:
      std::string algo_name() const { return "NR"; }

      DL_Group::Format group_format() const { return DL_Group::ANSI_X9_57; }

      size_t message_parts() const { return 2; }
      size_t message_part_size() const { return group_q().bytes(); }
      size_t max_input_bits() const { return (group_q().bits() - 1); }

      NR_PublicKey(const AlgorithmIdentifier& alg_id,
                   const secure_vector<byte>& key_bits);

      NR_PublicKey(const DL_Group& group, const BigInt& pub_key);
   protected:
      NR_PublicKey() {}
   };

/**
* Nyberg-Rueppel Private Key
*/
class BOTAN_DLL NR_PrivateKey : public NR_PublicKey,
                                public virtual DL_Scheme_PrivateKey
   {
   public:
      bool check_key(RandomNumberGenerator& rng, bool strong) const;

      NR_PrivateKey(const AlgorithmIdentifier& alg_id,
                    const secure_vector<byte>& key_bits,
                    RandomNumberGenerator& rng);

      NR_PrivateKey(RandomNumberGenerator& rng,
                    const DL_Group& group,
                    const BigInt& x = 0);
   };

/**
* Nyberg-Rueppel signature operation
*/
class BOTAN_DLL NR_Signature_Operation : public PK_Ops::Signature
   {
   public:
      NR_Signature_Operation(const NR_PrivateKey& nr);

      size_t message_parts() const { return 2; }
      size_t message_part_size() const { return q.bytes(); }
      size_t max_input_bits() const { return (q.bits() - 1); }

      secure_vector<byte> sign(const byte msg[], size_t msg_len,
                              RandomNumberGenerator& rng);
   private:
      const BigInt& q;
      const BigInt& x;
      Fixed_Base_Power_Mod powermod_g_p;
      Modular_Reducer mod_q;
   };

/**
* Nyberg-Rueppel verification operation
*/
class BOTAN_DLL NR_Verification_Operation : public PK_Ops::Verification
   {
   public:
      NR_Verification_Operation(const NR_PublicKey& nr);

      size_t message_parts() const { return 2; }
      size_t message_part_size() const { return q.bytes(); }
      size_t max_input_bits() const { return (q.bits() - 1); }

      bool with_recovery() const { return true; }

      secure_vector<byte> verify_mr(const byte msg[], size_t msg_len);
   private:
      const BigInt& q;
      const BigInt& y;

      Fixed_Base_Power_Mod powermod_g_p, powermod_y_p;
      Modular_Reducer mod_p, mod_q;
   };

}

#endif
