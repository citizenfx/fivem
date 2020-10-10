#include <StdInc.h>

#ifdef GTA_FIVE
#define CNL_ENDPOINT "https://lambda.fivem.net"

#include <LegitimacyAPI.h>

#include <json.hpp>

#include <skyr/url.hpp>

#include <HttpClient.h>

using json = nlohmann::json;

static HookFunction initFunction([]()
{
	std::thread([]()
	{
		HANDLE hPipe = INVALID_HANDLE_VALUE;

		for (int i = 0; i < 10; i++)
		{
			hPipe = CreateFileW(va(L"\\\\.\\pipe\\discord-ipc-%d", i), GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);

			if (hPipe != INVALID_HANDLE_VALUE)
			{
				break;
			}
		}

		if (hPipe == NULL || hPipe == INVALID_HANDLE_VALUE)
		{
			return;
		}

		auto writePipe = [hPipe](int opCode, const json& data)
		{
			auto dataStr = data.dump(-1, ' ', false, nlohmann::detail::error_handler_t::replace);

			std::vector<uint8_t> dataBuffer(dataStr.length() + 4 + 4);
			*(uint32_t*)&dataBuffer[0] = opCode;
			*(uint32_t*)&dataBuffer[4] = dataStr.length();
			memcpy(&dataBuffer[8], dataStr.data(), dataStr.length());

			DWORD bytesWritten;
			WriteFile(hPipe, dataBuffer.data(), dataBuffer.size(), &bytesWritten, NULL);
		};

		writePipe(0 /* HANDSHAKE */, json::object({
			{ "v", 1},
			{ "client_id", "382624125287399424"}
		}));

		auto closeConnection = [&writePipe, &hPipe]()
		{
			writePipe(2, {});
			CloseHandle(hPipe);
			hPipe = INVALID_HANDLE_VALUE;
		};

		auto handleMsg = [&writePipe, &closeConnection](int opCode, const json& data)
		{
			static std::string userId;

			switch (opCode)
			{
			case 1: // FRAME
				if (data["evt"] == "READY")
				{
					userId = data["data"]["user"]["id"].get<std::string>();

					// check with CnL if we have access
					Instance<::HttpClient>::Get()->DoPostRequest(
						CNL_ENDPOINT "/api/validate/discord",
						{
							{ "entitlementId", ros::GetEntitlementSource() },
							{ "userId", userId }
						},
						[writePipe, closeConnection](bool success, const char* data, size_t length)
					{
						if (!success && strstr(data, "HTTP 4") != nullptr)
						{
							writePipe(1 /* FRAME */, json::object({
								{ "cmd", "AUTHORIZE" },
								{ "args", json::object({
									{ "scopes", json::array({"identify"}) },
									{ "client_id", "382624125287399424" },
									{ "redirect_url", "https://cfx.re" },
									{ "prompt", "none" },
								}) },
								{ "nonce", "nonce1" },
								}));
						}
						else
						{
							closeConnection();
						}
					});
				}
				else if (data["nonce"] == "nonce1")
				{
					if (data["evt"] == "ERROR")
					{
						// user probably denied auth request in Discord UI
						// #TODO: store this and only ask again when asked by CfxUI
						closeConnection();
					}
					else
					{
						auto code = data["data"]["code"].get<std::string>();

						if (!code.empty())
						{
							Instance<::HttpClient>::Get()->DoPostRequest(
								CNL_ENDPOINT "/api/validate/discord",
								{
									{ "entitlementId", ros::GetEntitlementSource() },
									{ "authCode", code },
									{ "userId", userId }
								},
								[](bool, const char*, size_t)
							{

							});
						}

						closeConnection();
					}
				}
				break;
			case 3: // PING
				writePipe(4 /* PONG */, data);
				break;
			}
		};

		while (true)
		{
			if (hPipe == NULL || hPipe == INVALID_HANDLE_VALUE)
			{
				break;
			}

			// read from pipe
			struct  
			{
				uint32_t opCode;
				uint32_t length;
			} pktHdr;

			DWORD bytesAvail = 0;

			if (PeekNamedPipe(hPipe, NULL, 0, NULL, &bytesAvail, NULL) && bytesAvail > 0)
			{
				DWORD numRead = 0;
				ReadFile(hPipe, &pktHdr, sizeof(pktHdr), &numRead, NULL);

				if (numRead == sizeof(pktHdr) && pktHdr.length > 0)
				{
					std::vector<uint8_t> buf(pktHdr.length);
					ReadFile(hPipe, buf.data(), buf.size(), &numRead, NULL);

					if (numRead == buf.size())
					{
						try
						{
							handleMsg(pktHdr.opCode, json::parse(buf));
						}
						catch (json::exception& e)
						{

						}
					}
				}
			}
			else
			{
				Sleep(50);
			}
		}
	}).detach();
});
#endif
