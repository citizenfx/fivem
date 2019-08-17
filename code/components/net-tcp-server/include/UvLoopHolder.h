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

namespace net
{
class UvLoopHolder : public fwRefCountable
{
private:
	std::shared_ptr<uvw::Loop> m_loop;

	std::thread m_thread;

	bool m_shouldExit;

	std::string m_loopTag;

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
};
}
