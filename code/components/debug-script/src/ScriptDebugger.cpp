#include <StdInc.h>

#include <tbb/concurrent_unordered_map.h>

#include <atomic>
#include <thread>

#include <ResourceManager.h>
#include <ResourceScriptingComponent.h>

#include <fxScripting.h>
#include <om/OMClass.h>

#include <src/uWS.h>

#include <json.hpp>

#include <PushEnvironment.h>
#include <VFSManager.h>

using json = nlohmann::json;

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

	uWS::Hub m_socketHub;

public:
	class Connection
	{
	public:
		Connection(ScriptDebugger* debugger, uWS::WebSocket<true>* socket, uWS::HttpRequest initReq)
			: m_debugger(debugger), m_socket(socket)
		{
			debugger->AttachConnection(this);
		}

		~Connection()
		{
			m_debugger->DetachConnection(this);
		}

		void HandleMessage(std::string_view data);

		void SendMethod(const std::string& method, const json& data);

	private:
		ScriptDebugger* m_debugger;

		uWS::WebSocket<true>* m_socket;
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

private:
	struct ExecutionContext
	{
		int id;
		fx::Resource* resource;
		fx::OMPtr<IScriptRuntime> runtime;
	};

	std::atomic<int> m_curExecContext;

	tbb::concurrent_unordered_map<int, ExecutionContext> m_execContexts;

	void SendExecutionContext(Connection* connection, const ExecutionContext& cxt);

public:
	void SendExecutionContexts(Connection* connection);

	int AddExecutionContext(fx::Resource* resource, fx::OMPtr<IScriptRuntime> rt);

	void RemoveExecutionContext(int context);

private:
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

	tbb::concurrent_unordered_map<int, ScriptMetaData> m_scripts;

public:

	int AddScript(fx::Resource* resource, const std::string& fileName);

	void SendScriptParsed(Connection* connection, const ScriptMetaData& data);

	void SendScriptsParsed(Connection* connection);

	std::string GetScriptSource(const std::string& id);
};

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

		auto dataStr = retval.dump();
		m_socket->send(dataStr.c_str(), dataStr.size(), uWS::OpCode::TEXT);
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
	m_socket->send(dataStr.c_str(), dataStr.size(), uWS::OpCode::TEXT);
}

ScriptDebugger::ScriptDebugger(fx::ResourceManager* resman)
	: m_resourceManager(resman), m_curExecContext(0)
{
	
}

void ScriptDebugger::RunThread()
{
	m_socketHub.onConnection([this](uWS::WebSocket<true>* ws, uWS::HttpRequest req)
	{
		ws->setUserData(new Connection(this, ws, req));
	});

	m_socketHub.onDisconnection([this](uWS::WebSocket<true>* ws, int code, char* message, size_t length)
	{
		auto connection = reinterpret_cast<Connection*>(ws->getUserData());
		delete connection;
	});

	m_socketHub.onMessage([this](uWS::WebSocket<true>* ws, char* msg, size_t len, uWS::OpCode opCode)
	{
		if (opCode == uWS::OpCode::TEXT)
		{
			auto connection = reinterpret_cast<Connection*>(ws->getUserData());
			connection->HandleMessage(std::string_view{ msg, len });
		}
	});

	m_socketHub.onHttpRequest([](uWS::HttpResponse* res, uWS::HttpRequest, char*, size_t, size_t)
	{
		res->end("", 0);
	});

	if (m_socketHub.listen(13173))
	{
		m_socketHub.run();
	}
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
	{

	}

public:
	NS_DECL_IDEBUGEVENTLISTENER;
};

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

			scripting->OnOpenScript.Connect([this, resource, scripting](const std::string& fileName)
			{
				int scriptId = AddScript(resource, fileName);
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
