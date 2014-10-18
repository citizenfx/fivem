/*
* Parallel Hash
* (C) 1999-2007 Jack Lloyd
*
* Distributed under the terms of the Botan license
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
      void clear();
      std::string name() const;
      HashFunction* clone() const;

      size_t output_length() const;

      /**
      * @param hashes a set of hashes to compute in parallel
      */
      Parallel(const std::vector<HashFunction*>& hashes);
      ~Parallel();
   private:
      void add_data(const byte[], size_t);
      void final_result(byte[]);
      std::vector<HashFunction*> hashes;
   };

}

#endif
