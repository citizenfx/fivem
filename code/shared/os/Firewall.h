#pragma once

#include <iostream>
#include <string>
#include <string_view>

#ifdef _WIN32
#include <iwscapi.h>
#include <wscapi.h>
#include <versionhelpers.h>
#include <wrl.h>
#endif

namespace fx::os
{
class Firewall
{
public:
	static std::wstring GetActiveName(std::wstring_view defaultName = L"Unknown");
};
}
