/*
* Algorithm Lookup
* (C) 1999-2007,2015 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/

#ifndef BOTAN_LOOKUP_H__
#define BOTAN_LOOKUP_H__

#include <botan/types.h>
#include <string>
#include <vector>
#include <memory>

namespace Botan {

class BlockCipher;
class StreamCipher;
class HashFunction;
class MessageAuthenticationCode;
class PBKDF;

/*
* Get an algorithm object
*  NOTE: these functions create and return new objects, letting the
* caller assume ownership of them
*/

/**
* Block cipher factory method.
*
* @param algo_spec the name of the desired block cipher
* @return pointer to the block cipher object
*/
BOTAN_DLL BlockCipher* get_block_cipher(const std::string& algo_spec,
                                        const std::string& provider = "");

BOTAN_DLL std::unique_ptr<BlockCipher> make_block_cipher(const std::string& algo_spec,
                                                         const std::string& provider = "");

BOTAN_DLL std::vector<std::string> get_block_cipher_providers(const std::string& algo_spec);

/**
* Stream cipher factory method.
*
* @param algo_spec the name of the desired stream cipher
* @return pointer to the stream cipher object
*/
BOTAN_DLL StreamCipher* get_stream_cipher(const std::string& algo_spec,
                                          const std::string& provider = "");

BOTAN_DLL std::unique_ptr<StreamCipher> make_stream_cipher(const std::string& algo_spec,
                                                           const std::string& provider = "");

BOTAN_DLL std::vector<std::string> get_stream_cipher_providers(const std::string& algo_spec);

/**
* Hash function factory method.
*
* @param algo_spec the name of the desired hash function
* @return pointer to the hash function object
*/
BOTAN_DLL HashFunction* get_hash_function(const std::string& algo_spec,
                                          const std::string& provider = "");

BOTAN_DLL std::unique_ptr<HashFunction> make_hash_function(const std::string& algo_spec,
                                                           const std::string& provider = "");

inline HashFunction* get_hash(const std::string& algo_spec,
                              const std::string& provider = "")
   {
   return get_hash_function(algo_spec, provider);
   }

BOTAN_DLL std::vector<std::string> get_hash_function_providers(const std::string& algo_spec);

/**
* MAC factory method.
*
* @param algo_spec the name of the desired MAC
* @return pointer to the MAC object
*/
BOTAN_DLL MessageAuthenticationCode* get_mac(const std::string& algo_spec,
                                             const std::string& provider = "");

BOTAN_DLL std::unique_ptr<MessageAuthenticationCode> make_message_auth(const std::string& algo_spec,
                                                                       const std::string& provider = "");

BOTAN_DLL std::vector<std::string> get_mac_providers(const std::string& algo_spec);

/**
* Password based key derivation function factory method
* @param algo_spec the name of the desired PBKDF algorithm
* @return pointer to newly allocated object of that type
*/
BOTAN_DLL PBKDF* get_pbkdf(const std::string& algo_spec,
                           const std::string& provider = "");

}

#endif
