#include <StdInc.h>
#include <NativeHandlerLogging.h>

#include <stack>

static std::stack<std::shared_ptr<concurrency::concurrent_unordered_set<uint64_t>>> g_nativeLogs;

void NativeHandlerLogging::PushLog(const std::shared_ptr<concurrency::concurrent_unordered_set<uint64_t>>& ptr)
{
	g_nativeLogs.push(ptr);
}

std::shared_ptr<concurrency::concurrent_unordered_set<uint64_t>>NativeHandlerLogging::PopLog()
{
	auto ptr = g_nativeLogs.top();
	g_nativeLogs.pop();

	return ptr;
}

void NativeHandlerLogging::CountNative(uint64_t identifier)
{
	if (!g_nativeLogs.empty())
	{
		g_nativeLogs.top()->insert(identifier);
	}
}
