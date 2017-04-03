#pragma once

#ifdef COMPILING_GTA_STREAMING_FIVE
#define STREAMING_EXPORT DLL_EXPORT
#else
#define STREAMING_EXPORT DLL_IMPORT
#endif

namespace streaming
{
	class STREAMING_EXPORT Manager
	{
	private:
		inline Manager() {}

	public:
		void RequestObject(uint32_t objectId, int flags);

		void ReleaseObject(uint32_t objectId);

		static Manager* GetInstance();
	};

	void STREAMING_EXPORT LoadObjectsNow(bool a1);

	uint32_t STREAMING_EXPORT GetStreamingIndexForName(const std::string& name);
}

#if 0
namespace rage
{
	class strStreamingModule
	{

	};

	class STREAMING_EXPORT strStreamingModuleMgr
	{
	private:
		inline strStreamingModuleMgr() {}

	public:
		strStreamingModule* GetModuleFromExtension(const char* extension);

		static strStreamingModuleMgr* GetInstance();
	};
}
#endif