/**
 * (C) Copyright Projet SECRET, INRIA, Rocquencourt
 * (C) Bhaskar Biswas and  Nicolas Sendrier
 *
 * (C) 2014 cryptosource GmbH
 * (C) 2014 Falko Strenzke fstrenzke@cryptosource.de
 *
 * Botan is released under the Simplified BSD License (see license.txt)
 *
 */

#ifndef BOTAN_MCELIECE_KEY_H__
#define BOTAN_MCELIECE_KEY_H__

#include <botan/pk_keys.h>
#include <botan/polyn_gf2m.h>
#include <botan/exceptn.h>

namespace Botan {

class BOTAN_DLL McEliece_PublicKey : public virtual Public_Key
   {
   public:
      McEliece_PublicKey(const std::vector<byte>& key_bits);

      McEliece_PublicKey(std::vector<byte> const& pub_matrix, u32bit the_t, u32bit the_code_length) :
         m_public_matrix(pub_matrix),
         m_t(the_t),
         m_code_length(the_code_length)
            {}

      McEliece_PublicKey(const McEliece_PublicKey& other);

      secure_vector<byte> random_plaintext_element(RandomNumberGenerator& rng) const;

      std::string algo_name() const override { return "McEliece"; }

      /**
      * Get the maximum number of bits allowed to be fed to this key.
      * @result the maximum number of input bits
      */
      size_t max_input_bits() const override { return get_message_word_bit_length(); }

      AlgorithmIdentifier algorithm_identifier() const override;

      size_t estimated_strength() const override;

      std::vector<byte> x509_subject_public_key() const override;

      bool check_key(RandomNumberGenerator&, bool) const override
         { return true; }

      u32bit get_t() const { return m_t; }
      u32bit get_code_length() const { return m_code_length; }
      u32bit get_message_word_bit_length() const;
      const std::vector<byte>& get_public_matrix() const { return m_public_matrix; }

      bool operator==(const McEliece_PublicKey& other) const;
      bool operator!=(const McEliece_PublicKey& other) const { return !(*this == other); }

   protected:
      McEliece_PublicKey() {}

      std::vector<byte> m_public_matrix;
      u32bit m_t;
      u32bit m_code_length;
   };

class BOTAN_DLL McEliece_PrivateKey : public virtual McEliece_PublicKey,
                                      public virtual Private_Key
   {
   public:
      /**
      * Get the maximum number of bits allowed to be fed to this key.
      * @result the maximum number of input bits
      */
      size_t max_input_bits() const override { return m_Linv.size(); }

      /**
      Generate a McEliece key pair

      Suggested parameters for a given security level (SL)

      SL=80 n=1632 t=33 - 59 KB pubkey 140 KB privkey
      SL=107 n=2480 t=45 - 128 KB pubkey 300 KB privkey
      SL=128 n=2960 t=57 - 195 KB pubkey 459 KB privkey
      SL=147 n=3408 t=67 - 265 KB pubkey 622 KB privkey
      SL=191 n=4624 t=95 - 516 KB pubkey 1234 KB privkey
      SL=256 n=6624 t=115 - 942 KB pubkey 2184 KB privkey
      */
      McEliece_PrivateKey(RandomNumberGenerator& rng, size_t code_length, size_t t);

      McEliece_PrivateKey(const secure_vector<byte>& key_bits);

      McEliece_PrivateKey(polyn_gf2m const& goppa_polyn,
                          std::vector<u32bit> const& parity_check_matrix_coeffs,
                          std::vector<polyn_gf2m> const& square_root_matrix,
                          std::vector<gf2m> const& inverse_support,
                          std::vector<byte> const& public_matrix );

      bool check_key(RandomNumberGenerator& rng, bool strong) const override;

      polyn_gf2m const& get_goppa_polyn() const { return m_g; }
      std::vector<u32bit> const& get_H_coeffs() const { return m_coeffs; }
      std::vector<gf2m> const& get_Linv() const { return m_Linv; }
      std::vector<polyn_gf2m> const& get_sqrtmod() const { return m_sqrtmod; }

      inline u32bit get_dimension() const { return m_dimension; }

      inline u32bit get_codimension() const { return m_codimension; }

      secure_vector<byte> pkcs8_private_key() const override;

      bool operator==(const McEliece_PrivateKey & other) const;

      bool operator!=(const McEliece_PrivateKey& other) const { return !(*this == other); }

   private:
      polyn_gf2m m_g;
      std::vector<polyn_gf2m> m_sqrtmod;
      std::vector<gf2m> m_Linv;
      std::vector<u32bit> m_coeffs;

      u32bit m_codimension;
      u32bit m_dimension;
   };

/**
* Estimate work factor for McEliece
* @return estimated security level for these key parameters
*/
BOTAN_DLL size_t mceliece_work_factor(size_t code_size, size_t t);

}

#endif
