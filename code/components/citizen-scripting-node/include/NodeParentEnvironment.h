/*
* This file is part of the Cfx project - https://cfx.re/
 *
 * See LICENSE in the root of the source tree for information
 * regarding licensing.
 */

#pragma once

#include "StdInc.h"

#include <om/OMComponent.h>

#include <node.h>

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
