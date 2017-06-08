#include "StdInc.h"

#if _WIN32
#include <CoreConsole.h>

#include <CfxState.h>
#include <HostSharedData.h>

#include <TcpServer.h>
#include <TcpServerManager.h>

#include <NetBuffer.h>

#include <mutex>
#include <shared_mutex>

#include <MultiplexTcpServer.h>
#include <HttpServerImpl.h>

#include <ResourceManager.h>
#include <Profiler.h>

#include <shellapi.h>

#pragma comment(lib, "shell32.lib")

#include <optional>
#include <json.hpp>

using json = nlohmann::json;

#include <strsafe.h>

static std::shared_mutex g_mutex;
static std::set<net::TcpServerStream*> g_streams;

struct ConsoleBuffer
{
	char channels[512][48];
	char buffers[512][1024];
	int curBufferRead;
	volatile int curBufferWrite;

	ConsoleBuffer()
		: curBufferRead(0), curBufferWrite(0)
	{

	}
};

static HANDLE g_stateSema = CreateSemaphore(nullptr, 0, 100000, L"CFX_Console_Produce");
static HANDLE g_stateMutex = CreateMutex(nullptr, FALSE, L"CFX_Console_Consume");

static void DevConPrintListener(ConsoleChannel channel, const char* message)
{
	static HostSharedData<ConsoleBuffer> conData("ConsoleData");

	WaitForSingleObject(g_stateMutex, INFINITE);

	int curWrite = conData->curBufferWrite++;
	StringCbCopyA(conData->buffers[curWrite], sizeof(conData->buffers[curWrite]), message);
	StringCbCopyA(conData->channels[curWrite], sizeof(conData->channels[curWrite]), channel.data());

	if (conData->curBufferWrite >= _countof(conData->channels))
	{
		conData->curBufferWrite = 0;
	}

	if (conData->curBufferWrite != conData->curBufferRead)
	{
		ReleaseSemaphore(g_stateSema, 1, nullptr);
	}
	
	ReleaseMutex(g_stateMutex);
}

inline uint16_t sSwapShortRead(uint16_t x)
{
	return _byteswap_ushort(x);
}

inline uint32_t sSwapLongRead(uint32_t x)
{
	return _byteswap_ulong(x);
}

inline uint64_t sSwapLongLongRead(uint64_t x)
{
	return _byteswap_uint64(x);
}

static std::set<std::string> g_knownChannels = { "Any", "font-renderer" };

static void FlushKnownChannels(net::TcpServerStream* stream)
{
	net::Buffer buf;
	buf.Write(0x4E414843); // 'CHAN'
	buf.Write<uint16_t>(sSwapShortRead(211)); // protocol
	buf.Write<uint32_t>(sSwapLongRead(14 + (g_knownChannels.size() * 58)));
	buf.Write<uint32_t>(sSwapLongRead(g_knownChannels.size()));

	for (auto& channel : g_knownChannels)
	{
		buf.Write<uint32_t>(HashString(channel.c_str()));
		buf.Write<uint32_t>(0);
		buf.Write<uint32_t>(0);
		buf.Write<uint32_t>(sSwapLongRead(2));
		buf.Write<uint32_t>(sSwapLongRead(2));
		buf.Write<uint32_t>(0);

		char channelStr[30] = { 0 };
		StringCbCopyA(channelStr, sizeof(channelStr), channel.c_str());

		buf.Write(channelStr, sizeof(channelStr));

		buf.Write<uint32_t>(sSwapLongRead(1));
	}

	stream->Write(buf.GetData());
}

static void FlushKnownCommands(net::TcpServerStream* stream)
{
	static std::set<std::string> lastCmds;
	std::set<std::string> cmds;

	Instance<ConsoleCommandManager>::Get()->ForAllCommands([&](const std::string& cmd)
	{
		cmds.insert(cmd);
	});

	std::vector<std::string> cmdDiffs;
	std::set_difference(cmds.begin(), cmds.end(), lastCmds.begin(), lastCmds.end(), std::back_inserter(cmdDiffs));

	lastCmds.swap(cmds);

	for (const std::string& cmd : cmdDiffs)
	{
		net::Buffer buf;
		buf.Write(0x52415643); // 'CVAR'
		buf.Write<uint16_t>(sSwapShortRead(211)); // protocol
		buf.Write<uint32_t>(sSwapLongRead(93));
		buf.Write<uint16_t>(0);

		char cmdBuf[64];
		strncpy(cmdBuf, cmd.c_str(), sizeof(cmdBuf));
		cmdBuf[63] = '\0';

		buf.Write(cmdBuf, sizeof(cmdBuf));

		buf.Write<uint32_t>(0);
		buf.Write<uint32_t>(sSwapLongRead(0)); // flags
		buf.Write<uint32_t>(sSwapLongRead(0)); // min
		buf.Write<uint32_t>(sSwapLongRead(0)); // max
		buf.Write<uint8_t>(0x11);

		stream->Write(buf.GetData());
	}
}

