/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#pragma once

#include <thread>

#include <uv.h>

#include <uvw.hpp>

#include <tbb/concurrent_queue.h>

namespace net
{
class UvLoopHolder : public fwRefCountable
{
private:
	std::shared_ptr<uvw::Loop> m_loop;

	std::shared_ptr<uvw::AsyncHandle> m_async;

	std::thread m_thread;

	bool m_shouldExit;

	std::string m_loopTag;

	tbb::concurrent_queue<std::function<void()>> m_functionQueue;

public:
	UvLoopHolder(const std::string& loopTag);

	virtual ~UvLoopHolder();

	void AssertThread();

	inline uv_loop_t* GetLoop()
	{
		return m_loop->raw();
	}

	inline const std::shared_ptr<uvw::Loop>& Get()
	{
		return m_loop;
	}

	inline const std::string& GetLoopTag() const
	{
		return m_loopTag;
	}

	inline void EnqueueCallback(const std::function<void()>& fn)
	{
		m_functionQueue.push(fn);
		m_async->send();
	}
};
}
