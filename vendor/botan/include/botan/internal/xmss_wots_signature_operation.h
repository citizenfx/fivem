/**
 * XMSS WOTS Signature Operation
 * (C) 2016 Matthias Gierlings
 *
 * Botan is released under the Simplified BSD License (see license.txt)
 **/

#ifndef BOTAN_XMSS_WOTS_SIGNATURE_OPERATION_H__
#define BOTAN_XMSS_WOTS_SIGNATURE_OPERATION_H__

#include <cstddef>
#include <iterator>
#include <botan/assert.h>
#include <botan/types.h>
#include <botan/pk_ops.h>
#include <botan/internal/xmss_wots_addressed_privatekey.h>
#include <botan/internal/xmss_wots_common_ops.h>

namespace Botan {

/**
 * Signature generation operation for Winternitz One Time Signatures for use
 * in Extended Hash-Based Signatures (XMSS).
 *
 * This operation is not intended for stand-alone use and thus not registered
 * in the Botan algorithm registry.
 ***/
class XMSS_WOTS_Signature_Operation : public virtual PK_Ops::Signature,
                                      public XMSS_WOTS_Common_Ops
   {
   public:
      XMSS_WOTS_Signature_Operation(
         const XMSS_WOTS_Addressed_PrivateKey& private_key);

      virtual ~XMSS_WOTS_Signature_Operation() {}

      /**
       * Creates a XMSS WOTS signature for the message provided through call
       * to update(). XMSS wots only supports one message part and a fixed
       * message size of "n" bytes where "n" equals the element size of
       * the chosen XMSS WOTS signature method. The random number generator
       * argument is supplied for interface compatibility and remains unused.
       *
       * @return serialized Winternitz One Time Signature.
       **/
      secure_vector<uint8_t> sign(RandomNumberGenerator&) override;

      void update(const uint8_t msg[], size_t msg_len) override;

   private:
      wots_keysig_t sign(const secure_vector<uint8_t>& msg,
                         const wots_keysig_t& priv_key,
                         XMSS_Address& adrs,
                         const secure_vector<uint8_t>& seed);
      XMSS_WOTS_Addressed_PrivateKey m_priv_key;
      secure_vector<uint8_t> m_msg_buf;
   };

}

#endif

