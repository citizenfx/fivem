#include "StdInc.h"
#include "Streaming.h"

void NewRequestEvent::RunEvent(StreamThread* thread)
{
	StreamRequest* req = m_request;

	if (req->itemIdx != -1)
	{
		req->itemIdx &= (g_streamMask - 1);
	}

	StreamingItem* item = &g_streamingItems[req->itemIdx];

	if (req->reqLength)
	{
		uint32_t handle = item->device->getParentHandle();
		uint64_t fileStart;

		if (handle == -1)
		{
			handle = item->device->openBulk(item->fileName, &fileStart);
		}
		else
		{
			fileStart = item->fileStart;
		}

		if (handle == -1)
		{
			trace("handle is -1, completed request instantly?\n");

			item->completeRequest();
			return;
		}

		StreamRequestExt nreq;
		memcpy(&nreq, req, sizeof(StreamRequest));
		nreq.item = item;
		nreq.pageNum = 0xFFFFFFFF;
		nreq.readBuffer = nullptr;
		nreq.handle = handle;
		nreq.reqRead = (uint32_t)fileStart;
		nreq.readBuffer = nullptr;
		nreq.device = item->device;

		memset(&nreq.overlapped, 0, sizeof(OVERLAPPED));
		nreq.overlapped.hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

		if (item->flags & (1 << 30))
		{
			// resource?
			nreq.isResource = true;

			//memset(&nreq.strm, 0, sizeof(nreq.strm));
			nreq.strm = thread->GetStream();
			nreq.readBuffer = thread->GetReadBuffer();

			inflateReset(nreq.strm);

			//nreq.readBuffer = new char[65536];
		}
		else
		{
			nreq.isResource = false;
		}

		nreq.NextPage();

		thread->QueueRequest(nreq);
	}
}

void StreamRequestExt::NextPage()
{
	if (pageNum == -1)
	{
		pageNum = 0;
	}
	else
	{
		pageNum++;
	}

	if (isResource)
	{
		strm->avail_out = pages[pageNum].length;
		strm->next_out = (uint8_t*)pages[pageNum].buffer;
	}
}

void IOCompletedEvent::RunEvent(StreamThread* thread)
{
	uint32_t bytesRead = m_request->overlapped.InternalHigh;
	bool reqCompleted = false;

	if (m_request->isResource)
	{
		z_stream* strm = m_request->strm;
		strm->avail_in = bytesRead;
		strm->next_in = (uint8_t*)m_request->readBuffer;

		m_request->reqRead += bytesRead;

		int res = Z_OK;

		while (strm->avail_in && res == Z_OK && m_request->pageNum < m_request->reqLength)
		{
			res = inflate(strm, 0);

			if (strm->avail_out == 0)
			{
				m_request->NextPage();
			}
		}

		reqCompleted = true;
	}
	else
	{
		m_request->reqRead += bytesRead;

		m_request->NextPage();

		reqCompleted = true;
	}

	if (reqCompleted)
	{
		if (m_request->pageNum < m_request->reqLength)
		{
			thread->TriggerNextIO(m_request);
		}
		else
		{
			if (m_request->item->device->getParentHandle() == -1)
			{
				m_request->item->device->closeBulk(m_request->handle);
			}

			m_request->item->completeRequest();

			if (m_request->isResource)
			{
				m_request->strm->avail_out = -1;
			}

			if (m_request->completionCB)
			{
				m_request->completionCB(m_request->completionCBData, m_request->pages[0].buffer, m_request->reqLength, 0);
			}

			thread->RemoveRequest(m_request);
		}
	}
}

void StreamThread::TriggerNextIO(StreamRequestExt* request)
{
	rage::fiDevice* device = request->device;
	uint32_t handle = request->handle;

	m_curOverlapped = &request->overlapped;

	uint64_t fileStart = (request->reqStart + request->reqRead);
	uint64_t flags = 0x8000000000000000 | ((m_threadID == 1) ? (0x4000000000000000) : 0);

	StreamRequestPage* page = &request->pages[request->pageNum];

	uint32_t len;

	if (request->isResource)
	{
		len = device->readBulk(handle, fileStart | flags, request->readBuffer, min(page->length, (uint32_t)131072));
		request->lastAvailOut = request->strm->avail_in;
	}
	else
	{
		len = device->readBulk(handle, fileStart | flags, page->buffer, page->length);
	}

	if (len != -1)
	{
		IOCompletedEvent compEvent(request);
		compEvent.RunEvent(this);

		return;
	}
}

void StreamThread::QueueRequest(StreamRequestExt& request)
{
	m_requests.push_back(request);

	TriggerNextIO(&m_requests[m_requests.size() - 1]);
}

