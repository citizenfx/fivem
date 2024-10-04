#pragma once

namespace win32
{
inline std::string FormatMessage(DWORD errorCode)
{
	wchar_t* errorText = nullptr;

	auto flags = FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_MAX_WIDTH_MASK | FORMAT_MESSAGE_ALLOCATE_BUFFER;

	if (::FormatMessageW(flags, nullptr, errorCode, MAKELANGID(LANG_ENGLISH, SUBLANG_DEFAULT), (wchar_t*)&errorText, 0, nullptr) == 0)
	{
		::FormatMessageW(flags, nullptr, errorCode, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (wchar_t*)&errorText, 0, nullptr);
	}

	std::string retval;

	if (errorText)
	{
		retval = ToNarrow(errorText);
		::LocalFree(errorText);
	}

	return retval;
}
}
