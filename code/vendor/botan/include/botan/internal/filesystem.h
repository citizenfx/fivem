/*
* (C) 2015 Jack Lloyd
* (C) 2015 Simon Warta (Kullo GmbH)
*
* Botan is released under the Simplified BSD License (see license.txt)
*/

#ifndef BOTAN_UTIL_FILESYSTEM_H__
#define BOTAN_UTIL_FILESYSTEM_H__

#include <botan/types.h>
#include <vector>
#include <string>

namespace Botan {

BOTAN_DLL std::vector<std::string> get_files_recursive(const std::string& dir);

}

#endif
