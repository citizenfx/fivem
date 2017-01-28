/*
* RFC 6979 Deterministic Nonce Generator
* (C) 2014,2015 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/

#ifndef BOTAN_RFC6979_GENERATOR_H__
#define BOTAN_RFC6979_GENERATOR_H__

#include <botan/bigint.h>
#include <string>
#include <memory>

namespace Botan {

class HMAC_DRBG;

class BOTAN_DLL RFC6979_Nonce_Generator
   {
   public:
      /**
      * Note: keeps persistent reference to order
      */
      RFC6979_Nonce_Generator(const std::string& hash,
                              const BigInt& order,
                              const BigInt& x);

      ~RFC6979_Nonce_Generator();

      const BigInt& nonce_for(const BigInt& m);
   private:
      const BigInt& m_order;
      BigInt m_k;
      size_t m_qlen, m_rlen;
      std::unique_ptr<HMAC_DRBG> m_hmac_drbg;
      secure_vector<uint8_t> m_rng_in, m_rng_out;
   };

/**
* @param x the secret (EC)DSA key
* @param q the group order
* @param h the message hash already reduced mod q
* @param hash the hash function used to generate h
*/
BigInt BOTAN_DLL generate_rfc6979_nonce(const BigInt& x,
                                        const BigInt& q,
                                        const BigInt& h,
                                        const std::string& hash);

}

#endif
