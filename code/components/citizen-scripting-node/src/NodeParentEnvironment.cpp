/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"

#include <NodeParentEnvironment.h>
#include <console/Console.h>

#include <node/deps/v8/include/libplatform/libplatform.h>
#include "shared/RuntimeHelpers.h"
#include "UvLoopManager.h"

extern int g_argc;
extern char** g_argv;

namespace fx::nodejs
{
	result_t NodeParentEnvironment::Initialize()
	{
		// redirect initialization if it should be a simple node process
		if (IsStartNode())
		{
			return StartNode();
		}

		// initialize node process
		const std::vector<std::string> args = {
			"--trace-warnings",
			"--unhandled-rejections=warn",
			"--experimental-sqlite"
		};

		const auto result = node::InitializeOncePerProcess(args);

		if (result->errors().size())
		{
			for (const auto& error : result->errors())
			{
				console::PrintError(_CFX_NAME_STRING(_CFX_COMPONENT_NAME), "Error while initializing node: %s\n", error);
			}
			return FX_E_INVALIDARG;
		}

		m_platform.reset(result->platform());

		m_initialized = true;
		return FX_S_OK;
	}

	bool NodeParentEnvironment::IsStartNode() const
	{
		// checking for node20 is enough, no need to check for start-node, that is only for compatibility
		// and to not change other files with each update
		// see NodeScriptRuntime.cpp -> node::CreateEnvironment to know where this parameter comes from
		return std::find_if(g_argv, g_argv + g_argc, [](char* arg)
		{
			return strcmp(arg, "--fork-node22") == 0;
		}) != g_argv + g_argc;
	}

	result_t NodeParentEnvironment::StartNode()
	{
		return node::Start(g_argc, g_argv);
	}
}
