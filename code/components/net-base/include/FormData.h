#pragma once

#include <map>
#include <string>

#include "ComponentExport.h"

namespace net
{
/// <summary>
/// This function is parsing a POST string into a map.
/// The posts strings have the following format key=value&key2=value2
/// </summary>
COMPONENT_EXPORT(NET_BASE) std::map<std::string, std::string> DecodeFormData(const std::string_view& data);

/// <summary>
/// This function is decoding a URL encoded string.
/// </summary>
COMPONENT_EXPORT(NET_BASE) bool UrlDecode(const std::string_view& in, std::string& out, bool replacePlus = true);
}
