/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include "ToolComponentHelpers.h"

#include <boost/filesystem/path.hpp>

#define NDEBUG
#include <ipc/ipc_channel_reader.h>
#include <ipc/ipc_listener.h>
#include <ipc/ipc_sync_message.h>

static void Subprocess_HandleArguments(boost::program_options::wcommand_line_parser& parser, std::function<void()> cb)
{
	boost::program_options::options_description desc;

	desc.add_options()
		("cake", boost::program_options::value<std::vector<std::string>>()->required(), "");

	boost::program_options::positional_options_description positional;
	positional.add("cake", -1);

	parser.options(desc).
		   positional(positional).
		   allow_unregistered();

	cb();
}

std::wstring g_origProcess;

void DoLauncherUiSkip();

static void Subprocess_Run(const boost::program_options::variables_map& map)
{
	auto args = map["cake"].as<std::vector<std::string>>();

	boost::filesystem::path programPath(args[0]);

	auto parentPath = programPath.parent_path();
	SetCurrentDirectory(parentPath.wstring().c_str());

	trace("sub! %s\n", GetCommandLineA());

	ToolMode_SetPostLaunchRoutine([]()
	{
		DoLauncherUiSkip();
	});

	g_origProcess = programPath.wstring();
	ToolMode_LaunchGame(programPath.wstring().c_str());
}

DWORD ToolGetModuleFileNameW(LPWSTR fileName, DWORD nSize)
{
	wcsncpy(fileName, g_origProcess.c_str(), nSize);

	return wcslen(fileName);
}

static FxToolCommand rosSubprocess("ros:subprocess", Subprocess_HandleArguments, Subprocess_Run);

#include <json.hpp>
#include <boost/algorithm/string.hpp>
#include <Error.h>

using json = nlohmann::json;

struct MyListener;

static MyListener* RunListener(const std::wstring& a);

std::string HandleCfxLogin();

bool g_launchDone;

struct MyListener : public IPC::Listener, public IPC::MessageReplyDeserializer
{
	HANDLE hPipe;
	int initWindowReplyId;
	std::wstring name;
	std::string url;
	MyListener* child;

	std::map<int, std::function<void(bool, const std::string&)>> jsReplies;

