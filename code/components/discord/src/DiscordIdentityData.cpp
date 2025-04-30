#include <StdInc.h>

#if defined(GTA_FIVE) || defined(IS_RDR3)

#include <SharedLegitimacyAPI.h>
#include <CL2LaunchMode.h>
#include <json.hpp>
#include <skyr/url.hpp>
#include <HttpClient.h>
#include <thread>
#include <vector>
#include <string>
#include <windows.h>

using json = nlohmann::json;

static void InitializeDiscordIntegration();

static HookFunction initFunction(InitializeDiscordIntegration);

namespace discord_integration
{
static HANDLE g_hPipe = INVALID_HANDLE_VALUE;
static std::string g_userId;

static void WritePipe(int opCode, const json& data)
{
	if (g_hPipe == nullptr || g_hPipe == INVALID_HANDLE_VALUE)
	{
		return;
	}

	std::string dataStr = data.dump(-1, ' ', false, nlohmann::detail::error_handler_t::replace);
	std::vector<uint8_t> dataBuffer(dataStr.size() + 8);
	*reinterpret_cast<uint32_t*>(&dataBuffer[0]) = static_cast<uint32_t>(opCode);
	*reinterpret_cast<uint32_t*>(&dataBuffer[4]) = static_cast<uint32_t>(dataStr.size());
	memcpy(&dataBuffer[8], dataStr.data(), dataStr.size());

	DWORD bytesWritten = 0;
	WriteFile(g_hPipe, dataBuffer.data(), static_cast<DWORD>(dataBuffer.size()), &bytesWritten, NULL);
}

static void CloseConnection()
{
	if (g_hPipe != nullptr && g_hPipe != INVALID_HANDLE_VALUE)
	{
		WritePipe(2, {});
		CloseHandle(g_hPipe);
		g_hPipe = INVALID_HANDLE_VALUE;
	}
}

static void OnDiscordAuthenticateWithCode(bool, const char*, size_t)
{
}

static void OnDiscordAuthenticateWithoutCode(bool success, const char* data, size_t)
{
	if (g_hPipe == INVALID_HANDLE_VALUE)
	{
		return;
	}

	if (!success && data && strstr(data, "HTTP 4") != nullptr && !launch::IsSDKGuest())
	{
		WritePipe(
		1,
		json::object({ { "cmd", "AUTHORIZE" },
		{ "args", json::object({
				  { "scopes", json::array({ "identify", "guilds.join" }) },
				  { "client_id", "382624125287399424" },
				  { "redirect_url", "https://cfx.re" },
				  { "prompt", "none" },
				  }) },
		{ "nonce", "nonce1" } }));
	}
	else
	{
		CloseConnection();
	}
}

static void HandleMessage(int opCode, const json& data)
{
	switch (opCode)
	{
		case 1:
			if (data.contains("evt") && data["evt"] == "READY" && data.contains("data") && data["data"].contains("user"))
			{
				g_userId = data["data"]["user"]["id"].get<std::string>();
				if (g_userId == "0")
				{
					break;
				}

				cfx::legitimacy::AuthenticateDiscord(
				g_userId.c_str(),
				nullptr,
				&OnDiscordAuthenticateWithoutCode);
			}
			else if (data.contains("nonce") && data["nonce"] == "nonce1")
			{
				if (data.contains("evt") && data["evt"] == "ERROR")
				{
					CloseConnection();
				}
				else
				{
					if (data.contains("data") && data["data"].contains("code"))
					{
						std::string code = data["data"]["code"].get<std::string>();
						if (!code.empty())
						{
							cfx::legitimacy::AuthenticateDiscord(
							g_userId.c_str(),
							code.c_str(),
							&OnDiscordAuthenticateWithCode);
						}
					}
					CloseConnection();
				}
			}
			break;
		case 3:
			WritePipe(4, data);
			break;
		default:
			break;
	}
}

static void StartDiscordIPCThread()
{
	while (true)
	{
		for (int i = 0; i < 10; i++)
		{
			std::wstring pipeName = std::wstring(L"\\\\.\\pipe\\discord-ipc-") + std::to_wstring(i);
			g_hPipe = CreateFileW( pipeName.c_str(), GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);

			if (g_hPipe != INVALID_HANDLE_VALUE)
			{
				break;
			}
		}

		if (g_hPipe == NULL || g_hPipe == INVALID_HANDLE_VALUE)
		{
			::Sleep(5000);
			continue;
		}
		else
		{
			break;
		}
	}

	WritePipe(0, json::object({ { "v", 1 }, { "client_id", "382624125287399424" } }));
	while (true)
	{
		if (g_hPipe == NULL || g_hPipe == INVALID_HANDLE_VALUE)
		{
			break;
		}

		DWORD bytesAvailable = 0;
		if (PeekNamedPipe(g_hPipe, NULL, 0, NULL, &bytesAvailable, NULL) && bytesAvailable > 0)
		{
			struct
			{
				uint32_t opCode;
				uint32_t length;
			} pktHdr;

			DWORD bytesRead = 0;
			BOOL readOk = ReadFile(g_hPipe, &pktHdr, sizeof(pktHdr), &bytesRead, NULL);
			if (readOk && bytesRead == sizeof(pktHdr) && pktHdr.length > 0)
			{
				std::vector<uint8_t> buf(pktHdr.length);
				DWORD dataRead = 0;
				BOOL readOk2 = ReadFile(g_hPipe, buf.data(), static_cast<DWORD>(buf.size()), &dataRead, NULL);
				if (readOk2 && dataRead == buf.size())
				{
					try
					{
						json parsed = json::parse(buf);
						HandleMessage(pktHdr.opCode, parsed);
					}
					catch (const json::exception&)
					{
					}
				}
			}
		}
		else
		{
			::Sleep(50);
		}
	}
}
}

static void InitializeDiscordIntegration()
{
	std::thread th(discord_integration::StartDiscordIPCThread);
	th.detach();
}

#endif
