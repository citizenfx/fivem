/*
* Hash Function Base Class
* (C) 1999-2008 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/

#ifndef BOTAN_HASH_FUNCTION_BASE_CLASS_H__
#define BOTAN_HASH_FUNCTION_BASE_CLASS_H__

#include <botan/scan_name.h>
#include <botan/buf_comp.h>
#include <string>

namespace Botan {

/**
* This class represents hash function (message digest) objects
*/
class BOTAN_DLL HashFunction : public Buffered_Computation
   {
   public:
      /**
      * @return new object representing the same algorithm as *this
      */
      virtual HashFunction* clone() const = 0;

      virtual void clear() = 0;

      virtual std::string name() const = 0;

      /**
      * @return hash block size as defined for this algorithm
      */
      virtual size_t hash_block_size() const { return 0; }

      typedef SCAN_Name Spec;
   };

}

#endif
