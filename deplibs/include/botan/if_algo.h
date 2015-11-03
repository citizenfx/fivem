/*
* IF Scheme
* (C) 1999-2007 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/

#ifndef BOTAN_IF_ALGO_H__
#define BOTAN_IF_ALGO_H__

#include <botan/bigint.h>
#include <botan/x509_key.h>

namespace Botan {

/**
* This class represents public keys
* of integer factorization based (IF) public key schemes.
*/
class BOTAN_DLL IF_Scheme_PublicKey : public virtual Public_Key
   {
   public:
      IF_Scheme_PublicKey(const AlgorithmIdentifier& alg_id,
                          const secure_vector<byte>& key_bits);

      IF_Scheme_PublicKey(const BigInt& n, const BigInt& e) :
         n(n), e(e) {}

      bool check_key(RandomNumberGenerator& rng, bool) const override;

      AlgorithmIdentifier algorithm_identifier() const override;

      std::vector<byte> x509_subject_public_key() const override;

      /**
      * @return public modulus
      */
      const BigInt& get_n() const { return n; }

      /**
      * @return public exponent
      */
      const BigInt& get_e() const { return e; }

      size_t max_input_bits() const override { return (n.bits() - 1); }

      size_t estimated_strength() const override;

   protected:
      IF_Scheme_PublicKey() {}

      BigInt n, e;
   };

/**
* This class represents public keys
* of integer factorization based (IF) public key schemes.
*/
class BOTAN_DLL IF_Scheme_PrivateKey : public virtual IF_Scheme_PublicKey,
                                       public virtual Private_Key
   {
   public:

      IF_Scheme_PrivateKey(RandomNumberGenerator& rng,
                           const BigInt& prime1, const BigInt& prime2,
                           const BigInt& exp, const BigInt& d_exp,
                           const BigInt& mod);

      IF_Scheme_PrivateKey(RandomNumberGenerator& rng,
                           const AlgorithmIdentifier& alg_id,
                           const secure_vector<byte>& key_bits);

      bool check_key(RandomNumberGenerator& rng, bool) const override;

      /**
      * Get the first prime p.
      * @return prime p
      */
      const BigInt& get_p() const { return p; }

      /**
      * Get the second prime q.
      * @return prime q
      */
      const BigInt& get_q() const { return q; }

      /**
      * Get d with exp * d = 1 mod (p - 1, q - 1).
      * @return d
      */
      const BigInt& get_d() const { return d; }

      const BigInt& get_c() const { return c; }
      const BigInt& get_d1() const { return d1; }
      const BigInt& get_d2() const { return d2; }

      secure_vector<byte> pkcs8_private_key() const override;

   protected:
      IF_Scheme_PrivateKey() {}

      BigInt d, p, q, d1, d2, c;
   };

}

#endif
