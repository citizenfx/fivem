/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#pragma once

#include <thread>

#include <uv.h>

namespace net
{
class UvLoopHolder : public fwRefCountable
{
private:
	uv_loop_t m_loop;

	std::thread m_thread;

	bool m_shouldExit;

	std::string m_loopTag;

public:
	UvLoopHolder(const std::string& loopTag);

	virtual ~UvLoopHolder();

	inline uv_loop_t* GetLoop()
	{
		return &m_loop;
	}

	inline const std::string& GetLoopTag() const
	{
		return m_loopTag;
	}
};
}