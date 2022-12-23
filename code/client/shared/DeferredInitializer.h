#pragma once

struct DeferredInitializer
{
	template<typename TFn>
	inline static std::shared_ptr<DeferredInitializer> Create(TFn&& fn)
	{
		HANDLE event = CreateEventW(NULL, TRUE, FALSE, NULL);

		struct Data
		{
			TFn fn;
			HANDLE hEvent;

			explicit Data(TFn&& fn, HANDLE hEvent)
				: fn(std::move(fn)), hEvent(hEvent)
			{

			}
		};

		auto data = new Data(std::move(fn), event);

		QueueUserWorkItem([](LPVOID arg)
		{
			auto data = reinterpret_cast<Data*>(arg);
			data->fn();
			SetEvent(data->hEvent);

			delete data;

			return DWORD(0);
		}, data, 0);

		return std::make_shared<DeferredInitializer>(event);
	}

	inline ~DeferredInitializer()
	{
		if (event)
		{
			CloseHandle(event);
			event = NULL;
		}
	}

	inline void Wait()
	{
		if (event)
		{
			WaitForSingleObject(event, INFINITE);
			CloseHandle(event);
			event = NULL;
		}
	}

	explicit DeferredInitializer(HANDLE event)
		: event(event)
	{
	}

private:
	HANDLE event;
};
