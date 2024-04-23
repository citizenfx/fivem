/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"

#include <NodeParentEnvironment.h>
#include <console/Console.h>

#include <node/v8/libplatform/libplatform.h>
#include "shared/RuntimeHelpers.h"

namespace fx::nodejs
{
	result_t NodeParentEnvironment::Initialize()
	{
		// initialize node process
		const std::vector<std::string> args = {
			"--trace-warnings",
			"--unhandled-rejections=warn"
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

		m_isolate = node::NewIsolate(node::CreateArrayBufferAllocator(), uv_default_loop(), m_platform.get());
		if (!m_isolate)
		{
			console::PrintError(_CFX_NAME_STRING(_CFX_COMPONENT_NAME), "Error while initializing node: failed to create isolate\n");
			return FX_E_INVALIDARG;
		}

		SharedPushEnvironmentNoContext pushed(m_isolate);

		m_context.Reset(m_isolate, node::NewContext(m_isolate));

		m_initialized = true;
		return FX_S_OK;
	}
	
	void NodeParentEnvironment::Tick() const
	{
		SharedPushEnvironmentNoContext pushed(m_isolate);
		m_platform->DrainTasks(m_isolate);
	}
}
