/*
* PEM Encoding/Decoding
* (C) 1999-2007 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/

#ifndef BOTAN_PEM_H__
#define BOTAN_PEM_H__

#include <botan/data_src.h>

namespace Botan {

namespace PEM_Code {

/**
* Encode some binary data in PEM format
*/
BOTAN_DLL std::string encode(const byte data[],
                             size_t data_len,
                             const std::string& label,
                             size_t line_width = 64);

/**
* Encode some binary data in PEM format
*/
inline std::string encode(const std::vector<byte>& data,
                          const std::string& label,
                          size_t line_width = 64)
   {
   return encode(&data[0], data.size(), label, line_width);
   }

/**
* Encode some binary data in PEM format
*/
inline std::string encode(const secure_vector<byte>& data,
                          const std::string& label,
                          size_t line_width = 64)
   {
   return encode(&data[0], data.size(), label, line_width);
   }

/**
* Decode PEM data
* @param pem a datasource containing PEM encoded data
* @param label is set to the PEM label found for later inspection
*/
BOTAN_DLL secure_vector<byte> decode(DataSource& pem,
                                     std::string& label);

/**
* Decode PEM data
* @param pem a string containing PEM encoded data
* @param label is set to the PEM label found for later inspection
*/
BOTAN_DLL secure_vector<byte> decode(const std::string& pem,
                                     std::string& label);

/**
* Decode PEM data
* @param pem a datasource containing PEM encoded data
* @param label is what we expect the label to be
*/
BOTAN_DLL secure_vector<byte> decode_check_label(
   DataSource& pem,
   const std::string& label);

/**
* Decode PEM data
* @param pem a string containing PEM encoded data
* @param label is what we expect the label to be
*/
BOTAN_DLL secure_vector<byte> decode_check_label(
   const std::string& pem,
   const std::string& label);

/**
* Heuristic test for PEM data.
*/
BOTAN_DLL bool matches(DataSource& source,
                       const std::string& extra = "",
                       size_t search_range = 4096);

}

}

#endif
