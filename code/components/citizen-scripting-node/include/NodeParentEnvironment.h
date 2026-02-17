/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#pragma once

#include "StdInc.h"

#include <om/OMComponent.h>

#include <node.h>
#include <uv.h>

namespace fx::nodejs
{
	class NodeParentEnvironment
	{
	private:
		std::unique_ptr<node::MultiIsolatePlatform> m_platform;
		bool m_initialized = false;
		
	public:
		NodeParentEnvironment() = default;

		result_t Initialize();
		bool IsStartNode() const;
		result_t StartNode();

		node::MultiIsolatePlatform* GetPlatform() const
		{
			return m_platform.get();
		}

		bool IsInitialized() const
		{
			return m_initialized;
		}
	};
}
