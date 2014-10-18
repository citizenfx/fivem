/*
* PBE
* (C) 1999-2007 Jack Lloyd
*
* Distributed under the terms of the Botan license
*/

#ifndef BOTAN_PBE_BASE_H__
#define BOTAN_PBE_BASE_H__

#include <botan/asn1_oid.h>
#include <botan/data_src.h>
#include <botan/filter.h>
#include <botan/rng.h>

namespace Botan {

/**
* Password Based Encryption (PBE) Filter.
*/
class BOTAN_DLL PBE : public Filter
   {
   public:
      /**
      * DER encode the params (the number of iterations and the salt value)
      * @return encoded params
      */
      virtual std::vector<byte> encode_params() const = 0;

      /**
      * Get this PBE's OID.
      * @return object identifier
      */
      virtual OID get_oid() const = 0;
   };

}

#endif
