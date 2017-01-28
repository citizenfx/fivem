/**
 * XMSS_WOTS_Verification_Operation.h
 * (C) 2016 Matthias Gierlings
 *
 * Botan is released under the Simplified BSD License (see license.txt)
 **/

#ifndef BOTAN_XMSS_WOTS_VERIFICATION_OPERATION_H__
#define BOTAN_XMSS_WOTS_VERIFICATION_OPERATION_H__

#include <cstddef>
#include <iterator>
#include <botan/types.h>
#include <botan/pk_ops.h>
#include <botan/internal/xmss_wots_addressed_publickey.h>
#include <botan/internal/xmss_wots_common_ops.h>

namespace Botan {

/**
 * Provides signature verification capabilities for Winternitz One Time
 * Signatures used in Extended Merkle Tree Signatures (XMSS).
 *
 * This operation is not intended for stand-alone use and thus not registered
 * in the Botan algorithm registry.
 **/
class XMSS_WOTS_Verification_Operation
   : public virtual PK_Ops::Verification,
     public XMSS_WOTS_Common_Ops
   {
   public:
      XMSS_WOTS_Verification_Operation(
         const XMSS_WOTS_Addressed_PublicKey& public_key);

      virtual ~XMSS_WOTS_Verification_Operation() {}

      virtual bool is_valid_signature(const uint8_t sig[],
                                      size_t sig_len) override;

      void update(const uint8_t msg[], size_t msg_len) override;

   private:
      XMSS_WOTS_Addressed_PublicKey m_pub_key;
      secure_vector<uint8_t> m_msg_buf;
   };

}

#endif
