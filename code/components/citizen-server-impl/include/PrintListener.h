#pragma once

namespace fx
{
	struct FxPrintListener
	{
		static inline thread_local std::function<void(std::string_view cb)> listener;
		static inline thread_local std::function<void(ConsoleChannel& channel, std::string_view data)> filter;

		inline FxPrintListener()
		{
			auto& printFilterEvent = *console::CoreGetPrintFilterEvent();
			printFilterEvent.Connect([](ConsoleChannel& channel, const char* data)
			{
				if (filter)
				{
					filter(channel, data);
				}
			});

			console::CoreAddPrintListener([](ConsoleChannel channel, const char* data)
			{
				if (listener)
				{
					listener(data);
				}
			});
		}
	};

	extern FxPrintListener printListener;

	struct PrintFilterContext
	{
		inline PrintFilterContext(decltype(FxPrintListener::filter)&& fn)
		{
			oldFn = std::move(FxPrintListener::filter);
			FxPrintListener::filter = std::move(fn);
		}

		inline ~PrintFilterContext()
		{
			FxPrintListener::filter = std::move(oldFn);
		}

	private:
		decltype(FxPrintListener::filter) oldFn;
	};

	struct PrintListenerContext
	{
		inline PrintListenerContext(std::function<void(std::string_view cb)>&& fn)
		{
			oldFn = std::move(FxPrintListener::listener);
			FxPrintListener::listener = std::move(fn);
		}

		inline ~PrintListenerContext()
		{
			FxPrintListener::listener = std::move(oldFn);
		}

	private:
		std::function<void(std::string_view cb)> oldFn;
	};

	struct ScopeDestructor
	{
		inline ScopeDestructor(std::function<void()>&& fn)
			: m_fn(std::move(fn))
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