	// Inherited via Listener
	virtual bool OnMessageReceived(const IPC::Message& message)
	{
		auto type = message.type();

		base::PickleIterator iter(message);
		auto payload = message.payload();
		auto payloadSize = message.payload_size();

		std::vector<char> data((char*)payload, (char*)payload + payloadSize);

		if (type == 0x1000a)
		{
			iter.ReadString(&url);

			IPC::SyncMessage outMsg(0x7FFFFFFF, 0x2001B, IPC::Message::PRIORITY_NORMAL, new MyListener());
			initWindowReplyId = IPC::SyncMessage::GetMessageId(outMsg);

			Write(outMsg);
		}
		else if (type == 0x1000b)
		{
			// exec js
			std::wstring js;
			iter.ReadString16(&js);

			if (js.find(L"RGSC_JS_PING") != std::string::npos)
			{
				child->SendJSCallback("RGSC_PONG", "");
			}
			else if (js.find(L"READY_TO_ACCEPT") != std::string::npos)
			{
				child->SendJSCallback("RGSC_READY_TO_ACCEPT_COMMANDS", "");

				auto child = this->child;

				child->SendJSSync("RGSC_GET_TITLE_ID", "", [child](bool success, const std::string& jd)
				{
					json j = json::parse(jd);

					child->SendJSCallback("RGSC_SIGN_IN", HandleCfxLogin());
				});
			}
			else if (js.find(L"RGSC_JS_RECEIVE_MESSAGE") != std::string::npos)
			{
				auto s = L"RGSC_JS_RECEIVE_MESSAGE('";
				auto e = L"');} else {R";

				auto start = js.find(s) + wcslen(s);
				auto end = js.find(e);

				auto str = js.substr(start, end - start);
				boost::algorithm::replace_all(str, L"\\\"", "\"");
				boost::algorithm::replace_all(str, L"\\\\", "\\");

				try
				{
					auto j = json::parse(ToNarrow(str));
					auto targetTitle =
#ifdef GTA_FIVE
						"gta5"
#elif defined(IS_RDR3)
						"rdr2"
#else
						""
#endif
						;

					static bool verified;
					static bool launched;
					static bool verifying;
					static bool launchDone;

					for (json& cmd : j["Commands"])
					{
						auto c = cmd.value("Command", "");
						const json& p = cmd["Parameter"];

						trace("SC JS message: %s -> %s\n", c, p.dump(-1, ' ', false, nlohmann::detail::error_handler_t::replace));

						if (c == "SetGameLaunchState")
						{
							if (p.value("launchState", "") == "failed")
							{
								FatalError("Failed to launch through MTL.");
							}

							launchDone = true;
						}

						if (c == "SetTitleInfo") {
							if (p.value("titleName", "") == targetTitle) {
								if (p["status"].value("entitlement", false) && !p["status"].value("install", false) && !verified) {
									if (!verifying) {
										child->SendJSCallback("RGSC_RAISE_UI_EVENT", json::object({
											{ "EventId", 2 }, // LauncherV3UiEvent

											{"Data", json::object({
												{"Action", "Install"},
												{"Parameter", json::object({
													{"titleName", targetTitle},
													{"location", "C:\\Program Files\\Rockstar Games\\Red Dead Redemption 2"},
													{"desktopShortcut", false},
													{"startMenuShortcut", false}
												})}
											})}
											}).dump());

										verifying = true;
									}

									if (p["status"].value("updateState", "") == "starting")
									{
										Sleep(500);

										child->SendJSCallback("RGSC_RAISE_UI_EVENT", json::object({
											{ "EventId", 2 }, // LauncherV3UiEvent

											{"Data", json::object({
												{"Action", "Verify"},
												{"Parameter", json::object({
													{"titleName", targetTitle},
												})}
											})}
											}).dump());
									}
								}
								else if (p["status"].value("install", false) && (p["status"].value("updateState", "") == "notUpdating" || p["status"].value("updateState", "") == "updateQueued"))
								{
									if (verified && !launched)
									{
										launched = true;

										for (int i = 0; i < 3; i++)
										{
											if (launchDone || g_launchDone)
											{
												break;
											}

											child->SendJSCallback("RGSC_SET_CLOUD_SAVE_ENABLED", json::object({
												{ "Enabled", false },
												{ "RosTitleName", targetTitle }
												}).dump());

											child->SendJSCallback("RGSC_RAISE_UI_EVENT", json::object({
												{ "EventId", 2 }, // LauncherV3UiEvent

												{"Data", json::object({
													{"Action", "Launch"},
													{"Parameter", json::object({
														{"titleName", targetTitle},
														{"args", ""}
													})}
												})}
												}).dump());

											Sleep(5000);
										}
									}
									else if (!verified)
									{
										child->SendJSCallback("RGSC_RAISE_UI_EVENT", json::object({
											{ "EventId", 2 }, // LauncherV3UiEvent

											{"Data", json::object({
												{"Action", "Verify"},
												{"Parameter", json::object({
													{"titleName", targetTitle},
													{"args", ""}
												})}
											})}
											}).dump());
									}
								}
								
								if (p["status"].value("updateState", "") == "verifying")
								{
									verified = true;
								}
							}
						}
					}
				}
				catch (json::exception& e)
				{

				}
			}
		}
		else if (type == 0xfffffff0)
		{
			int replyId;
			iter.ReadInt(&replyId);

			if (replyId == initWindowReplyId)
			{
				std::string str;
				iter.ReadString(&str);

				child = RunListener(ToWide(str));

				{
					IPC::Message outMsg(1, 0x20008, IPC::Message::PRIORITY_NORMAL);
					outMsg.WriteInt(1);

					Write(outMsg);
				}

				{
					IPC::Message outMsg(1, 0x20013, IPC::Message::PRIORITY_NORMAL);
					outMsg.WriteInt64(0); // hwnd
					outMsg.WriteInt64(0); // more hwnd

					Write(outMsg);
				}

				{
					IPC::Message outMsg(1, 0x20006, IPC::Message::PRIORITY_NORMAL);
					outMsg.WriteString(url);
					outMsg.WriteInt(1);
					outMsg.WriteInt(1);

					Write(outMsg);
				}

				{
					IPC::Message outMsg(1, 0x2000B, IPC::Message::PRIORITY_NORMAL);
					outMsg.WriteString(url);

					Write(outMsg);
				}

				{
					IPC::Message outMsg(1, 0x20007, IPC::Message::PRIORITY_NORMAL);
					outMsg.WriteString(url);
					outMsg.WriteInt(1);
					outMsg.WriteInt(200);

					Write(outMsg);
				}

				{
					IPC::Message outMsg(1, 0x20008, IPC::Message::PRIORITY_NORMAL);
					outMsg.WriteInt(0);

					Write(outMsg);
				}
			}
			else if (jsReplies.find(replyId) != jsReplies.end())
			{
				int succ;
				std::wstring json;
				iter.ReadInt(&succ);
				iter.ReadString16(&json);

				jsReplies[replyId](succ ? true : false, ToNarrow(json));
				jsReplies.erase(replyId);
			}
		}

		return true;
	}

