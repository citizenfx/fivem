/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include <terminal/internal/WorkerThread.h>

#include <mmsystem.h>

namespace terminal
{
WorkerThread::WorkerThread()
	: m_residualTimeout(0)
{
	m_threadHandle = std::thread([=] ()
	{
		ThreadFunc();
	});

	m_threadHandle.detach();
}

void WorkerThread::ThreadFunc()
{
	std::vector<HANDLE> waitHandles;
	std::vector<HandleFn> waitHandleCallbacks;

	// add ourselves as wait handle to make sure the wait handle list always contains one handle
	
	// to do this, we need to duplicate the GetCurrentThread result, as waiting doesn't like the NtCurrentThread pseudo-handle
	HANDLE ourHandle;
	DuplicateHandle(GetCurrentProcess(), GetCurrentThread(), GetCurrentProcess(), &ourHandle, SYNCHRONIZE, FALSE, 0);

	m_waitHandleList.push_back(std::make_pair(ourHandle, [] (HANDLE)
	{
		FatalError("Wait call satisfied our thread handle - this shouldn't be possible.");
	}));

	m_residualTimeout = 50;

	SetThreadName(-1, "Terminal Worker");

	while (true)
	{
		// lock the wait handle list
		m_waitHandleListMutex.lock();

		// set up the wait handle list
		waitHandles.resize(m_waitHandleList.size());
		waitHandleCallbacks.resize(m_waitHandleList.size());

		// set up the temporary arrays
		int i = 0;

		for (auto&& pair : m_waitHandleList)
		{
			waitHandles[i] = pair.first;
			waitHandleCallbacks[i] = pair.second;

			i++;
		}

		// unlock
		m_waitHandleListMutex.unlock();

		// set up a wait for the base timeout
		DWORD preWaitTime = timeGetTime(); // get base time

		// TODO: support >64 objects if ever needed
		DWORD result = WaitForMultipleObjects(waitHandles.size(), &waitHandles[0], FALSE, m_residualTimeout);

		// reduce the residual timeout
		m_residualTimeout -= timeGetTime() - preWaitTime;

		// invoke tick callbacks if the residual timeout is at or below 0 (i.e. it expired)
		if (m_residualTimeout <= 0)
		{
			for (auto&& cb : m_tickCallbackList)
			{
				cb();
			}

			m_residualTimeout = 50;
		}

		// check the wait result
		if (result >= WAIT_OBJECT_0 && result <= (WAIT_OBJECT_0 + waitHandles.size()))
		{
			int handleIndex = (result - WAIT_OBJECT_0);

			waitHandleCallbacks[handleIndex](waitHandles[handleIndex]);
		}

		// process removal of wait handles
		for (auto&& handle : m_removeHandleList)
		{
			m_waitHandleListMutex.lock();

			for (auto& it = m_waitHandleList.begin(); it != m_waitHandleList.end(); it++)
			{
				if (it->first == handle)
				{
					m_waitHandleList.erase(it);
					break;
				}
			}

			m_waitHandleListMutex.unlock();
		}
	}
}

void WorkerThread::RegisterTickCallback(std::function<void()> callback)
{
	m_tickCallbackList.push_back(callback);
}

void WorkerThread::RegisterWaitHandle(HANDLE handle, HandleFn callback)
{
	m_waitHandleListMutex.lock();

	m_waitHandleList.push_back(std::make_pair(handle, callback));

	m_waitHandleListMutex.unlock();
}

void WorkerThread::UnregisterWaitHandle(HANDLE handle)
{
	m_removeHandleList.push_back(handle);
}

static InitFunction initFunction([] ()
{
	Instance<WorkerThread>::Set(new WorkerThread());
}, -500);
}