#pragma once

namespace ServerConsoleHost
{
class ConHostSvImpl;

class
#ifdef COMPILING_CONHOST_SERVER
DLL_EXPORT
#else
DLL_IMPORT
#endif
ConHostSv
{
public:
	ConHostSv();

	virtual ~ConHostSv();

	void Run(std::function<bool()>&& fn);

	void* GetPlatformWindowHandle();

private:
	std::unique_ptr<ConHostSvImpl> m_impl;
};
}
