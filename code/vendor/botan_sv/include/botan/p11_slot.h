/*
* PKCS#11 Slot
* (C) 2016 Daniel Neus
* (C) 2016 Philipp Weber
*
* Botan is released under the Simplified BSD License (see license.txt)
*/

#ifndef BOTAN_P11_SLOT_H_
#define BOTAN_P11_SLOT_H_

#include <string>
#include <vector>
#include <functional>

#include <botan/p11_module.h>

namespace Botan {
namespace PKCS11 {

/// Represents a PKCS#11 Slot, i.e., a card reader
class BOTAN_PUBLIC_API(2,0) Slot final
   {
   public:
      /**
      * @param module the PKCS#11 module to use
      * @param slot_id the slot id to use
      */
      Slot(Module& module, SlotId slot_id);

      /// @return a reference to the module that is used
      inline Module& module() const
         {
         return m_module;
         }

      /// @return the slot id
      inline SlotId slot_id() const
         {
         return m_slot_id;
         }

      /**
      * Get available slots
      * @param module the module to use
      * @param token_present true if only slots with attached tokens should be returned, false for all slots
      * @return a list of available slots (calls C_GetSlotList)
      */
      static std::vector<SlotId> get_available_slots(Module& module, bool token_present);

      /// @return information about the slot (`C_GetSlotInfo`)
      SlotInfo get_slot_info() const;

      /// Obtains a list of mechanism types supported by the slot (`C_GetMechanismList`)
      std::vector<MechanismType> get_mechanism_list() const;

      /// Obtains information about a particular mechanism possibly supported by a slot (`C_GetMechanismInfo`)
      MechanismInfo get_mechanism_info(MechanismType mechanism_type) const;

      /// Obtains information about a particular token in the system (`C_GetTokenInfo`)
      TokenInfo get_token_info() const;

      /**
      * Calls `C_InitToken` to initialize the token
      * @param label the label for the token (must not exceed 32 bytes according to PKCS#11)
      * @param so_pin the PIN of the security officer
      */
      void initialize(const std::string& label, const secure_string& so_pin) const;

   private:
      const std::reference_wrapper<Module> m_module;
      const SlotId m_slot_id;
   };

}
}

#endif
