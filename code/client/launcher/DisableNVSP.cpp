#include "StdInc.h"

#ifdef LAUNCHER_PERSONALITY_MAIN
#include <cpr/cpr.h>
#include <rapidjson/document.h>

#include <optional>

using namespace std::string_literals;

struct NvidiaConnectionInfo
{
	int port;
	std::string secret;

	NvidiaConnectionInfo(int port, const std::string& secret)
		: port(port), secret(secret)
	{

	}
};

template<typename TContainer>
static std::optional<NvidiaConnectionInfo> ParseNvidiaState(const TContainer& data)
{
	rapidjson::Document doc;
	doc.Parse(data.data(), data.size());

	if (!doc.HasParseError() && doc.IsObject())
	{
		if (doc.HasMember("port") && doc["port"].IsInt() && doc.HasMember("secret") && doc["secret"].IsString())
		{
			return NvidiaConnectionInfo{ doc["port"].GetInt(), doc["secret"].GetString() };
		}
	}

	return {};
}

static std::optional<NvidiaConnectionInfo> GetNvidiaStateNew()
{
	std::optional<NvidiaConnectionInfo> rv;
	auto fileMapping = OpenFileMappingW(FILE_MAP_READ, FALSE, L"{8BA1E16C-FC54-4595-9782-E370A5FBE8DA}");

	if (fileMapping != 0 && fileMapping != INVALID_HANDLE_VALUE)
	{
		const void* data = MapViewOfFile(fileMapping, FILE_MAP_READ, 0, 0, 0);

		if (data)
		{
			MEMORY_BASIC_INFORMATION mbi = { 0 };
			VirtualQuery(data, &mbi, sizeof(mbi));

			if (mbi.RegionSize > 0)
			{
				rv = ParseNvidiaState(std::string{ (const char*)data, mbi.RegionSize });
			}

			UnmapViewOfFile(data);
		}

		CloseHandle(fileMapping);
	}

	return rv;
}

static std::optional<NvidiaConnectionInfo> GetNvidiaState()
{
	if (auto result = GetNvidiaStateNew())
	{
		return result;
	}

	// we shouldn't even bother with systems that somehow broke core environment variables
	auto lad = _wgetenv(L"localappdata");

	if (!lad)
	{
		return {};
	}

	std::wstring path = lad + L"\\NVIDIA Corporation\\NvNode\\nodejs.json"s;

	FILE* f = _wfopen(path.c_str(), L"rb");

	if (f)
	{
		fseek(f, 0, SEEK_END);
		int flen = ftell(f);
		fseek(f, 0, SEEK_SET);

		std::vector<char> data(flen);
		fread(&data[0], 1, data.size(), f);
		fclose(f);

		return ParseNvidiaState(data);
	}

	return {};
}

static std::optional<bool> GetShadowPlayStatus(const NvidiaConnectionInfo& connection)
{
	auto r = cpr::Get(
		cpr::Url{fmt::sprintf("http://localhost:%d/ShadowPlay/v.1.0/Launch", connection.port)},
		cpr::Header{ {"X_LOCAL_SECURITY_COOKIE", connection.secret } }
	);

	if (!r.error)
	{
		rapidjson::Document doc;
		doc.Parse(r.text.data(), r.text.size());

		if (!doc.HasParseError() && doc.IsObject())
		{
			if (doc.HasMember("launch") && doc["launch"].IsBool())
			{
				return doc["launch"].GetBool();
			}
		}
	}

	return {};
}

static bool SetShadowPlayStatus(const NvidiaConnectionInfo& connection, bool enabled)
{
	auto r = cpr::Post(
		cpr::Url{ fmt::sprintf("http://localhost:%d/ShadowPlay/v.1.0/Launch", connection.port) },
		cpr::Header{ { "X_LOCAL_SECURITY_COOKIE", connection.secret }, { "Content-Type", "application/json" } },
		cpr::Body{ fmt::sprintf("{\"launch\":%s}", enabled ? "true" : "false") }
	);

	return (!r.error && r.status_code <= 399);
}

static void WriteSPEnableCookie()
{
	FILE* f = _wfopen(MakeRelativeCitPath(L"data\\cache\\enable_nvsp").c_str(), L"wb");

	if (f)
	{
		fclose(f);
	}
}

void NVSP_DisableOnStartup()
{
	std::wstring fpath = MakeRelativeCitPath(L"VMP.ini");

	bool disableNVSP = true;

	if (GetFileAttributes(fpath.c_str()) != INVALID_FILE_ATTRIBUTES)
	{
		disableNVSP = (GetPrivateProfileInt(L"Game", L"DisableNVSP", 1, fpath.c_str()) != 0);
	}

	if (!disableNVSP)
	{
		trace("DisableNVSP != 0, not disabling NVSP.\n");
		return;
	}

	std::thread([]()
	{
		auto nvConn = GetNvidiaState();

		if (nvConn)
		{
			trace("Detected NVIDIA Node, attempting to query ShadowPlay status...\n");

			auto state = GetShadowPlayStatus(*nvConn);

			if (!state)
			{
				trace("Couldn't query ShadowPlay status from NvNode, bailing out.\n");
			}
			else if (!*state)
			{
				trace("NvNode claims ShadowPlay is disabled, bailing out.\n");
			}
			else
			{
				trace("NvNode claims ShadowPlay is *enabled*, disabling for this session.\n");

				WriteSPEnableCookie();

				SetShadowPlayStatus(*nvConn, false);

				// wait for NVSP to be disabled
				int attempts = 0;

				while (*GetShadowPlayStatus(*nvConn))
				{
					Sleep(100);

					if (attempts > 30)
					{
						trace("Waited 30x for ShadowPlay to be disabled, continuing anyway.\n");
						break;
					}

					++attempts;
				}

				trace("Disabled NVSP.\n");
			}
		}
	}).detach();
}

void NVSP_ShutdownSafely()
{
	FILE* f = _wfopen(MakeRelativeCitPath(L"data\\cache\\enable_nvsp").c_str(), L"rb");

	if (f)
	{
		fclose(f);

		trace("Found NVSP cookie, re-enabling NVSP.\n");

		auto nvConn = GetNvidiaState();

		if (nvConn)
		{
			SetShadowPlayStatus(*nvConn, true);
		}

		_wunlink(MakeRelativeCitPath(L"data\\cache\\enable_nvsp").c_str());
	}
}
#endif
