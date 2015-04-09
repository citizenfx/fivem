/**
 * (C) 2014 cryptosource GmbH
 * (C) 2014 Falko Strenzke fstrenzke@cryptosource.de
 *
 * Botan is released under the Simplified BSD License (see license.txt)
 */

#ifndef BOTAN_GF2M_ROOTFIND_DCMP_H__
#define BOTAN_GF2M_ROOTFIND_DCMP_H__

#include <botan/polyn_gf2m.h>

namespace Botan {

/**
* Find the roots of a polynomial over GF(2^m) using the method by Federenko
* et al.
*/
secure_vector<gf2m> find_roots_gf2m_decomp(const polyn_gf2m & polyn,
                                           u32bit code_length);

}

#endif
