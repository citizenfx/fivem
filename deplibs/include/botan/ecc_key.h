/*
* ECDSA
* (C) 2007 Falko Strenzke, FlexSecure GmbH
*          Manuel Hartl, FlexSecure GmbH
* (C) 2008-2010 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/

#ifndef BOTAN_ECC_PUBLIC_KEY_BASE_H__
#define BOTAN_ECC_PUBLIC_KEY_BASE_H__

#include <botan/ec_group.h>
#include <botan/pk_keys.h>
#include <botan/x509_key.h>
#include <botan/pkcs8.h>

namespace Botan {

/**
* This class represents abstract ECC public keys. When encoding a key
* via an encoder that can be accessed via the corresponding member
* functions, the key will decide upon its internally stored encoding
* information whether to encode itself with or without domain
* parameters, or using the domain parameter oid. Furthermore, a public
* key without domain parameters can be decoded. In that case, it
* cannot be used for verification until its domain parameters are set
* by calling the corresponding member function.
*/
class BOTAN_DLL EC_PublicKey : public virtual Public_Key
   {
   public:
      EC_PublicKey(const EC_Group& dom_par,
                   const PointGFp& pub_point);

      EC_PublicKey(const AlgorithmIdentifier& alg_id,
                   const secure_vector<byte>& key_bits);

      /**
      * Get the public point of this key.
      * @throw Invalid_State is thrown if the
      * domain parameters of this point are not set
      * @result the public point of this key
      */
      const PointGFp& public_point() const { return public_key; }

      AlgorithmIdentifier algorithm_identifier() const;

      std::vector<byte> x509_subject_public_key() const;

      bool check_key(RandomNumberGenerator& rng,
                     bool strong) const;

      /**
      * Get the domain parameters of this key.
      * @throw Invalid_State is thrown if the
      * domain parameters of this point are not set
      * @result the domain parameters of this key
      */
      const EC_Group& domain() const { return domain_params; }

      /**
      * Set the domain parameter encoding to be used when encoding this key.
      * @param enc the encoding to use
      */
      void set_parameter_encoding(EC_Group_Encoding enc);

      /**
      * Return the DER encoding of this keys domain in whatever format
      * is preset for this particular key
      */
      std::vector<byte> DER_domain() const
         { return domain().DER_encode(domain_format()); }

      /**
      * Get the domain parameter encoding to be used when encoding this key.
      * @result the encoding to use
      */
      EC_Group_Encoding domain_format() const
         { return domain_encoding; }

      size_t estimated_strength() const override;

   protected:
      EC_PublicKey() : domain_encoding(EC_DOMPAR_ENC_EXPLICIT) {}

      EC_Group domain_params;
      PointGFp public_key;
      EC_Group_Encoding domain_encoding;
   };

/**
* This abstract class represents ECC private keys
*/
class BOTAN_DLL EC_PrivateKey : public virtual EC_PublicKey,
                                public virtual Private_Key
   {
   public:
     EC_PrivateKey(RandomNumberGenerator& rng,
                   const EC_Group& domain,
                   const BigInt& private_key);

      EC_PrivateKey(const AlgorithmIdentifier& alg_id,
                    const secure_vector<byte>& key_bits);

      secure_vector<byte> pkcs8_private_key() const;

      /**
      * Get the private key value of this key object.
      * @result the private key value of this key object
      */
      const BigInt& private_value() const;
   protected:
      EC_PrivateKey() {}

      BigInt private_key;
   };

}

#endif
