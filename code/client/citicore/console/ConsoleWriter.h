#pragma once

namespace console
{
class IWriter
{
public:
	virtual uint64_t Create(const std::string& path) = 0;

	virtual void Write(uint64_t handle, const void* data, size_t count) = 0;

	virtual void Close(uint64_t handle) = 0;
};
}

#ifndef COMPILING_CORE
#ifdef  _WIN32
inline void SetConsoleWriter(console::IWriter* writer)
{
	using TCoreFunc = decltype(&SetConsoleWriter);

	static TCoreFunc func;

	if (!func)
	{
		func = (TCoreFunc)GetProcAddress(GetModuleHandle(L"CoreRT.dll"), "SetConsoleWriter");
	}

	return (func) ? func(writer) : (void)nullptr;
}
#else
extern "C" void SetConsoleWriter(console::IWriter* writer);
#endif
#else
extern "C" DLL_EXPORT void SetConsoleWriter(console::IWriter* writer);
#endif
