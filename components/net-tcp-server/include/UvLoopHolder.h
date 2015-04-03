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
class UvLoopHolder
{
private:
	uv_loop_t m_loop;

	std::thread m_thread;

	bool m_shouldExit;

public:
	UvLoopHolder();

	virtual ~UvLoopHolder();

	inline uv_loop_t* GetLoop()
	{
		return &m_loop;
	}
};
}