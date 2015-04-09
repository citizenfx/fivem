/*
* EMSA Classes
* (C) 1999-2007 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/

#ifndef BOTAN_PUBKEY_EMSA_H__
#define BOTAN_PUBKEY_EMSA_H__

#include <botan/scan_name.h>
#include <botan/secmem.h>
#include <botan/rng.h>

namespace Botan {

/**
* Encoding Method for Signatures, Appendix
*/
class BOTAN_DLL EMSA
   {
   public:
      typedef SCAN_Name Spec;

      /**
      * Add more data to the signature computation
      * @param input some data
      * @param length length of input in bytes
      */
      virtual void update(const byte input[], size_t length) = 0;

      /**
      * @return raw hash
      */
      virtual secure_vector<byte> raw_data() = 0;

      /**
      * Return the encoding of a message
      * @param msg the result of raw_data()
      * @param output_bits the desired output bit size
      * @param rng a random number generator
      * @return encoded signature
      */
      virtual secure_vector<byte> encoding_of(const secure_vector<byte>& msg,
                                             size_t output_bits,
                                             RandomNumberGenerator& rng) = 0;

      /**
      * Verify the encoding
      * @param coded the received (coded) message representative
      * @param raw the computed (local, uncoded) message representative
      * @param key_bits the size of the key in bits
      * @return true if coded is a valid encoding of raw, otherwise false
      */
      virtual bool verify(const secure_vector<byte>& coded,
                          const secure_vector<byte>& raw,
                          size_t key_bits) = 0;
      virtual ~EMSA() {}
   };

/**
* Factory method for EMSA (message-encoding methods for signatures
* with appendix) objects
* @param algo_spec the name of the EME to create
* @return pointer to newly allocated object of that type
*/
BOTAN_DLL EMSA* get_emsa(const std::string& algo_spec);

}

#endif
