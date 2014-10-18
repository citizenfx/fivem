/*
* Library Initialization
* (C) 1999-2008 Jack Lloyd
*
* Distributed under the terms of the Botan license
*/

#ifndef BOTAN_LIBRARY_INITIALIZER_H__
#define BOTAN_LIBRARY_INITIALIZER_H__

#include <botan/build.h>
#include <string>

namespace Botan {

/**
* This class represents the Library Initialization/Shutdown Object. It
* has to exceed the lifetime of any Botan object used in an
* application.  You can call initialize/deinitialize or use
* LibraryInitializer in the RAII style.
*/
class BOTAN_DLL LibraryInitializer
   {
   public:
      /**
      * Initialize the library
      * @param options a string listing initialization options
      */
      static void initialize(const std::string& options = "");

      /**
      * Shutdown the library
      */
      static void deinitialize();

      /**
      * Initialize the library
      * @param options a string listing initialization options
      */
      LibraryInitializer(const std::string& options = "")
         { LibraryInitializer::initialize(options); }

      ~LibraryInitializer() { LibraryInitializer::deinitialize(); }
   };

}

#endif
