/*
* PKCS#11 Module
* (C) 2016 Daniel Neus, Sirrix AG
* (C) 2016 Philipp Weber, Sirrix AG
*
* Botan is released under the Simplified BSD License (see license.txt)
*/

#ifndef BOTAN_P11_MODULE_H_
#define BOTAN_P11_MODULE_H_

#include <string>
#include <memory>

#include <botan/p11.h>
#include <botan/dyn_load.h>

namespace Botan {
namespace PKCS11 {

/**
* Loads the PKCS#11 shared library
* Calls C_Initialize on load and C_Finalize on destruction
*/
class BOTAN_PUBLIC_API(2,0) Module final
   {
   public:
      /**
      * Loads the shared library and calls C_Initialize
      * @param file_path the path to the PKCS#11 shared library
      * @param init_args flags to use for `C_Initialize`
      */
      Module(const std::string& file_path, C_InitializeArgs init_args = { nullptr, nullptr, nullptr, nullptr, static_cast< CK_FLAGS >(Flag::OsLockingOk), nullptr });

/* Microsoft Visual Studio <= 2013 does not support default generated move special member functions.
   Everything else we target should support it */
#if !defined( _MSC_VER ) || ( _MSC_VER >= 1900 )
      Module(Module&& other) = default;
      Module& operator=(Module&& other) = default;
#endif

      // Dtor calls C_Finalize(). A copy could be deleted while the origin still exists
      // Furthermore std::unique_ptr member -> not copyable
      Module(const Module& other) = delete;
      Module& operator=(const Module& other) = delete;

      /// Calls C_Finalize()
      ~Module() BOTAN_NOEXCEPT;

      /**
      * Reloads the module and reinitializes it
      * @param init_args flags to use for `C_Initialize`
      */
      void reload(C_InitializeArgs init_args = { nullptr, nullptr, nullptr, nullptr, static_cast< CK_FLAGS >(Flag::OsLockingOk), nullptr });

      inline LowLevel* operator->() const
         {
         return m_low_level.get();
         }

      /// @return general information about Cryptoki
      inline Info get_info() const
         {
         Info info;
         m_low_level->C_GetInfo(&info);
         return info;
         }

   private:
      const std::string m_file_path;
      FunctionListPtr m_func_list = nullptr;
      std::unique_ptr<Dynamically_Loaded_Library> m_library = nullptr;
      std::unique_ptr<LowLevel> m_low_level = nullptr;
   };

}
}

#endif
