/*
* Base class for message authentiction codes
* (C) 1999-2007 Jack Lloyd
*
* Distributed under the terms of the Botan license
*/

#ifndef BOTAN_MESSAGE_AUTH_CODE_BASE_H__
#define BOTAN_MESSAGE_AUTH_CODE_BASE_H__

#include <botan/buf_comp.h>
#include <botan/sym_algo.h>
#include <string>

namespace Botan {

/**
* This class represents Message Authentication Code (MAC) objects.
*/
class BOTAN_DLL MessageAuthenticationCode : public Buffered_Computation,
                                            public SymmetricAlgorithm
   {
   public:
      /**
      * Verify a MAC.
      * @param in the MAC to verify as a byte array
      * @param length the length of param in
      * @return true if the MAC is valid, false otherwise
      */
      virtual bool verify_mac(const byte in[], size_t length);

      /**
      * Get a new object representing the same algorithm as *this
      */
      virtual MessageAuthenticationCode* clone() const = 0;

      /**
      * Get the name of this algorithm.
      * @return name of this algorithm
      */
      virtual std::string name() const = 0;
   };

}

#endif
