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
class UvLoopTimer
{
private:
	std::vector<NodeScriptRuntime*> m_runtimes;
	UvHandleContainer<uv_timer_t> m_timer;
	bool m_initialized = false;

public:
	UvLoopTimer() = default;

	void Initialize();
	void Shutdown();

	void AddRuntime(NodeScriptRuntime* rt);
	void RemoveRuntime(NodeScriptRuntime* rt);
};
}
