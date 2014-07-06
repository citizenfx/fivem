#pragma once

#include <scrThread.h>
#include <sysAllocator.h>

class GAMESPEC_EXPORT CMissionCleanupEntry
{
private:
	uint8_t m_type;
	uint32_t m_data;
	GtaThread* m_thread;
	uint32_t m_unk1;
	uint32_t m_unkAlways0;
	uint8_t pad[24];

public:
	CMissionCleanupEntry();

	void Reset();
};

class GAMESPEC_EXPORT_VMT CMissionCleanup : public rage::sysUseAllocator
{
private:
	CMissionCleanupEntry m_scriptEntries[256];
	int m_unknown;
	CMissionCleanupEntry m_dependentEntries[200];

public:
	CMissionCleanup();

	virtual ~CMissionCleanup();

	virtual void CleanUp(GtaThread* scriptThread);

	void Initialize();
};