/*
* A vague catch all include file for Botan
* (C) 1999-2007 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/

#ifndef BOTAN_BOTAN_H__
#define BOTAN_BOTAN_H__

#include <botan/lookup.h>
#include <botan/version.h>
#include <botan/parsing.h>

#include <botan/rng.h>

#if defined(BOTAN_HAS_AUTO_SEEDING_RNG)
  #include <botan/auto_rng.h>
#endif

#endif
