#include <StdInc.h>
#include <CefOverlay.h>
#include <CoreConsole.h>

#include <include/base/cef_bind.h>
#include <include/wrapper/cef_closure_task.h>
#include <include/cef_trace.h>

class CefTracingNotification : public CefEndTracingCallback, public CefCompletionCallback
{
public:
	IMPLEMENT_REFCOUNTING(CefTracingNotification);

	// Inherited via CefEndTracingCallback
	virtual void OnEndTracingComplete(const CefString& tracing_file) override
	{
		trace("Ended Chrome tracing, saved to file %s.\n", tracing_file.ToString());
	}

	virtual void OnComplete() override
	{
		trace("Started Chrome tracing.\n");
	}
};

static CefRefPtr<CefTracingNotification> tracingNotification;

static void BeginTracing(const std::string& categories)
{
	if (!CefBeginTracing(categories, tracingNotification))
	{
		trace("Failed to begin Chrome tracing.\n");
	}
}

static void EndTracing(const std::string& filename)
{
	if (!CefEndTracing(filename, tracingNotification))
	{
		trace("Failed to end Chrome tracing.\n");
	}
}

static InitFunction initFunction([]()
{
	tracingNotification = new CefTracingNotification();

	static ConsoleCommand nuiBeginTracing("nui_beginChromeTracing", []()
	{
		CefPostTask(TID_UI, base::Bind(&BeginTracing, ""));
	});

	static ConsoleCommand nuiBeginTracingCategories("nui_beginChromeTracing", [](const std::string& categories)
	{
		CefPostTask(TID_UI, base::Bind(&BeginTracing, categories));
	});

	static ConsoleCommand nuiEndTracing("nui_endChromeTracing", [](const std::string& filename)
	{
		CefPostTask(TID_UI, base::Bind(&EndTracing, filename));
	});
});
