#include <StdInc.h>
#include <ExtDownloader.h>

#include <CfxSubProcess.h>
#include <IteratorView.h>

#include <websocketpp/config/asio_no_tls_client.hpp>
#include <websocketpp/client.hpp>

#include <json.hpp>

#include <ToolComponentHelpers.h>

#include <EnvironmentBlockHelpers.h>

#include <boost/filesystem.hpp>

#include <boost/random/random_device.hpp>
#include <boost/uuid/random_generator.hpp>
#include <boost/uuid/uuid_io.hpp>

#include <tbb/concurrent_unordered_map.h>

using client = websocketpp::client<websocketpp::config::asio_client>;
using message_ptr = websocketpp::config::asio_client::message_type::ptr;

using json = nlohmann::json;

static std::string g_uuid;

class ExtDownloaderJsonRpcClient
{
public:
	ExtDownloaderJsonRpcClient()
	{
		m_reqThread = std::thread([this]()
		{
			SetThreadName(-1, "ExtDownloader");

			Sleep(1500);

			try
			{
				c.init_asio();

				c.set_message_handler([this](websocketpp::connection_hdl hdl, message_ptr msg)
				{
					if (msg->get_opcode() == websocketpp::frame::opcode::TEXT)
					{
						try
						{
							auto jsonData = json::parse(msg->get_payload());

							if (jsonData.find("id") == jsonData.end())
							{
								OnNotification(jsonData.value("method", ""), jsonData["params"]);
							}
							else
							{
								OnReply(jsonData.value("id", ""), jsonData["result"]);
							}
						}
						catch (std::exception& e)
						{
							trace("extdl err: %s\ndata: %s\n", e.what(), msg->get_payload());
						}
					}
				});

				websocketpp::lib::error_code ec;
				auto con = c.get_connection(fmt::sprintf("ws://127.0.0.1:%d/jsonrpc", 6673), ec);

				if (!ec)
				{
					c.connect(con);

					m_connectionHandle = con->get_handle();
				}

				c.run();
			}
			catch (std::exception& e)
			{
				trace("extdl err: %s\n", e.what());
			}
		});
	}

	~ExtDownloaderJsonRpcClient()
	{
		c.stop();

		if (m_reqThread.joinable())
		{
			m_reqThread.join();
		}
	}

public:
	using TNotificationHandler = std::function<void(const json&)>;
	using TReplyHandler = std::function<void(const json&)>;

	void RegisterNotificationHandler(const std::string& method, const TNotificationHandler& handler);

	void SendRequest(const std::string& method, const json& params, const TReplyHandler& cb);

private:
	void OnNotification(const std::string& method, const json& params);

	void OnReply(const std::string& id, const json& params);

	client c;

	std::thread m_reqThread;

	websocketpp::connection_hdl m_connectionHandle;

	tbb::concurrent_unordered_multimap<std::string, TNotificationHandler> m_notificationHandlers;

	tbb::concurrent_unordered_map<std::string, TReplyHandler> m_replyHandlers;

	static std::atomic<int> ms_currentId;
};

void ExtDownloaderJsonRpcClient::OnNotification(const std::string& method, const json& params)
{
	auto range = fx::GetIteratorView(m_notificationHandlers.equal_range(method));

	for (auto& handler : range)
	{
		handler.second(params);
	}
}

void ExtDownloaderJsonRpcClient::OnReply(const std::string& id, const json& params)
{
	auto handler = m_replyHandlers.find(id);

	if (handler != m_replyHandlers.end())
	{
		handler->second(params);
		handler->second = {};
	}
}

void ExtDownloaderJsonRpcClient::RegisterNotificationHandler(const std::string& method, const TNotificationHandler& handler)
{
	m_notificationHandlers.insert({ method, handler });
}

void ExtDownloaderJsonRpcClient::SendRequest(const std::string& method, const json& params, const TReplyHandler& cb)
{
	auto id = ms_currentId++;
	auto idStr = fmt::sprintf("cfx_%d", id);

	auto data = json::object({
		{"jsonrpc", "2.0"},
		{"id", idStr},
		{"method", method},
		{"params", params}
	});

	m_replyHandlers.insert({ idStr, cb });

	websocketpp::lib::error_code ec;
	c.send(m_connectionHandle, data.dump(-1, ' ', false, nlohmann::detail::error_handler_t::replace), websocketpp::frame::opcode::text, ec);
}

std::atomic<int> ExtDownloaderJsonRpcClient::ms_currentId = 0;

class ExtDownloaderImpl : public ExtDownloader
{
public:
	ExtDownloaderImpl()
		: m_shutdown(false)
	{
		m_pumpThread = std::thread([this]()
		{
			SetThreadName(-1, "ExtDownloadPump");

			while (!m_shutdown)
			{
				using namespace std::chrono_literals;

				std::this_thread::sleep_for(500ms);

				for (auto& download : m_downloads)
				{
					if (!std::get<1>(download.second))
					{
						continue;
					}

					m_client.SendRequest("aria2.tellStatus", json::array({
						g_uuid,
						download.first,
						json::array({
							"status",
							"totalLength",
							"completedLength",
							"downloadSpeed",
							"errorCode",
							"errorMessage"
						})
					}), [this, download](const json& result)
					{
						if (result.is_null())
						{
							return;
						}

						std::string status = result.value("status", "");

						if (status == "waiting" || status == "active")
						{
							ProgressInfo pi;
							pi.downloadNow = strtoull(result["completedLength"].get<std::string>().c_str(), nullptr, 10);
							pi.downloadTotal = strtoull(result["totalLength"].get<std::string>().c_str(), nullptr, 10);

							std::get<0>(download.second).progressCallback(pi);
						}
					});
				}
			}
		});

		m_client.RegisterNotificationHandler("aria2.onDownloadComplete", [this](const json& reply)
		{
			auto gid = reply[0].value("gid", "");

			auto it = m_downloads.find(gid);

			if (it != m_downloads.end())
			{
				auto& download = *it;

				std::get<1>(download.second)();

				m_downloads[download.first] = {};
			}
		});
	}

