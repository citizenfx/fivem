#pragma once

#include <fiDevice.h>
#include <vector>

#include <StreamingTypes.h>

#ifdef COMPILING_GTA_STREAMING_NY
#include <zlib.h>

#define STREAMING_EXPORT DLL_EXPORT
#else
#define STREAMING_EXPORT DLL_IMPORT
#endif

namespace rage
{
	using ResourceFlags = uint32_t;
}

struct StreamHandle
{
	// +4
	uint32_t imgIndex : 8;
	uint32_t offset : 24;
	// +8
	uint32_t unk : 2;
	uint32_t size : 30;
};

struct CStreamingInfo
{
	// +0
	uint32_t realSize;
	// +4
	StreamHandle handle;
	// +12
	uint16_t unk2;
	// +14
	uint16_t flags;
	// +16
	uint32_t unk3;
	// +20
	uint16_t nextOnCd;
	// +22
	uint8_t moduleByIdx;
	uint8_t module;
};

class STREAMING_EXPORT CStreamingInfoManager
{
public:
	CStreamingInfo* Entries;

	// +36: used streaming memory, header blocks
	// +48: used streaming memory, virtual blocks

	uint32_t registerIMGFile(const char* name, uint32_t offset, uint32_t size, uint8_t imgNum, uint32_t index, uint32_t resourceType);

	static CStreamingInfoManager* GetInstance();
};

struct StreamingItem
{
	int handle;
	int blockSize;
	rage::fiDevice* device;
	uint32_t streamCounter;
	DWORD flags;
	int pad2;
	union
	{
		int fileStart;
		char* fileName;
	};

	void completeRequest();
};

extern StreamingItem** g_streamingItems;
extern int* g_nextStreamingItem;
extern uint32_t* g_streamMask;

struct StreamRequestPage
{
	void* buffer;
	uint32_t length;
	int pad;
};

struct StreamRequestPage2
{
	int pad;
	void* buffer;
	uint32_t length;
};

struct StreamRequest
{
	int itemIdx;
	int pad;
	StreamRequestPage pages[127];
	char pad2[8];
	uint32_t reqLength;
	uint32_t reqStart; // 1544

	void(*completionCB)(int userData, void* pageBuffer, uint32_t length, int); // 1548
	int completionCBData; // 1552
};

#ifdef COMPILING_GTA_STREAMING_NY
struct StreamRequestExt : public StreamRequest
{
	StreamingItem* item;
	bool isResource;
	OVERLAPPED overlapped;
	uint32_t reqRead;
	uint32_t pageNum;
	char* readBuffer;
	uint32_t handle;
	uint32_t lastAvailOut;
	rage::fiDevice* device;

	z_stream* strm;

	void NextPage();
};

struct StreamThread_GtaLocal
{
	StreamRequest requests[16];
	CRITICAL_SECTION counterLock; // 24896
	char pad[8]; // 24920
	uint32_t inRequestNum;
	int requestNum; // 24932
	int pendingRequests; // 24936
	HANDLE hSemaphore;
};

class StreamThread;

class StreamEvent
{
public:
	virtual void RunEvent(StreamThread* thread) = 0;
};

class NewRequestEvent : public StreamEvent
{
private:
	StreamRequest* m_request;

public:
	NewRequestEvent(StreamRequest* request)
		: m_request(request)
	{

	}

	virtual void RunEvent(StreamThread* thread);
};

class IOCompletedEvent : public StreamEvent
{
private:
	StreamRequestExt* m_request;

public:
	IOCompletedEvent(StreamRequestExt* request)
		: m_request(request)
	{

	}

	virtual void RunEvent(StreamThread* thread);
};

class StreamThread
{
private:
	std::vector<StreamRequestExt> m_requests;

	StreamThread_GtaLocal* m_local;

	HANDLE m_objects[64];

	LPOVERLAPPED m_curOverlapped;

	int m_threadID;

	z_stream m_streams[16];

	char m_readBuffers[16][131072];

	char m_readBuffersUsed[16];

	int m_curStream;

	int m_curReadBuffer;

public:
	inline void SetThreadID(int num) { m_threadID = num; }

	z_stream* GetStream();

	char* GetReadBuffer();

	void RunThread(StreamThread_GtaLocal* local);

	StreamEvent* WaitIdly();

	inline LPOVERLAPPED GetCurrentOverlapped()
	{
		return m_curOverlapped;
	}

	void QueueRequest(StreamRequestExt& request);

	void RemoveRequest(StreamRequestExt* request);

	void TriggerNextIO(StreamRequestExt* request);

	void QueueNativeRequest(StreamRequest& request);
};

StreamThread* GetStreamThread(int id);
#endif

#if 0
#include "ResourceCache.h"

void CSM_CreateStreamingFile(int index, const StreamingResource& entry);
#endif
