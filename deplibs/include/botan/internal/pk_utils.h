/*
* Public Key Algos Utility Header
* (C) 2015 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/

#ifndef BOTAN_PK_UTILS_H__
#define BOTAN_PK_UTILS_H__

#include <botan/internal/algo_registry.h>
#include <botan/internal/pk_ops_impl.h>
#include <botan/numthry.h>
#include <algorithm>

namespace Botan {

template<typename OP, typename T>
OP* make_pk_op(const typename T::Spec& spec)
   {
   if(auto* key = dynamic_cast<const typename T::Key_Type*>(&spec.key()))
      return new T(*key, spec.padding());
   return nullptr;
   }

#define BOTAN_REGISTER_PK_OP(T, NAME, TYPE) BOTAN_REGISTER_NAMED_T(T, NAME, TYPE, (make_pk_op<T, TYPE>))

#define BOTAN_REGISTER_PK_ENCRYPTION_OP(NAME, TYPE) BOTAN_REGISTER_PK_OP(PK_Ops::Encryption, NAME, TYPE)
#define BOTAN_REGISTER_PK_DECRYPTION_OP(NAME, TYPE) BOTAN_REGISTER_PK_OP(PK_Ops::Decryption, NAME, TYPE)
#define BOTAN_REGISTER_PK_SIGNATURE_OP(NAME, TYPE) BOTAN_REGISTER_PK_OP(PK_Ops::Signature, NAME, TYPE)
#define BOTAN_REGISTER_PK_VERIFY_OP(NAME, TYPE) BOTAN_REGISTER_PK_OP(PK_Ops::Verification, NAME, TYPE)
#define BOTAN_REGISTER_PK_KEY_AGREE_OP(NAME, TYPE) BOTAN_REGISTER_PK_OP(PK_Ops::Key_Agreement, NAME, TYPE)

}

#endif
