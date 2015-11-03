/*
* Parallel Hash
* (C) 1999-2007 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/

#ifndef BOTAN_PARALLEL_HASH_H__
#define BOTAN_PARALLEL_HASH_H__

#include <botan/hash.h>
#include <vector>

namespace Botan {

/**
* Parallel Hashes
*/
class BOTAN_DLL Parallel : public HashFunction
   {
   public:
      void clear() override;
      std::string name() const override;
      HashFunction* clone() const override;

      size_t output_length() const override;

      /**
      * @param hashes a set of hashes to compute in parallel
      */
      Parallel(const std::vector<HashFunction*>& hashes);

      Parallel(const Parallel&) = delete;
      Parallel& operator=(const Parallel&) = delete;

      static Parallel* make(const Spec& spec);
   private:
      Parallel() {}

      void add_data(const byte[], size_t) override;
      void final_result(byte[]) override;

      std::vector<std::unique_ptr<HashFunction>> hashes;
   };

}

#endif
