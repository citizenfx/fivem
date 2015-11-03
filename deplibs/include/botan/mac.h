/*
* Base class for message authentiction codes
* (C) 1999-2007 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/

#ifndef BOTAN_MESSAGE_AUTH_CODE_BASE_H__
#define BOTAN_MESSAGE_AUTH_CODE_BASE_H__

#include <botan/buf_comp.h>
#include <botan/sym_algo.h>
#include <botan/scan_name.h>
#include <string>

namespace Botan {

/**
* This class represents Message Authentication Code (MAC) objects.
*/
class BOTAN_DLL MessageAuthenticationCode : public Buffered_Computation,
                                            public SymmetricAlgorithm
   {
   public:
      typedef SCAN_Name Spec;

      /**
      * Create an instance based on a name
      * Will return a null pointer if the algo/provider combination cannot
      * be found. If provider is empty then best available is chosen.
      */
      static std::unique_ptr<MessageAuthenticationCode> create(const std::string& algo_spec,
                                                               const std::string& provider = "");

      /**
      * Returns the list of available providers for this algorithm, empty if not available
      */
      static std::vector<std::string> providers(const std::string& algo_spec);

      virtual ~MessageAuthenticationCode();

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
   };

}

#endif
