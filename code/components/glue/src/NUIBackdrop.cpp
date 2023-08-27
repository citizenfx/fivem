#include <StdInc.h>

#if __has_include(<CefOverlay.h>)
#include <CoreConsole.h>
#include <NUISchemeHandlerFactory.h>

#include <include/cef_parser.h>

#include <CefOverlay.h>

#include <VFSManager.h>

class NuiBackdropResourceHandler : public CefResourceHandler
{
public:
	virtual bool ProcessRequest(CefRefPtr<CefRequest> request, CefRefPtr<CefCallback> callback) override
	{
		CefURLParts parts;
		CefParseURL(request->GetURL(), parts);

		std::string path = CefString(&parts.path).ToString().substr(1);
		if (path == "user.png")
		{
			auto e = console::GetDefaultContext()->GetVariableManager()->FindEntryRaw("ui_customBackdrop");

			if (e)
			{
				FILE* f = _wfopen(ToWide(e->GetValue()).c_str(), L"rb");

				if (f)
				{
					fseek(f, 0, SEEK_END);
					m_result.resize(ftell(f));
					fseek(f, 0, SEEK_SET);
					fread(&m_result[0], 1, m_result.size(), f);
					fclose(f);

					m_found = true;
				}
				else
				{
					e->SetValue("");
				}
			}
		}

		callback->Continue();

		return true;
	}

	virtual void GetResponseHeaders(CefRefPtr<CefResponse> response, int64& response_length, CefString& redirectUrl) override
	{
		response->SetMimeType("application/octet-stream");

		if (!m_found)
		{
			response->SetStatus(404);
		}
		else
		{
			response->SetStatus(200);
		}

		CefResponse::HeaderMap map;
		response->GetHeaderMap(map);

		map.insert({ "cache-control", "max-age=7200" });

		response->SetHeaderMap(map);

		if (m_found)
		{
			response_length = m_result.size();
		}
		else
		{
			response_length = 0;
		}

		m_cursor = 0;
	}

	virtual void Cancel() override
	{
	}

	virtual bool ReadResponse(void* data_out, int bytes_to_read, int& bytes_read, CefRefPtr<CefCallback> callback) override
	{
		if (m_found)
		{
			int toRead = std::min(m_result.size() - m_cursor, (size_t)bytes_to_read);
			memcpy(data_out, (char*)m_result.data() + m_cursor, toRead);

			m_cursor += toRead;
			bytes_read = toRead;

			return (bytes_read > 0);
		}

		return false;
	}

private:
	bool m_found = false;
	std::vector<uint8_t> m_result;
	size_t m_cursor = 0;

	IMPLEMENT_REFCOUNTING(NuiBackdropResourceHandler);
};

static InitFunction initfunction([]()
{
	OnSchemeCreateRequest.Connect([](const char* name, CefRefPtr<CefRequest> request, CefRefPtr<CefResourceHandler>& handler)
	{
		if (!strcmp(name, "http") || !strcmp(name, "https"))
		{
			// parse the URL to get the hostname
			CefString url = request->GetURL();
			CefURLParts urlParts;

			if (CefParseURL(url, urlParts))
			{
				CefString hostString = &urlParts.host;

				if (hostString == "nui-backdrop")
				{
					handler = new NuiBackdropResourceHandler();

					return false;
				}
			}
		}

		return true;
	});

	nui::OnInitialize.Connect([]
	{
		nui::RegisterSchemeHandlerFactory("https", "nui-backdrop", Instance<NUISchemeHandlerFactory>::Get());
	});
});
#endif
