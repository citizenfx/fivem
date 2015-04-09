/*
* (C) 2015 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/

#ifndef BOTAN_UTIL_FS_H__
#define BOTAN_UTIL_FS_H__

#include <botan/types.h>
#include <vector>
#include <string>

namespace Botan {

BOTAN_DLL std::vector<std::string>
list_all_readable_files_in_or_under(const std::string& dir);

}

#endif
