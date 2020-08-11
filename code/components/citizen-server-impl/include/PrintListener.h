#pragma once

namespace fx
{
struct FxPrintListener
{
	static thread_local std::function<void(const std::string_view& cb)> listener;

	inline FxPrintListener()
	{
		console::CoreAddPrintListener([](ConsoleChannel channel, const char* data) {
			if (listener)
			{
				listener(data);
			}
		});
	}
};

extern FxPrintListener printListener;

struct PrintListenerContext
{
	inline PrintListenerContext(const std::function<void(const std::string_view& cb)>& fn)
	{
		oldFn = FxPrintListener::listener;

		FxPrintListener::listener = fn;
	}

	inline ~PrintListenerContext()
	{
		FxPrintListener::listener = oldFn;
	}

private:
	std::function<void(const std::string_view& cb)> oldFn;
};

struct ScopeDestructor
{
	inline ScopeDestructor(const std::function<void()>& fn)
		: m_fn(fn)
	{
	}

	inline ~ScopeDestructor()
	{
		if (m_fn)
		{
			m_fn();
		}
	}

private:
	std::function<void()> m_fn;
};
}
