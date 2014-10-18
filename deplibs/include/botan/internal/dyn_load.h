/*
* Dynamically Loaded Object
* (C) 2010 Jack Lloyd
*
* Distributed under the terms of the Botan license
*/

#ifndef BOTAN_DYNAMIC_LOADER_H__
#define BOTAN_DYNAMIC_LOADER_H__

#include <string>

namespace Botan {

/**
* Represents a DLL or shared object
*/
class Dynamically_Loaded_Library
   {
   public:
      /**
      * Load a DLL (or fail with an exception)
      * @param lib_name name or path to a library
      *
      * If you don't use a full path, the search order will be defined
      * by whatever the system linker does by default. Always using fully
      * qualified pathnames can help prevent code injection attacks (eg
      * via manipulation of LD_LIBRARY_PATH on Linux)
      */
      Dynamically_Loaded_Library(const std::string& lib_name);

      /**
      * Unload the DLL
      * @warning Any pointers returned by resolve()/resolve_symbol()
      * should not be used after this destructor runs.
      */
      ~Dynamically_Loaded_Library();

      /**
      * Load a symbol (or fail with an exception)
      * @param symbol names the symbol to load
      * @return address of the loaded symbol
      */
      void* resolve_symbol(const std::string& symbol);

      /**
      * Convenience function for casting symbol to the right type
      * @param symbol names the symbol to load
      * @return address of the loaded symbol
      */
      template<typename T>
      T resolve(const std::string& symbol)
         {
         return reinterpret_cast<T>(resolve_symbol(symbol));
         }

   private:
      Dynamically_Loaded_Library(const Dynamically_Loaded_Library&);
      Dynamically_Loaded_Library& operator=(const Dynamically_Loaded_Library&);

      std::string lib_name;
      void* lib;
   };

}

#endif
