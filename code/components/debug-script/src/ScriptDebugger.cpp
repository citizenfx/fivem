#include <StdInc.h>

#if !defined(_MSC_VER) || _MSC_VER < 1923
#include <tbb/concurrent_unordered_map.h>

#include <atomic>
#include <optional>
#include <thread>

#include <ResourceManager.h>
#include <ResourceScriptingComponent.h>

#include <fxScripting.h>
#include <om/OMClass.h>

#include <CoreConsole.h>

#undef trace
#include <src/App.h>

#include <json.hpp>

#include <PushEnvironment.h>
#include <VFSManager.h>

using json = nlohmann::json;

struct Location
{
	std::string scriptId;
	int lineNumber;
	int columnNumber;
};

void from_json(const json& j, Location& l)
{
	j.at("scriptId").get_to(l.scriptId);
	j.at("lineNumber").get_to(l.lineNumber);
	j.at("columnNumber").get_to(l.columnNumber);
}

struct BreakLocation
{
	std::string scriptId;
	int lineNumber;
};

void to_json(json& j, const BreakLocation& l)
{
	j = json{ {"scriptId", l.scriptId}, {"lineNumber", l.lineNumber } };
}

class ScriptDebugger
{
public:
	ScriptDebugger(fx::ResourceManager* resman);

	void Start();

	virtual ~ScriptDebugger() = default;

private:
	void RunThread();

private:
	fx::ResourceManager* m_resourceManager;

	std::thread m_listenThread;

public:
	class Connection
	{
	public:
		void init(ScriptDebugger* debugger, uWS::WebSocket<false, true>* socket, uWS::HttpRequest initReq)
		{
			m_debugger = debugger;
			m_socket = socket;

			debugger->AttachConnection(this);
		}

		void uninit()
		{
			m_debugger->DetachConnection(this);
		}

		void HandleMessage(std::string_view data);

		void SendMethod(const std::string& method, const json& data);

	private:
		ScriptDebugger* m_debugger;

		uWS::WebSocket<false, true>* m_socket;
	};

public:
	using TResultCB = std::function<void(bool, const json&)>;
	using THandler = std::function<void(Connection*, json&, const TResultCB&)>;

	static void RegisterHandler(const std::string& method, const THandler& handler);

private:
	std::set<Connection*> m_connections;

	static std::map<std::string, THandler>* ms_handlers;

private:
	inline void AttachConnection(Connection* conn)
	{
		m_connections.insert(conn);
	}

	inline void DetachConnection(Connection* conn)
	{
		m_connections.erase(conn);
	}

public:
	struct ExecutionContext
	{
		int id;
		fx::Resource* resource;
		fx::OMPtr<IScriptRuntime> runtime;
	};

private:
	std::atomic<int> m_curExecContext;

	tbb::concurrent_unordered_map<int, ExecutionContext> m_execContexts;

	void SendExecutionContext(Connection* connection, const ExecutionContext& cxt);

public:
	void SendExecutionContexts(Connection* connection);

	int AddExecutionContext(fx::Resource* resource, fx::OMPtr<IScriptRuntime> rt);

	void RemoveExecutionContext(int context);

	void AddBreakLocation(const BreakLocation& location);

	std::vector<BreakLocation> GetBreakLocationsBetween(const Location& start, const Location& end);

public:
	struct ScriptMetaData
	{
		int id;
		std::string fileName;
		int endLine;
		int endColumn;
		std::string contentHash;
		int execContext;
		int length;
	};

private:
	tbb::concurrent_unordered_map<int, ScriptMetaData> m_scripts;

	tbb::concurrent_unordered_map<std::string, std::set<int>> m_breakLocations;

public:

	int AddScript(fx::Resource* resource, const std::string& fileName);

	void SendScriptParsed(Connection* connection, const ScriptMetaData& data);

	void SendScriptsParsed(Connection* connection);

	std::string GetScriptSource(const std::string& id);

	std::optional<ScriptMetaData> GetScript(const std::string& id);

	std::optional<ExecutionContext> GetExecutionContext(int id);
};

auto ScriptDebugger::GetExecutionContext(int id) -> std::optional<ExecutionContext>
{
	auto it = m_execContexts.find(id);

	if (it != m_execContexts.end())
	{
		return it->second;
	}

	return {};
}

auto ScriptDebugger::GetScript(const std::string& id) -> std::optional<ScriptMetaData>
{
	auto it = m_scripts.find(std::stoi(id));

	if (it != m_scripts.end())
	{
		return it->second;
	}

	return {};
}

void ScriptDebugger::AddBreakLocation(const BreakLocation& location)
{
	m_breakLocations[location.scriptId].insert(location.lineNumber);
}

