/*
* Algorithm Lookup
* (C) 1999-2007,2015 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/

#ifndef BOTAN_LOOKUP_H__
#define BOTAN_LOOKUP_H__

#include <botan/block_cipher.h>
#include <botan/stream_cipher.h>
#include <botan/hash.h>
#include <botan/mac.h>
#include <botan/exceptn.h>
#include <string>
#include <vector>
#include <memory>

namespace Botan {

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
inline BlockCipher* get_block_cipher(const std::string& algo_spec,
                                     const std::string& provider = "")
   {
   return BlockCipher::create(algo_spec, provider).release();
   }

inline std::unique_ptr<BlockCipher> make_block_cipher(const std::string& algo_spec,
                                                      const std::string& provider = "")
   {
   std::unique_ptr<BlockCipher> p(BlockCipher::create(algo_spec, provider));
   if(p)
      return p;
   throw Algorithm_Not_Found(algo_spec);
   }

inline std::vector<std::string> get_block_cipher_providers(const std::string& algo_spec)
   {
   return BlockCipher::providers(algo_spec);
   }

/**
* Stream cipher factory method.
*
* @param algo_spec the name of the desired stream cipher
* @return pointer to the stream cipher object
*/
inline StreamCipher* get_stream_cipher(const std::string& algo_spec,
                                       const std::string& provider = "")
   {
   return StreamCipher::create(algo_spec, provider).release();
   }

inline std::unique_ptr<StreamCipher> make_stream_cipher(const std::string& algo_spec,
                                                        const std::string& provider = "")
   {
   std::unique_ptr<StreamCipher> p(StreamCipher::create(algo_spec, provider));
   if(p)
      return p;
   throw Algorithm_Not_Found(algo_spec);
   }

inline std::vector<std::string> get_stream_cipher_providers(const std::string& algo_spec)
   {
   return StreamCipher::providers(algo_spec);
   }

/**
* Hash function factory method.
*
* @param algo_spec the name of the desired hash function
* @return pointer to the hash function object
*/
inline HashFunction* get_hash_function(const std::string& algo_spec,
                                       const std::string& provider = "")
   {
   return HashFunction::create(algo_spec, provider).release();
   }

inline std::unique_ptr<HashFunction> make_hash_function(const std::string& algo_spec,
                                                        const std::string& provider = "")
   {
   std::unique_ptr<HashFunction> p(HashFunction::create(algo_spec, provider));
   if(p)
      return p;
   throw Algorithm_Not_Found(algo_spec);
   }

inline HashFunction* get_hash(const std::string& algo_spec,
                              const std::string& provider = "")
   {
   return get_hash_function(algo_spec, provider);
   }

inline std::vector<std::string> get_hash_function_providers(const std::string& algo_spec)
   {
   return HashFunction::providers(algo_spec);
   }

/**
* MAC factory method.
*
* @param algo_spec the name of the desired MAC
* @return pointer to the MAC object
*/
inline MessageAuthenticationCode* get_mac(const std::string& algo_spec,
                                             const std::string& provider = "")
   {
   return MessageAuthenticationCode::create(algo_spec, provider).release();
   }

inline std::unique_ptr<MessageAuthenticationCode> make_message_auth(const std::string& algo_spec,
                                                                       const std::string& provider = "")
   {
   std::unique_ptr<MessageAuthenticationCode> p(MessageAuthenticationCode::create(algo_spec, provider));
   if(p)
      return p;
   throw Algorithm_Not_Found(algo_spec);
   }

inline std::vector<std::string> get_mac_providers(const std::string& algo_spec)
   {
   return MessageAuthenticationCode::providers(algo_spec);
   }

}

#endif
