#pragma once

#ifdef _WIN32
inline const char* FormatModuleAddress(void* address)
{
	const char* moduleBaseString = "";
	HMODULE module;

	if (GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT, (LPCWSTR)address, &module))
	{
		wchar_t filename[MAX_PATH * 2] = { 0 };
		GetModuleFileNameW(module, filename, std::size(filename));

		wchar_t* basename = filename;
		if (auto pos = wcsrchr(filename, '\\'))
		{
			basename = pos + 1;
		}

		return va("%s+%X", ToNarrow(basename), (char*)address - (char*)module);
	}

	return va("%p", address);
}
#else
inline const char* FormatModuleAddress(void* address)
{
	return va("%p", address);
}
#endif
