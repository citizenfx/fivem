/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include "UvLoopHolder.h"
#include "UvLoopManager.h"
#include "memdbgon.h"

namespace net
{
UvLoopHolder::UvLoopHolder(const std::string& loopTag)
	: m_shouldExit(false), m_loopTag(loopTag)
{
	// initialize the libuv loop
	m_loop = uvw::Loop::create();

	// assign our pointer to the loop
	m_loop->data(std::make_shared<void*>(this));

	m_async = m_loop->resource<uvw::AsyncHandle>();

	m_async->on<uvw::AsyncEvent>([this](const uvw::AsyncEvent& ev, uvw::AsyncHandle& handle)
	{
		std::function<void()> fn;

		while (m_functionQueue.try_pop(fn))
		{
			fn();
		}
	});

	// start the loop's runtime thread
	m_thread = std::thread([=] ()
	{
		SetThreadName(-1, const_cast<char*>(va(
#ifdef _WIN32
			"UV loop: %s"
#else
			// pthread names are limited in length, so we keep it short
			"luv_%s"
#endif
			, m_loopTag.c_str())));

		Instance<UvLoopManager>::Get()->SetCurrent(this);

		// start running the loop
		while (!m_shouldExit)
		{
			// execute the loop - this will probably return instantly before any events are added
			m_loop->run<uvw::Loop::Mode::DEFAULT>();

			// wait for a bit to not cause a full-load loop
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
		}

		// close the async handle from within the loop thread and run the
		// loop one final time so libuv can process the close callback
		// before the loop is destroyed
		m_async->close();
		m_loop->run<uvw::Loop::Mode::DEFAULT>();

		// clean up the libuv loop
		m_loop = {};
	});
}

UvLoopHolder::~UvLoopHolder()
{
	// mark the thread as needing to exit
	m_shouldExit = true;

	// stop the loop as soon as possible
	m_loop->stop();

	// wake the loop thread using the existing async handle so it can
	// observe m_shouldExit and shut down cleanly
	m_async->send();

	// wait for the thread to exit cleanly
	if (m_thread.joinable())
	{
		m_thread.join();
	}
}

void UvLoopHolder::AssertThread()
{
#if _DEBUG
	assert(std::this_thread::get_id() == m_thread.get_id());
#endif
}
}
