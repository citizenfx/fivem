/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#pragma once

#include "StdInc.h"

#include <om/OMComponent.h>

#include <node/src/node.h>
#include <node/deps/uv/include/uv.h>

namespace fx::nodejs
{
	class NodeParentEnvironment
	{
	private:
		v8::Isolate* m_isolate;
		v8::UniquePersistent<v8::Context> m_context;
		std::unique_ptr<node::MultiIsolatePlatform> m_platform;
		bool m_initialized = false;
		
	public:
		NodeParentEnvironment() = default;

		result_t Initialize();
		void Tick() const;

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
