/*
* X.509 SIGNED Object
* (C) 1999-2007 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/

#ifndef BOTAN_X509_OBJECT_H__
#define BOTAN_X509_OBJECT_H__

#include <botan/asn1_obj.h>
#include <botan/x509_key.h>
#include <botan/rng.h>
#include <vector>

namespace Botan {

/**
* This class represents abstract X.509 signed objects as
* in the X.500 SIGNED macro
*/
class BOTAN_DLL X509_Object : public ASN1_Object
   {
   public:
      /**
      * The underlying data that is to be or was signed
      * @return data that is or was signed
      */
      std::vector<uint8_t> tbs_data() const;

      /**
      * @return signature on tbs_data()
      */
      std::vector<uint8_t> signature() const;

      /**
      * @return signature algorithm that was used to generate signature
      */
      AlgorithmIdentifier signature_algorithm() const;

      /**
      * @return hash algorithm that was used to generate signature
      */
      std::string hash_used_for_signature() const;

      /**
      * Create a signed X509 object.
      * @param signer the signer used to sign the object
      * @param rng the random number generator to use
      * @param alg_id the algorithm identifier of the signature scheme
      * @param tbs the tbs bits to be signed
      * @return signed X509 object
      */
      static std::vector<uint8_t> make_signed(class PK_Signer* signer,
                                           RandomNumberGenerator& rng,
                                           const AlgorithmIdentifier& alg_id,
                                           const secure_vector<uint8_t>& tbs);

      /**
      * Check the signature on this data
      * @param key the public key purportedly used to sign this data
      * @return true if the signature is valid, otherwise false
      */
      bool check_signature(const Public_Key& key) const;

      /**
      * Check the signature on this data
      * @param key the public key purportedly used to sign this data
      *        the pointer will be deleted after use
      * @return true if the signature is valid, otherwise false
      */
      bool check_signature(const Public_Key* key) const;

      /**
      * DER encode an X509_Object
      * See @ref ASN1_Object::encode_into()
      */
      void encode_into(class DER_Encoder& to) const override;

      /**
      * Decode a BER encoded X509_Object
      * See @ref ASN1_Object::decode_from()
      */
      void decode_from(class BER_Decoder& from) override;

      /**
      * @return BER encoding of this
      */
      std::vector<uint8_t> BER_encode() const;

      /**
      * @return PEM encoding of this
      */
      std::string PEM_encode() const;

      virtual ~X509_Object() {}
   protected:
      X509_Object(DataSource& src, const std::string& pem_labels);
      X509_Object(const std::vector<uint8_t>& vec, const std::string& labels);

#if defined(BOTAN_TARGET_OS_HAS_FILESYSTEM)
      X509_Object(const std::string& file, const std::string& pem_labels);
#endif

      void do_decode();
      X509_Object() {}
      AlgorithmIdentifier m_sig_algo;
      std::vector<uint8_t> m_tbs_bits, m_sig;
   private:
      virtual void force_decode() = 0;
      void init(DataSource&, const std::string&);

      std::vector<std::string> m_PEM_labels_allowed;
      std::string m_PEM_label_pref;
   };

}

#endif