static void HandleConsoleMessage(const std::string& channel, const std::string& buffer)
{
	bool channelsChanged = false;

	if (g_knownChannels.find(channel) == g_knownChannels.end())
	{
		channelsChanged = true;

		g_knownChannels.insert(channel);
	}

	{
		net::Buffer buf;
		buf.Write(0x544E5250); // 'PRNT'
		buf.Write<uint16_t>(sSwapShortRead(211)); // protocol
		buf.Write<uint32_t>(sSwapLongRead(40 + 1 + buffer.length()));
		buf.Write<uint16_t>(0);
		buf.Write<uint32_t>(HashString(channel.c_str()));

		char dummyBuffer[24] = { 0 };
		buf.Write(dummyBuffer, 24);

		buf.Write(buffer.data(), buffer.size() + 1);

		{
			std::shared_lock<std::shared_mutex> lock(g_mutex);

			for (auto& stream : g_streams)
			{
				if (channelsChanged)
				{
					FlushKnownChannels(stream);
				}

				FlushKnownCommands(stream);

				stream->Write(buf.GetData());
			}
		}
	}
}

static InitFunction initFunction([]()
{
	static HostSharedData<CfxState> hostData("CfxInitState");
	static HostSharedData<ConsoleBuffer> conData("ConsoleData");

	console::CoreAddPrintListener(DevConPrintListener);

	if (!hostData->IsMasterProcess() && !hostData->IsGameProcess())
	{
		return;
	}

	std::thread([]()
	{
		SetThreadName(-1, "DevCon Thread");

		while (true)
		{
			// wait for an incoming semaphore
			WaitForSingleObject(g_stateSema, INFINITE);

			// lock the mutex
			WaitForSingleObject(g_stateMutex, INFINITE);

			// try to catch up
			int start = conData->curBufferRead;
			int end = conData->curBufferWrite;

			while (start != end)
			{
				std::string channel = conData->channels[start];
				std::string buffer = conData->buffers[start];

				HandleConsoleMessage(channel, buffer);

				++start;

				if (start >= _countof(conData->channels))
				{
					start = 0;
				}
			}

			conData->curBufferRead = start;

			// release the mutex
			ReleaseMutex(g_stateMutex);
		}
	}).detach();

	static fwRefContainer<net::TcpServerManager> tcpStack = new net::TcpServerManager();
	static fwRefContainer<net::TcpServer> tcpServer = tcpStack->CreateServer(net::PeerAddress::FromString("0.0.0.0:29100", 29100, net::PeerAddress::LookupType::NoResolution).get());

	if (!tcpServer.GetRef())
	{
		return;
	}

	static fwRefContainer<net::MultiplexTcpServer> multiServer = new net::MultiplexTcpServer();
	multiServer->AttachToServer(tcpServer);

	auto httpPatternMatcher = [](const std::vector<uint8_t>& bytes)
	{
		if (bytes.size() > 10)
		{
			auto firstR = std::find(bytes.begin(), bytes.end(), '\r');

			if (firstR != bytes.end())
			{
				auto firstN = firstR + 1;

				if (firstN != bytes.end())
				{
					if (*firstN == '\n')
					{
						std::string match(firstR - 8, firstR);

						if (match.find("HTTP/") == 0)
						{
							return net::MultiplexPatternMatchResult::Match;
						}
					}

					return net::MultiplexPatternMatchResult::NoMatch;
				}
			}
		}

		return net::MultiplexPatternMatchResult::InsufficientData;
	};

	struct Handler : public net::HttpHandler
	{
		std::function<bool(fwRefContainer<net::HttpRequest>, fwRefContainer<net::HttpResponse>)> handler;

		virtual bool HandleRequest(fwRefContainer<net::HttpRequest> request, fwRefContainer<net::HttpResponse> response) override
		{
			return handler(request, response);
		}
	};

	static std::optional<json> lastProfile;

	fx::ResourceManager::OnInitializeInstance.Connect([](fx::ResourceManager* resman)
	{
		resman->GetComponent<fx::ProfilerComponent>()->OnRequestView.Connect([](const json& json)
		{
			lastProfile = json;

			ShellExecuteW(nullptr, L"open", fmt::sprintf(L"http://frontend.chrome-dev.tools/serve_rev/@901bcc219d9204748f9c256ceca0f2cd68061006/inspector.html?loadTimelineFromURL=http://localhost:29100/profileData.json").c_str(), NULL, NULL, SW_SHOW);
		});
	}, INT32_MAX);

	static fwRefContainer<Handler> httpHandler = new Handler();
	httpHandler->handler = [=](fwRefContainer<net::HttpRequest> request, fwRefContainer<net::HttpResponse> response)
	{
		if (request->GetPath() == "/profileData.json")
		{
			response->SetHeader("Access-Control-Allow-Origin", "*");

			response->End(lastProfile->dump(-1, ' ', false, json::error_handler_t::replace));

			return true;
		}

		response->SetStatusCode(404);
		response->End(fmt::sprintf("Route %s not found.", request->GetPath()));

		return true;
	};

	static fwRefContainer<net::HttpServer> httpServer = new net::HttpServerImpl();
	httpServer->RegisterHandler(httpHandler);

	httpServer->AttachToServer(multiServer->CreateServer(httpPatternMatcher));

	static auto devconServer = multiServer->CreateServer([](const std::vector<uint8_t>& bytes)
	{
		if (bytes.size() >= 4)
		{
			return (*(uint32_t*)bytes.data() == 0x52435050 || *(uint32_t*)bytes.data() == 0x444E4D43)
				? net::MultiplexPatternMatchResult::Match
				: net::MultiplexPatternMatchResult::NoMatch;
		}

		return net::MultiplexPatternMatchResult::InsufficientData;
	});

	devconServer->SetConnectionCallback([](fwRefContainer<net::TcpServerStream> stream)
	{
		auto localStream = stream;

		stream->SetReadCallback([=](const std::vector<uint8_t>& data)
		{
			if (*(uint32_t*)data.data() == 0x52435050)
			{
				// send an AINF
				std::string commandLine = ToNarrow(GetCommandLine());

				net::Buffer buf;
				buf.Write(0x464E4941); // 'AINF'
				buf.Write<uint16_t>(sSwapShortRead(211)); // protocol
				buf.Write<uint32_t>(sSwapLongRead(commandLine.length() + 1 + 97));
				buf.Write<uint16_t>(0);
				//buf.Write<uint32_t>(HashString("Cfx"));
				buf.Write<uint32_t>(0xEFF8A1A);
				buf.Write<uint32_t>(0);
				buf.Write<uint32_t>(0x321F0C00);

				char gameName[32] = { 0 };
				char appName[32] = { 0 };

				strcpy(gameName, "CitizenFX");
				strcpy(appName, "CitizenFX");

				buf.Write(gameName, sizeof(gameName));
				buf.Write(appName, sizeof(appName));

				buf.Write<uint8_t>(0xFF);
				buf.Write(sSwapLongRead(0x8));

				buf.Write<uint32_t>(sSwapLongRead(commandLine.length() + 1));
				buf.Write(commandLine.data(), commandLine.length() + 1);

				localStream->Write(buf.GetData());

				FlushKnownChannels(localStream.GetRef());

				FlushKnownCommands(localStream.GetRef());

				{
					std::unique_lock<std::shared_mutex> lock(g_mutex);
					g_streams.insert(localStream.GetRef());
				}
			}
			else if (*(uint32_t*)data.data() == 0x444E4D43) // CMND
			{
				net::Buffer buf(data);
				buf.Read<uint32_t>();

				uint16_t protocol = sSwapShortRead(buf.Read<uint16_t>());
				uint32_t length = sSwapLongRead(buf.Read<uint32_t>());
				buf.Read<uint16_t>();

				std::vector<uint8_t> d(buf.GetRemainingBytes());
				buf.Read(d.data(), d.size() - 1);

				Instance<console::Context>::Get()->AddToBuffer(std::string(d.begin(), d.end()) + "\n");
			}
		});

		stream->SetCloseCallback([=]()
		{
			std::unique_lock<std::shared_mutex> lock(g_mutex);
			g_streams.erase(localStream.GetRef());
		});
	});
});
#endif