void StreamThread::RemoveRequest(StreamRequestExt* request)
{
	for (int i = 0; i < _countof(m_readBuffers); i++)
	{
		if (request->readBuffer == m_readBuffers[i])
		{
			m_readBuffersUsed[i] = false;
			break;
		}
	}

	uint32_t idx = ((request - &m_requests[0]));

	m_requests.erase(m_requests.begin() + idx);
}

void StreamingItem::completeRequest()
{
	InterlockedDecrement(&this->streamCounter);
	InterlockedDecrement((LONG*)0x184A25C); // stream wait counter
}

void StreamThread::QueueNativeRequest(StreamRequest& request)
{
	if (m_local->counterLock.DebugInfo)
	{
		EnterCriticalSection(&m_local->counterLock);
	}

	m_local->pendingRequests++;

	InterlockedIncrement(&m_local->inRequestNum);

	if (m_local->inRequestNum == 16)
	{
		m_local->inRequestNum = 0;
	}

	m_local->requests[m_local->inRequestNum] = request;

	ReleaseSemaphore(m_local->hSemaphore, 1, nullptr);

	if (m_local->counterLock.DebugInfo)
	{
		LeaveCriticalSection(&m_local->counterLock);
	}
}

StreamEvent* StreamThread::WaitIdly()
{
	// global sema
	int objectCount = 1;
	m_objects[0] = m_local->hSemaphore;

	// add request handles
	for (auto& req : m_requests)
	{
		if (req.overlapped.hEvent)
		{
			m_objects[objectCount] = req.overlapped.hEvent;
			objectCount++;
		}
	}

	// w8 b8 m8
	DWORD result = WaitForMultipleObjects(objectCount, m_objects, FALSE, INFINITE);

	if (result == WAIT_OBJECT_0)
	{
		// streaming request added by GTA, get it
		if (m_local->counterLock.DebugInfo)
		{
			EnterCriticalSection(&m_local->counterLock);
		}

		m_local->requestNum++;
		if (m_local->requestNum == 16)
		{
			m_local->requestNum = 0;
		}

		m_local->pendingRequests--;

		StreamRequest* request = &m_local->requests[m_local->requestNum];

		if (m_local->counterLock.DebugInfo)
		{
			LeaveCriticalSection(&m_local->counterLock);
		}

		if (!request->completionCB)
		{
			return nullptr;
		}

		return new NewRequestEvent(request);
	}
	else if (result >= (WAIT_OBJECT_0 + 1))
	{
		int reqId = result - (WAIT_OBJECT_0 + 1);
		StreamRequestExt* request = &m_requests[reqId];

		return new IOCompletedEvent(request);
	}

	return nullptr;
}

void StreamThread::RunThread(StreamThread_GtaLocal* local)
{
	m_requests.reserve(8);
	m_local = local;

	for (int i = 0; i < 16; i++)
	{
		inflateInit(&m_streams[i]);
		m_streams[i].avail_out = -1;
	}

	StreamEvent* ev = WaitIdly();

	while (ev)
	{
		ev->RunEvent(this);
		delete ev;

		ev = WaitIdly();
	}
}

char* StreamThread::GetReadBuffer()
{
	auto increment = [&] ()
	{
		m_curReadBuffer++;

		if (m_curReadBuffer >= 16)
		{
			m_curReadBuffer = 0;
		}
	};

	increment();

	while (m_readBuffersUsed[m_curReadBuffer])
	{
		increment();
	}

	m_readBuffersUsed[m_curReadBuffer] = true;

	return m_readBuffers[m_curReadBuffer];
}

z_stream* StreamThread::GetStream()
{
	auto increment = [&] ()
	{
		m_curStream++;

		if (m_curStream >= 16)
		{
			m_curStream = 0;
		}
	};

	increment();

	while (m_streams[m_curStream].avail_out != -1)
	{
		increment();
	}

	m_streams[m_curStream].avail_out = 0;

	return &m_streams[m_curStream];
}

static StreamThread_GtaLocal* streamLocal = (StreamThread_GtaLocal*)0x198F6C8;
static StreamThread modLocal[2];

StreamThread* GetStreamThread(int id)
{
	return &modLocal[id];
}

LPOVERLAPPED StreamWorker_GetOverlapped(int streamThreadNum)
{
	return modLocal[streamThreadNum].GetCurrentOverlapped();
}

void StreamWorker_Thread(int threadNum)
{
	StreamThread_GtaLocal* local = &streamLocal[threadNum];

	modLocal[threadNum].SetThreadID(threadNum);
	modLocal[threadNum].RunThread(local);
}