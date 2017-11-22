/*
 * This file is part of FiveM (https://fivem.net), created by CitizenFX (https://github.com/citizenfx)
 *
 * See root directory for information regarding LICENSE and other instruction.
 * 
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

	void AssertThread();

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
