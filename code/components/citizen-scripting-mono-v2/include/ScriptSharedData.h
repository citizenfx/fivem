#pragma once

#include <atomic>

namespace fx::mono
{
struct ScriptSharedData
{
public:
	std::atomic<uint64_t> m_scheduledTime = ~uint64_t(0);
};
}
