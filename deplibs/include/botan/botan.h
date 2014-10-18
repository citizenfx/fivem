/*
* A vague catch all include file for Botan
* (C) 1999-2007 Jack Lloyd
*
* Distributed under the terms of the Botan license
*/

#ifndef BOTAN_BOTAN_H__
#define BOTAN_BOTAN_H__

#include <botan/init.h>
#include <botan/lookup.h>
#include <botan/libstate.h>
#include <botan/version.h>
#include <botan/parsing.h>

#include <botan/rng.h>

#if defined(BOTAN_HAS_AUTO_SEEDING_RNG)
  #include <botan/auto_rng.h>
#endif

#endif
