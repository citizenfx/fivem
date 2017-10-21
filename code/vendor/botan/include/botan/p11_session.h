/*
* PKCS#11 Session
* (C) 2016 Daniel Neus, Sirrix AG
* (C) 2016 Philipp Weber, Sirrix AG
*
* Botan is released under the Simplified BSD License (see license.txt)
*/

#ifndef BOTAN_P11_SESSION_H_
#define BOTAN_P11_SESSION_H_

#include <botan/p11_slot.h>

#include <utility>

namespace Botan {
namespace PKCS11 {
class Module;

/// Represents a PKCS#11 session
class BOTAN_PUBLIC_API(2,0) Session final
   {
   public:
      /**
      * @param slot the slot to use
      * @param read_only true if the session should be read only, false to create a read-write session
      */
      Session(Slot& slot, bool read_only);

      /**
      * @param slot the slot to use
      * @param flags the flags to use for the session. Remark: Flag::SerialSession is mandatory
      * @param callback_data application-defined pointer to be passed to the notification callback
      * @param notify_callback address of the notification callback function
      */
      Session(Slot& slot, Flags flags, VoidPtr callback_data, Notify notify_callback);

      /// Takes ownership of a session
      Session(Slot& slot, SessionHandle handle);

/* Microsoft Visual Studio <= 2013 does not support default generated move special member functions.
   Everything else we target should support it */
#if !defined( _MSC_VER ) || ( _MSC_VER >= 1900 )
      Session(Session&& other) = default;
      Session& operator=(Session&& other) = default;
#endif

      // Dtor calls C_CloseSession() and eventually C_Logout. A copy could close the session while the origin still exists
      Session(const Session& other) = delete;
      Session& operator=(const Session& other) = delete;

      /// Logout user and close the session on destruction
      ~Session() BOTAN_NOEXCEPT;

      /// @return a reference to the slot
      inline const Slot& slot() const
         {
         return m_slot;
         }

      /// @return the session handle of this session
      inline SessionHandle handle() const
         {
         return m_handle;
         }

      /// @return a reference to the used module
      inline Module& module() const
         {
         return m_slot.module();
         }

      /// @return the released session handle
      SessionHandle release();

      /**
      * Login to this session
      * @param userType the user type to use for the login
      * @param pin the PIN of the user
      */
      void login(UserType userType, const secure_string& pin);

      /// Logout from this session
      void logoff();

      /// @return information about this session
      SessionInfo get_info() const;

      /// Calls `C_SetPIN` to change the PIN using the old PIN (requires a logged in session)
      void set_pin(const secure_string& old_pin, const secure_string& new_pin) const;

      /// Calls `C_InitPIN` to change or initialize the PIN using the SO_PIN (requires a logged in session)
      void init_pin(const secure_string& new_pin);

   private:
      const Slot& m_slot;
      SessionHandle m_handle;
      bool m_logged_in;
   };

}
}

#endif