std::vector<BreakLocation> ScriptDebugger::GetBreakLocationsBetween(const Location& start, const Location& end)
{
	const auto& set = m_breakLocations[start.scriptId];
	std::vector<BreakLocation> vec;

	for (auto it = set.lower_bound(start.lineNumber); it != set.upper_bound(end.lineNumber); it++)
	{
		if (*it >= end.lineNumber)
		{
			continue;
		}

		vec.push_back(BreakLocation{ start.scriptId, *it });
	}

	return vec;
}

std::map<std::string, ScriptDebugger::THandler>* ScriptDebugger::ms_handlers;

void ScriptDebugger::RegisterHandler(const std::string& method, const THandler& handler)
{
	if (!ms_handlers)
	{
		ms_handlers = new std::map<std::string, THandler>();
	}

	(*ms_handlers)[method] = handler;
}

void ScriptDebugger::Connection::HandleMessage(std::string_view data)
{
	if (!ms_handlers)
	{
		ms_handlers = new std::map<std::string, THandler>();
	}

	json req = json::parse(data);

	std::string method = req.value("method", "");
	int id = req.value("id", 0);

	auto handler = ms_handlers->find(method);
	auto resultCB = [this, id](bool success, const json& data)
	{
		json retval = json{
			{ "id", id }
		};

		if (!success)
		{
			retval["error"] = data;
		}
		else
		{
			retval["result"] = data;
		}

		auto dataStr = retval.dump(-1, ' ', false, json::error_handler_t::ignore);
		m_socket->send(dataStr, uWS::OpCode::TEXT);
	};

	if (handler != ms_handlers->end())
	{
		auto params = json::object();

		auto paramsIt = req.find("params");

		if (paramsIt != req.end())
		{
			params = *paramsIt;
		}

		handler->second(this, params, resultCB);
	}
	else
	{
		resultCB(false, json{
			{"code", -32601},
			{"message", fmt::sprintf("'%s' wasn't found", method)}
		});
	}
}

void ScriptDebugger::Connection::SendMethod(const std::string& method, const json& data)
{
	json retval = json{
		{ "method", method },
		{ "params", data }
	};

	auto dataStr = retval.dump();
	m_socket->send(dataStr, uWS::OpCode::TEXT);
}

ScriptDebugger::ScriptDebugger(fx::ResourceManager* resman)
	: m_resourceManager(resman), m_curExecContext(0)
{
	
}

void ScriptDebugger::RunThread()
{
	uWS::App::WebSocketBehavior behavior;

	behavior.open = [this](auto* ws, auto* req)
	{
		auto connection = reinterpret_cast<Connection*>(ws->getUserData());
		connection->init(this, ws, *req);
	};

	behavior.close = [this](auto* ws, int code, std::string_view message)
	{
		auto connection = reinterpret_cast<Connection*>(ws->getUserData());
		connection->uninit();
	};

	behavior.message = [this](auto* ws, std::string_view msg, uWS::OpCode opCode)
	{
		if (opCode == uWS::OpCode::TEXT)
		{
			auto connection = reinterpret_cast<Connection*>(ws->getUserData());
			connection->HandleMessage(msg);
		}
	};

	uWS::App()
		.get("/json/list", [](uWS::HttpResponse<false>* res, auto* req)
		{
			res->writeHeader("content-type", "application/json; charset=UTF-8");

			res->end(json::array({
				json{
					{ "description", "cfx" },
					{ "title", "cfx" },
					{ "id", "fade" },
					{ "url", "file://"},
					{ "type", "page" },
					{ "webSocketDebuggerUrl", "ws://localhost:13173/meow" }
				}
			}).dump());
		})
		.ws<Connection>("/*", std::move(behavior)).listen(13173, [](auto*) {}).run();
}

int ScriptDebugger::AddExecutionContext(fx::Resource* resource, fx::OMPtr<IScriptRuntime> rt)
{
	int id = m_curExecContext++;
	auto ec = ExecutionContext{ id, resource, rt };

	m_execContexts[id] = ec;

	for (auto& connection : m_connections)
	{
		SendExecutionContext(connection, ec);
	}

	return id;
}

void ScriptDebugger::SendExecutionContext(Connection* connection, const ExecutionContext& cxt)
{
	connection->SendMethod("Runtime.executionContextCreated", json{
		{"context", json{
			{"id", cxt.id},
			{"origin", ""},
			{"name", fmt::sprintf("%s - %d", cxt.resource->GetName(), cxt.runtime->GetInstanceId())}
		}}
	});
}

