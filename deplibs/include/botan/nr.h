/*
* Nyberg-Rueppel
* (C) 1999-2010 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/

#ifndef BOTAN_NYBERG_RUEPPEL_H__
#define BOTAN_NYBERG_RUEPPEL_H__

#include <botan/dl_algo.h>

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

}

#endif
