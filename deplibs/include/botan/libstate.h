/*
* Library Internal/Global State
* (C) 1999-2008 Jack Lloyd
*
* Distributed under the terms of the Botan license
*/

#ifndef BOTAN_LIB_STATE_H__
#define BOTAN_LIB_STATE_H__

#include <botan/global_state.h>
#include <botan/algo_factory.h>
#include <botan/rng.h>
#include <mutex>
#include <string>
#include <vector>
#include <map>

namespace Botan {

/**
* Global Library State
*/
class BOTAN_DLL Library_State
   {
   public:
      Library_State() {}

      Library_State(const Library_State&) = delete;
      Library_State& operator=(const Library_State&) = delete;

      void initialize();

      /**
      * @return global Algorithm_Factory
      */
      Algorithm_Factory& algorithm_factory() const;

      /**
      * @return global RandomNumberGenerator
      */
      RandomNumberGenerator& global_rng();

      void poll_available_sources(class Entropy_Accumulator& accum);

   private:
      static std::vector<std::unique_ptr<EntropySource>> entropy_sources();

      std::unique_ptr<Serialized_RNG> m_global_prng;

      std::mutex m_entropy_src_mutex;
      std::vector<std::unique_ptr<EntropySource>> m_sources;

      std::unique_ptr<Algorithm_Factory> m_algorithm_factory;
   };

}

#endif