void ScriptDebugger::SendExecutionContexts(Connection* connection)
{
	for (auto& cxt : m_execContexts)
	{
		SendExecutionContext(connection, cxt.second);
	}
}

int ScriptDebugger::AddScript(fx::Resource* resource, const std::string& fileName)
{
	fx::OMPtr<IScriptRuntime> rt;
	auto result = fx::GetCurrentScriptRuntime(&rt);

	int ecId = 0;

	for (auto& ecPair : m_execContexts)
	{
		if (ecPair.second.runtime.GetRef() == rt.GetRef())
		{
			ecId = ecPair.second.id;
			break;
		}
	}

	int id = m_curExecContext++;

	ScriptMetaData md;
	md.id = id;
	md.fileName = fileName;
	md.endLine = 999999;
	md.endColumn = 0;

	{
		auto stream = vfs::OpenRead(fileName);

		if (!stream.GetRef())
		{
			return -1;
		}

		md.length = stream->GetLength();

		auto d = stream->ReadToEnd();
		auto cHash = fmt::sprintf("%08X", HashRageString((char*)d.data()));;

		md.contentHash = cHash;
	}

	md.execContext = ecId;
	
	m_scripts[id] = md;

	for (auto& connection : m_connections)
	{
		SendScriptParsed(connection, md);
	}

	return id;
}

std::string ScriptDebugger::GetScriptSource(const std::string& id)
{
	int idInt = atoi(id.c_str());

	auto it = m_scripts.find(idInt);

	if (it != m_scripts.end())
	{
		auto stream = vfs::OpenRead(it->second.fileName);
		auto data = stream->ReadToEnd();

		return std::string((char*)data.data(), (char*)data.data() + data.size());
	}

	return "";
}

void ScriptDebugger::SendScriptParsed(Connection* connection, const ScriptMetaData& data)
{
	connection->SendMethod("Debugger.scriptParsed", json{
		{"scriptId", fmt::sprintf("%d", data.id)},
		{"url", data.fileName},
		{"startLine", 0},
		{"startColumn", 0},
		{"endLine", data.endLine},
		{"endColumn", data.endColumn},
		{"executionContextId", data.execContext},
		{"hash", data.contentHash},
		{"length", data.length},
		{"isLiveEdit", false},
		{"sourceMapURL", ""}, // TODO
		{"hasSourceURL", false},
		{"isModule", false}
	});
}

void ScriptDebugger::SendScriptsParsed(Connection* connection)
{
	for (auto& cxt : m_scripts)
	{
		SendScriptParsed(connection, cxt.second);
	}
}

class DebugEventListener : public fx::OMClass<DebugEventListener, IDebugEventListener>
{
public:
	inline DebugEventListener(ScriptDebugger* debugger, fx::Resource* resource, IScriptDebugRuntime* scRt)
		: m_debugger(debugger)
	{

	}

public:
	NS_DECL_IDEBUGEVENTLISTENER;

private:
	ScriptDebugger* m_debugger;
};

result_t DebugEventListener::OnBreakpointsDefined(int scriptId, char* blob)
{
	console::Printf("meh", "defined %d\n", scriptId);

	auto breakpointLines = json::parse(blob);
	BreakLocation location;

	for (int line : breakpointLines)
	{
		location.scriptId = std::to_string(scriptId);
		location.lineNumber = line;

		m_debugger->AddBreakLocation(location);
	}

	return FX_S_OK;
}

void ScriptDebugger::Start()
{
	fx::Resource::OnInitializeInstance.Connect([this](fx::Resource* resource)
	{
		resource->OnStart.Connect([this, resource]()
		{
			auto scripting = resource->GetComponent<fx::ResourceScriptingComponent>();
			scripting->OnCreatedRuntimes.Connect([this, resource, scripting]()
			{
				scripting->ForAllRuntimes([this, resource](fx::OMPtr<IScriptRuntime> rt)
				{
					int ec = AddExecutionContext(resource, rt);

					fx::OMPtr<IScriptDebugRuntime> dbRt;

					if (FX_SUCCEEDED(rt.As<IScriptDebugRuntime>(&dbRt)))
					{
						auto listener = fx::MakeNew<DebugEventListener>(this, resource, dbRt.GetRef());
						dbRt->SetDebugEventListener(listener.GetRef());
					}
				});
			});

			scripting->OnOpenScript.Connect([this, resource, scripting](const std::string& fileName, const std::string& tempHackReferenceName)
			{
				int scriptId = AddScript(resource, fileName);

				auto script = GetScript(fmt::sprintf("%d", scriptId));

				if (script)
				{
					auto ec = GetExecutionContext(script->execContext);

					if (ec)
					{
						fx::OMPtr<IScriptDebugRuntime> debugRuntime;

						if (FX_SUCCEEDED(ec->runtime.As(&debugRuntime)))
						{
							console::Printf("ok", "%s: %d\n", const_cast<char*>(fileName.c_str()), scriptId);
							debugRuntime->SetScriptIdentifier(const_cast<char*>(tempHackReferenceName.c_str()), scriptId);
						}
					}
				}
			});
		}, -1000000);

		resource->OnStop.Connect([this, resource]()
		{
			// #TODOSD: remove execution contexts
		}, INT32_MIN);
	});

	m_listenThread = std::thread{ [this]()
	{
		RunThread();
	} };

	// for now
	m_listenThread.detach();
}

