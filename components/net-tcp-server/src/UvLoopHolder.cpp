/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include "UvLoopHolder.h"
#include "memdbgon.h"

namespace net
{
UvLoopHolder::UvLoopHolder(const std::string& loopTag)
	: m_shouldExit(false), m_loopTag(loopTag)
{
	// initialize the libuv loop
	uv_loop_init(&m_loop);

	// assign our pointer to the loop
	m_loop.data = this;

	// start the loop's runtime thread
	m_thread = std::thread([=] ()
	{
		// start running the loop
		while (!m_shouldExit)
		{
			// execute the loop - this will probably return instantly before any events are added
			uv_run(&m_loop, UV_RUN_DEFAULT);

			// wait for a bit to not cause a full-load loop
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
		}

		// clean up the libuv loop
		uv_loop_close(&m_loop);
	});
}

UvLoopHolder::~UvLoopHolder()
{
	// mark the thread as needing to exit
	m_shouldExit = true;

	// stop the loop as soon as possible
	uv_stop(&m_loop);

	// signal the loop so it can get triggered
	uv_async_t async;
	
	uv_async_init(&m_loop, &async, [] (uv_async_t*)
	{

	});

	uv_async_send(&async);

	// wait for the thread to exit cleanly
	if (m_thread.joinable())
	{
		m_thread.join();
	}

	// clean up the async
	uv_close(reinterpret_cast<uv_handle_t*>(&async), nullptr);
}
}