	~ExtDownloaderImpl()
	{
		m_shutdown = true;

		if (m_pumpThread.joinable())
		{
			m_pumpThread.join();
		}
	}

	virtual std::string StartDownload(const std::string& url, const std::string& filePath, const HttpRequestOptions& options, const std::function<void()>& doneCb) override
	{
		auto gid = GenerateGid();

		auto path = boost::filesystem::path(filePath);

		auto headers = json::array();

		for (auto& header : options.headers)
		{
			headers.push_back(fmt::sprintf("%s: %s", header.first, header.second));
		}

		m_client.SendRequest("aria2.addUri", json::array({
				g_uuid,
				json::array({ url }),
				json::object({
					{ "out", path.filename().string() },
					{ "dir", path.parent_path().string() },
					{ "auto-file-renaming", "false" },
					{ "always-resume", "false" },
					{ "remove-control-file", "true" },
					{ "allow-overwrite", "true" },
					{ "header", headers },
					{ "gid", gid }
				}),
				999999
			}), [this, gid, options, doneCb](const json& result)
		{
			if (result.get<std::string>() == gid)
			{
				m_downloads.insert({ gid, { options, doneCb, options.weight } });

				ReorderDownloads();
			}
		});

		return gid;
	}

	virtual void SetRequestWeight(const std::string& gid, int newWeight) override
	{
		auto it = m_downloads.find(gid);

		if (it != m_downloads.end())
		{
			if (newWeight == -1)
			{
				newWeight = std::get<2>(it->second);
			}

			if (newWeight != std::get<0>(it->second).weight)
			{
				std::get<0>(it->second).weight = newWeight;

				ReorderDownloads();
			}
		}
	}

private:
	using DownloadEntry = std::tuple<HttpRequestOptions, std::function<void()>, int>;

private:
	void ReorderDownloads()
	{
		std::vector<std::reference_wrapper<std::pair<const std::string, DownloadEntry>>> downloads;

		for (auto& dl : m_downloads)
		{
			if (std::get<1>(dl.second))
			{
				downloads.push_back(dl);
			}
		}

		std::sort(downloads.begin(), downloads.end(), [](const auto& left, const auto& right)
		{
			return (std::get<0>(right.get().second).weight < std::get<0>(left.get().second).weight);
		});

		for (int i = 0; i < downloads.size(); i++)
		{
			m_client.SendRequest("aria2.changePosition", json::array({
					g_uuid,
					downloads[i].get().first,
					i,
					"POS_SET"
				}), [](const json& result)
			{
				
			});
		}
	}

	std::string GenerateGid()
	{
		return fmt::sprintf("%016llx", ms_gid++);
	}

	tbb::concurrent_unordered_map<std::string, DownloadEntry> m_downloads;

	static std::atomic<int> ms_gid;

	ExtDownloaderJsonRpcClient m_client;

	volatile bool m_shutdown;

	std::thread m_pumpThread;
};

std::atomic<int> ExtDownloaderImpl::ms_gid = 1;

std::shared_ptr<ExtDownloader> CreateExtDownloader()
{
	return std::make_shared<ExtDownloaderImpl>();
}

static void StartExtDownloader()
{
	if (GetFileAttributes(MakeRelativeCitPath(L"bin\\aria2c.exe").c_str()) == INVALID_FILE_ATTRIBUTES)
	{
		return;
	}

	// parse the existing environment block
	EnvironmentMap environmentMap;

	{
		wchar_t* environmentStrings = GetEnvironmentStrings();

		ParseEnvironmentBlock(environmentStrings, environmentMap);

		FreeEnvironmentStrings(environmentStrings);
	}

	// insert tool mode value
	environmentMap[L"CitizenFX_ToolMode"] = L"1";

	// output the environment into a new block
	std::vector<wchar_t> newEnvironment;

	BuildEnvironmentBlock(environmentMap, newEnvironment);

	// create a new application name
	g_uuid = "token:" + boost::uuids::to_string(boost::uuids::basic_random_generator<boost::random_device>()());

	// set the command line
	const wchar_t* newCommandLine = va(L"\"%s\" --check-certificate=false --stop-with-process=%d --enable-rpc=true --rpc-listen-port=6673 --rpc-secret=%s --console-log-level=warn", MakeRelativeCitPath(L"bin\\aria2c.exe").c_str(), GetCurrentProcessId(), ToWide(g_uuid.substr(6)));

	// and go create the new fake process
	PROCESS_INFORMATION pi;
	STARTUPINFO si = { 0 };
	si.cb = sizeof(STARTUPINFO);

	CreateProcessW(nullptr, const_cast<wchar_t*>(newCommandLine), nullptr, nullptr, FALSE, CREATE_UNICODE_ENVIRONMENT | CREATE_NO_WINDOW, &newEnvironment[0], MakeRelativeCitPath(L"").c_str(), &si, &pi);
}

static HookFunction hookFunction([]()
{
	StartExtDownloader();
});