static std::unique_ptr<ScriptDebugger> g_scriptDebugger;

static InitFunction initFunction([]()
{
	fx::ResourceManager::OnInitializeInstance.Connect([](fx::ResourceManager* resman)
	{
		g_scriptDebugger = std::make_unique<ScriptDebugger>(resman);
		g_scriptDebugger->Start();
	});
});

struct ScriptDebuggerHandler
{
	ScriptDebuggerHandler(const std::string& method, const ScriptDebugger::THandler& handler)
	{
		ScriptDebugger::RegisterHandler(method, handler);
	}
};

static ScriptDebuggerHandler _profilerEnable("Profiler.enable", [](ScriptDebugger::Connection*, json& data, const ScriptDebugger::TResultCB& cb)
{
	cb(true, json::object());
});

static ScriptDebuggerHandler _runtimeEnable("Runtime.enable", [](ScriptDebugger::Connection* connection, json& data, const ScriptDebugger::TResultCB& cb)
{
	g_scriptDebugger->SendExecutionContexts(connection);

	cb(true, json::object());
});

static ScriptDebuggerHandler _debuggerEnable("Debugger.enable", [](ScriptDebugger::Connection* connection, json& data, const ScriptDebugger::TResultCB& cb)
{
	// TODO: return `Debugger.scriptParsed` events for every script file
	// should this be every _source_ file or every _assembly_ file?
	g_scriptDebugger->SendScriptsParsed(connection);

	cb(true, json::object());
});

static ScriptDebuggerHandler _debuggerSetPause("Debugger.setPauseOnExceptions", [](ScriptDebugger::Connection*, json& data, const ScriptDebugger::TResultCB& cb)
{
	cb(true, json::object());
});

static ScriptDebuggerHandler _debuggerSetAsyncCallStackDepth("Debugger.setAsyncCallStackDepth", [](ScriptDebugger::Connection*, json& data, const ScriptDebugger::TResultCB& cb)
{
	cb(true, json::object());
});

static ScriptDebuggerHandler _debuggerSetBlackBoxPatterns("Debugger.setBlackboxPatterns", [](ScriptDebugger::Connection*, json& data, const ScriptDebugger::TResultCB& cb)
{
	cb(true, json::object());
});

static ScriptDebuggerHandler _debuggerRunIfWaiting("Runtime.runIfWaitingForDebugger", [](ScriptDebugger::Connection*, json& data, const ScriptDebugger::TResultCB& cb)
{
	cb(true, json::object());
});

static ScriptDebuggerHandler _debuggerGetScriptSource("Debugger.getScriptSource", [](ScriptDebugger::Connection*, json& data, const ScriptDebugger::TResultCB& cb)
{
	cb(true, json{
		{"scriptSource", g_scriptDebugger->GetScriptSource(data.value("scriptId", "0"))}
	});
});

static ScriptDebuggerHandler _debuggerGetPossibleBreakpoints("Debugger.getPossibleBreakpoints", [](ScriptDebugger::Connection*, json& data, const ScriptDebugger::TResultCB& cb)
{
	Location start = data["start"];
	Location end = data["end"];

	auto breakLocations = g_scriptDebugger->GetBreakLocationsBetween(start, end);

	cb(true, json{
		{"locations", breakLocations}
	});
});

static ScriptDebuggerHandler _debuggerSetBreakpointsActive("Debugger.setBreakpointsActive", [](ScriptDebugger::Connection*, json& data, const ScriptDebugger::TResultCB& cb)
{
	cb(true, json::object());
});

static ScriptDebuggerHandler _debuggerSetBreakpointByUrl("Debugger.setBreakpointByUrl", [](ScriptDebugger::Connection*, json& data, const ScriptDebugger::TResultCB& cb)
{
	cb(true, json::object({
		{ "breakpointId", "11111" },
		{ "locations", json::array() }
	}));
});
#endif
