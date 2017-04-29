/*
* OID Registry
* (C) 1999-2007 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/

#ifndef BOTAN_OIDS_H__
#define BOTAN_OIDS_H__

#include <botan/asn1_oid.h>

namespace Botan {

namespace OIDS {

/**
* Resolve an OID
* @param oid the OID to look up
* @return name associated with this OID
*/
BOTAN_DLL std::string lookup(const OID& oid);

/**
* Find the OID to a name. The lookup will be performed in the
* general OID section of the configuration.
* @param name the name to resolve
* @return OID associated with the specified name
*/
BOTAN_DLL OID lookup(const std::string& name);

/**
* See if an OID exists in the internal table.
* @param oid the oid to check for
* @return true if the oid is registered
*/
inline bool have_oid(const std::string& oid)
   {
   return (lookup(oid).empty() == false);
   }

}

}

#endif
