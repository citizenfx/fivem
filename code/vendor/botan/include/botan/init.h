/*
* Library Initialization
* (C) 1999-2008,2015 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/

#ifndef BOTAN_LIBRARY_INITIALIZER_H__
#define BOTAN_LIBRARY_INITIALIZER_H__

#include <botan/types.h>
#include <string>

namespace Botan {

/*
* Previously botan had state whose lifetime had to be explicitly
* managed by the application. As of 1.11.14 this is no longer the
* case, and this class is no longer needed and kept only for backwards
* compatibility.
*/
class BOTAN_DLL BOTAN_DEPRECATED("LibraryInitializer is no longer required") LibraryInitializer
   {
   public:
      explicit LibraryInitializer(const std::string& /*ignored*/ = "") { }

      static void initialize(const std::string& /*ignored*/ = "") {}
      static void deinitialize() {}
   };

}

#endif
