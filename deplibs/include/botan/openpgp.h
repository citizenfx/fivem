/*
* OpenPGP Codec
* (C) 1999-2007 Jack Lloyd
*
* Distributed under the terms of the Botan license
*/

#ifndef BOTAN_OPENPGP_CODEC_H__
#define BOTAN_OPENPGP_CODEC_H__

#include <botan/data_src.h>
#include <string>
#include <map>

namespace Botan {

/**
* @param input the input data
* @param length length of input in bytes
* @param label the human-readable label
* @param headers a set of key/value pairs included in the header
*/
BOTAN_DLL std::string PGP_encode(
   const byte input[],
   size_t length,
   const std::string& label,
   const std::map<std::string, std::string>& headers);

/**
* @param input the input data
* @param length length of input in bytes
* @param label the human-readable label
*/
BOTAN_DLL std::string PGP_encode(
   const byte input[],
   size_t length,
   const std::string& label);

/**
* @param source the input source
* @param label is set to the human-readable label
* @param headers is set to any headers
* @return decoded output as raw binary
*/
BOTAN_DLL secure_vector<byte> PGP_decode(
   DataSource& source,
   std::string& label,
   std::map<std::string, std::string>& headers);

/**
* @param source the input source
* @param label is set to the human-readable label
* @return decoded output as raw binary
*/
BOTAN_DLL secure_vector<byte> PGP_decode(
   DataSource& source,
   std::string& label);

}

#endif
