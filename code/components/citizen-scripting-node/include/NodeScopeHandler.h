/*
 * This file is part of the Cfx project - https://cfx.re/
 *
 * See LICENSE in the root of the source tree for information
 * regarding licensing.
 */

#pragma once

#include "StdInc.h"

#include "UvTcpServer.h"
#include "UvLoopManager.h"
#include "NodeScriptRuntime.h"

namespace fx::nodejs
{
	class ScopeHandler
	{
	private:
		bool m_initialized = false;

	public:
		ScopeHandler() = default;

		void Initialize();
		void Shutdown();
	};
}
