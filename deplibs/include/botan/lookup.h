/*
* Algorithm Lookup
* (C) 1999-2007 Jack Lloyd
*
* Distributed under the terms of the Botan license
*/

#ifndef BOTAN_LOOKUP_H__
#define BOTAN_LOOKUP_H__

#include <botan/libstate.h>
#include <botan/engine.h>
#include <botan/filters.h>
#include <botan/mode_pad.h>
#include <botan/kdf.h>
#include <botan/eme.h>
#include <botan/emsa.h>
#include <botan/pbkdf.h>

namespace Botan {

/**
* Retrieve an object prototype from the global factory
* @param algo_spec an algorithm name
* @return constant prototype object (use clone to create usable object),
          library retains ownership
*/
inline const BlockCipher*
retrieve_block_cipher(const std::string& algo_spec)
   {
   Algorithm_Factory& af = global_state().algorithm_factory();
   return af.prototype_block_cipher(algo_spec);
   }

/**
* Retrieve an object prototype from the global factory
* @param algo_spec an algorithm name
* @return constant prototype object (use clone to create usable object),
          library retains ownership
*/
inline const StreamCipher*
retrieve_stream_cipher(const std::string& algo_spec)
   {
   Algorithm_Factory& af = global_state().algorithm_factory();
   return af.prototype_stream_cipher(algo_spec);
   }

/**
* Retrieve an object prototype from the global factory
* @param algo_spec an algorithm name
* @return constant prototype object (use clone to create usable object),
          library retains ownership
*/
inline const HashFunction*
retrieve_hash(const std::string& algo_spec)
   {
   Algorithm_Factory& af = global_state().algorithm_factory();
   return af.prototype_hash_function(algo_spec);
   }

/**
* Retrieve an object prototype from the global factory
* @param algo_spec an algorithm name
* @return constant prototype object (use clone to create usable object),
          library retains ownership
*/
inline const MessageAuthenticationCode*
retrieve_mac(const std::string& algo_spec)
   {
   Algorithm_Factory& af = global_state().algorithm_factory();
   return af.prototype_mac(algo_spec);
   }

/*
* Get an algorithm object
*  NOTE: these functions create and return new objects, letting the
* caller assume ownership of them
*/

/**
* Block cipher factory method.
* @deprecated Call algorithm_factory() directly
*
* @param algo_spec the name of the desired block cipher
* @return pointer to the block cipher object
*/
inline BlockCipher* get_block_cipher(const std::string& algo_spec)
   {
   Algorithm_Factory& af = global_state().algorithm_factory();
   return af.make_block_cipher(algo_spec);
   }

/**
* Stream cipher factory method.
* @deprecated Call algorithm_factory() directly
*
* @param algo_spec the name of the desired stream cipher
* @return pointer to the stream cipher object
*/
inline StreamCipher* get_stream_cipher(const std::string& algo_spec)
   {
   Algorithm_Factory& af = global_state().algorithm_factory();
   return af.make_stream_cipher(algo_spec);
   }

/**
* Hash function factory method.
* @deprecated Call algorithm_factory() directly
*
* @param algo_spec the name of the desired hash function
* @return pointer to the hash function object
*/
inline HashFunction* get_hash(const std::string& algo_spec)
   {
   Algorithm_Factory& af = global_state().algorithm_factory();
   return af.make_hash_function(algo_spec);
   }

/**
* MAC factory method.
* @deprecated Call algorithm_factory() directly
*
* @param algo_spec the name of the desired MAC
* @return pointer to the MAC object
*/
inline MessageAuthenticationCode* get_mac(const std::string& algo_spec)
   {
   Algorithm_Factory& af = global_state().algorithm_factory();
   return af.make_mac(algo_spec);
   }

/**
* Password based key derivation function factory method
* @param algo_spec the name of the desired PBKDF algorithm
* @return pointer to newly allocated object of that type
*/
BOTAN_DLL PBKDF* get_pbkdf(const std::string& algo_spec);

/**
* @deprecated Use get_pbkdf
* @param algo_spec the name of the desired algorithm
* @return pointer to newly allocated object of that type
*/
inline PBKDF* get_s2k(const std::string& algo_spec)
   {
   return get_pbkdf(algo_spec);
   }

/*
* Get a cipher object
*/

/**
* Factory method for general symmetric cipher filters.
* @param algo_spec the name of the desired cipher
* @param key the key to be used for encryption/decryption performed by
* the filter
* @param iv the initialization vector to be used
* @param direction determines whether the filter will be an encrypting
* or decrypting filter
* @return pointer to newly allocated encryption or decryption filter
*/
BOTAN_DLL Keyed_Filter* get_cipher(const std::string& algo_spec,
                                   const SymmetricKey& key,
                                   const InitializationVector& iv,
                                   Cipher_Dir direction);

/**
* Factory method for general symmetric cipher filters.
* @param algo_spec the name of the desired cipher
* @param key the key to be used for encryption/decryption performed by
* the filter
* @param direction determines whether the filter will be an encrypting
* or decrypting filter
* @return pointer to the encryption or decryption filter
*/
BOTAN_DLL Keyed_Filter* get_cipher(const std::string& algo_spec,
                                   const SymmetricKey& key,
                                   Cipher_Dir direction);

/**
* Factory method for general symmetric cipher filters. No key will be
* set in the filter.
*
* @param algo_spec the name of the desired cipher
* @param direction determines whether the filter will be an encrypting or
* decrypting filter
* @return pointer to the encryption or decryption filter
*/
BOTAN_DLL Keyed_Filter* get_cipher(const std::string& algo_spec,
                                   Cipher_Dir direction);

/**
* Check if an algorithm exists.
* @param algo_spec the name of the algorithm to check for
* @return true if the algorithm exists, false otherwise
*/
BOTAN_DLL bool have_algorithm(const std::string& algo_spec);

/**
* Check if a block cipher algorithm exists.
* @deprecated Call algorithm_factory() directly
*
* @param algo_spec the name of the algorithm to check for
* @return true if the algorithm exists, false otherwise
*/
inline bool have_block_cipher(const std::string& algo_spec)
   {
   Algorithm_Factory& af = global_state().algorithm_factory();
   return (af.prototype_block_cipher(algo_spec) != nullptr);
   }

/**
* Check if a stream cipher algorithm exists.
* @deprecated Call algorithm_factory() directly
*
* @param algo_spec the name of the algorithm to check for
* @return true if the algorithm exists, false otherwise
*/
inline bool have_stream_cipher(const std::string& algo_spec)
   {
   Algorithm_Factory& af = global_state().algorithm_factory();
   return (af.prototype_stream_cipher(algo_spec) != nullptr);
   }

/**
* Check if a hash algorithm exists.
* @deprecated Call algorithm_factory() directly
*
* @param algo_spec the name of the algorithm to check for
* @return true if the algorithm exists, false otherwise
*/
inline bool have_hash(const std::string& algo_spec)
   {
   Algorithm_Factory& af = global_state().algorithm_factory();
   return (af.prototype_hash_function(algo_spec) != nullptr);
   }

/**
* Check if a MAC algorithm exists.
* @deprecated Call algorithm_factory() directly
*
* @param algo_spec the name of the algorithm to check for
* @return true if the algorithm exists, false otherwise
*/
inline bool have_mac(const std::string& algo_spec)
   {
   Algorithm_Factory& af = global_state().algorithm_factory();
   return (af.prototype_mac(algo_spec) != nullptr);
   }

/*
* Query information about an algorithm
*/

/**
* Find out the block size of a certain symmetric algorithm.
* @deprecated Call algorithm_factory() directly
*
* @param algo_spec the name of the algorithm
* @return block size of the specified algorithm
*/
BOTAN_DLL size_t block_size_of(const std::string& algo_spec);

/**
* Find out the output length of a certain symmetric algorithm.
* @deprecated Call algorithm_factory() directly
*
* @param algo_spec the name of the algorithm
* @return output length of the specified algorithm
*/
BOTAN_DLL size_t output_length_of(const std::string& algo_spec);

}

#endif