	void Write(const IPC::Message& message)
	{
		DWORD bytes_written = 0;

		auto ev = CreateEvent(NULL, FALSE, FALSE, NULL);

		OVERLAPPED overlapped = { 0 };
		overlapped.hEvent = ev;

		BOOL ok = WriteFile(hPipe,
			message.data(),
			static_cast<int>(message.size()),
			&bytes_written,
			&overlapped);

		if (!ok && GetLastError() == ERROR_IO_PENDING)
		{
			WaitForSingleObject(ev, INFINITE);
		}
	}

	void SendJSCallback(const std::string& name, const std::string& json)
	{
		IPC::Message outMsg(1, 0x20009, IPC::Message::PRIORITY_NORMAL);
		outMsg.WriteString16(ToWide(name));
		outMsg.WriteString16(ToWide(json));

		auto n = ToNarrow(this->name);
		outMsg.WriteData(n.c_str(), n.length());

		Write(outMsg);
	}

	void SendJSSync(const std::string& name, const std::string& json, const std::function<void(bool, const std::string&)>& fn)
	{
		IPC::SyncMessage outMsg(1, 0x2000A, IPC::Message::PRIORITY_NORMAL, new MyListener());
		outMsg.WriteString16(ToWide(name));
		outMsg.WriteString16(ToWide(json));

		auto n = ToNarrow(this->name);
		outMsg.WriteData(n.c_str(), n.length());

		Write(outMsg);

		jsReplies[IPC::SyncMessage::GetMessageId(outMsg)] = fn;
	}


	virtual bool SerializeOutputParameters(const IPC::Message& msg, base::PickleIterator iter) override
	{
		return false;
	}
};

class MockChannelReader : public IPC::internal::ChannelReader {
public:
	MockChannelReader(MyListener* listener)
		: ChannelReader(listener) , listener_(listener){}

	ReadState ReadData(char* buffer, int buffer_len, int* bytes_read) override {
		if (data_.empty())
			return READ_PENDING;

		size_t read_len = std::min(static_cast<size_t>(buffer_len), data_.size());
		memcpy(buffer, data_.data(), read_len);
		*bytes_read = static_cast<int>(read_len);
		data_.erase(0, read_len);
		return READ_SUCCEEDED;
	}

	bool ShouldDispatchInputMessage(IPC::Message* msg) override { return true; }

	bool GetAttachments(IPC::Message* msg) override { return true; }

	bool DidEmptyInputBuffers() override { return true; }

	void HandleInternalMessage(const IPC::Message& msg) override
	{
		listener_->Write(msg);

		auto id = msg.type();
		auto rid = msg.routing_id();
		auto data = msg.payload();

		if (listener_->name.find(L"_channel_") != std::string::npos)
		{
			listener_->SendJSCallback("RGSC_FINISHED_FUNCTION_BINDINGS", "");
		}
	}

	void AppendData(const void* data, size_t size) {
		data_.append(static_cast<const char*>(data), size);
	}

	void AppendMessageData(const IPC::Message& message) {
		AppendData(message.data(), message.size());
	}

private:
	std::string data_;
	MyListener* listener_;
};

static MyListener* RunListener(const std::wstring& a)
{
	auto hPipe = CreateFileW(fmt::sprintf(L"\\\\.\\pipe\\chrome.%s", a).c_str(), GENERIC_READ | GENERIC_WRITE,
		0,
		NULL,
		OPEN_EXISTING,
		SECURITY_SQOS_PRESENT | SECURITY_ANONYMOUS | FILE_FLAG_OVERLAPPED |
		0,
		NULL);

	MyListener* listener = new MyListener();
	listener->hPipe = hPipe;
	listener->name = a;

	std::thread([listener, hPipe]()
	{
		MockChannelReader reader(listener);

		auto ev = CreateEvent(NULL, FALSE, FALSE, NULL);

		while (true)
		{
			DWORD bytesRead = 0;
			uint8_t data[2048];

			OVERLAPPED overlapped = { 0 };
			overlapped.hEvent = ev;

			if (ReadFile(hPipe, data, sizeof(data), &bytesRead, &overlapped) || GetLastError() == ERROR_IO_PENDING)
			{
				if (GetLastError() == ERROR_IO_PENDING)
				{
					WaitForSingleObject(ev, INFINITE);

					GetOverlappedResult(hPipe, &overlapped, &bytesRead, FALSE);
				}

				reader.AppendData(data, bytesRead);

				reader.ProcessIncomingMessages();
			}
		}
	}).detach();

	return listener;
}

void SubprocessPipe(const std::wstring& s)
{
	auto a = s.substr(s.find(L"--rgsc_ipc_channel_name=") + 24);
	a = a.substr(0, a.find(L" "));

	RunListener(a);
}
