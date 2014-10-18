/*
* Core Engine
* (C) 1999-2007 Jack Lloyd
*
* Distributed under the terms of the Botan license
*/

#ifndef BOTAN_CORE_ENGINE_H__
#define BOTAN_CORE_ENGINE_H__

#include <botan/engine.h>

namespace Botan {

/**
* Core Engine
*/
class Core_Engine : public Engine
   {
   public:
      std::string provider_name() const override { return "core"; }

      PK_Ops::Key_Agreement*
         get_key_agreement_op(const Private_Key& key, RandomNumberGenerator& rng) const override;

      PK_Ops::Signature*
         get_signature_op(const Private_Key& key, RandomNumberGenerator& rng) const override;

      PK_Ops::Verification* get_verify_op(const Public_Key& key, RandomNumberGenerator& rng) const override;

      PK_Ops::Encryption* get_encryption_op(const Public_Key& key, RandomNumberGenerator& rng) const override;

      PK_Ops::Decryption* get_decryption_op(const Private_Key& key, RandomNumberGenerator& rng) const override;

      Modular_Exponentiator* mod_exp(const BigInt& n,
                                     Power_Mod::Usage_Hints) const override;

      Keyed_Filter* get_cipher(const std::string&, Cipher_Dir,
                               Algorithm_Factory&);

      BlockCipher* find_block_cipher(const SCAN_Name&,
                                     Algorithm_Factory&) const override;

      StreamCipher* find_stream_cipher(const SCAN_Name&,
                                       Algorithm_Factory&) const override;

      HashFunction* find_hash(const SCAN_Name& request,
                              Algorithm_Factory&) const override;

      MessageAuthenticationCode* find_mac(const SCAN_Name& request,
                                          Algorithm_Factory&) const override;

      PBKDF* find_pbkdf(const SCAN_Name& algo_spec,
                        Algorithm_Factory& af) const override;
   };

/**
* Create a cipher mode filter object
* @param block_cipher a block cipher object
* @param direction are we encrypting or decrypting?
* @param mode the name of the cipher mode to use
* @param padding the mode padding to use (only used for ECB, CBC)
*/
Keyed_Filter* get_cipher_mode(const BlockCipher* block_cipher,
                              Cipher_Dir direction,
                              const std::string& mode,
                              const std::string& padding);

}

#endif
