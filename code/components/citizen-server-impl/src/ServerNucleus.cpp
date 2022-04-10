#include <StdInc.h>

#include <CoreConsole.h>

#include <GameServer.h>
#include <HttpClient.h>

#include <ResourceEventComponent.h>
#include <ResourceManager.h>

#include <ServerInstanceBase.h>
#include <ServerInstanceBaseRef.h>

#include <ServerLicensingComponent.h>

#include <TcpListenManager.h>
#include <ReverseTcpServer.h>

#include <StructuredTrace.h>

#include <json.hpp>
#include <lua.hpp>
#include <lua_rapidjsonlib.h>

using json = nlohmann::json;

struct Lua
{
	Lua()
		: L(nullptr)
	{
		static const luaL_Reg lualibs[] = {
			{ "_G", luaopen_base },
			{ LUA_TABLIBNAME, luaopen_table },
			{ LUA_STRLIBNAME, luaopen_string },
			{ LUA_MATHLIBNAME, luaopen_math },
			{ LUA_UTF8LIBNAME, luaopen_utf8 },
			{ "json", luaopen_rapidjson },
			{ NULL, NULL }
		};

		L = luaL_newstate();

		const luaL_Reg* lib = lualibs;
		for (; lib->func; lib++)
		{
			luaL_requiref(L, lib->name, lib->func, 1);
			lua_pop(L, 1);
		}
	}

	~Lua()
	{
		if (L)
		{
			lua_close(L);
			L = nullptr;
		}
	}

	std::optional<bool> EvaluateExpression(const std::string& expr, const json& context)
	{
		if (luaL_loadbufferx(L, expr.c_str(), expr.size(), "@expr", "t") != LUA_OK)
		{
			return {};
		}

		// stack: [expr chunk]

		lua_getglobal(L, "json");
		lua_getfield(L, -1, "decode");
		lua_remove(L, -2);

		// stack: [expr chunk], [json.decode]

		auto j = context.dump(-1, ' ', false, nlohmann::detail::error_handler_t::ignore);
		lua_pushlstring(L, j.c_str(), j.size());
		if (lua_pcall(L, 1, 1, 0) != LUA_OK)
		{
			return {};
		}

		// stack: [expr chunk], [json table]

		// store _G in new env
		lua_pushglobaltable(L);
		lua_setfield(L, -2, "_G");

		// set _ENV to the JSON chunk
		lua_setupvalue(L, -2, 1);

		// stack: [expr chunk, with json table as env]
		if (lua_pcall(L, 0, 1, 0) != LUA_OK)
		{
			const char* e = lua_tostring(L, -1);
			return {};
		}

		// stack: retval
		auto rv = lua_toboolean(L, -1);
		lua_pop(L, 1);

		return rv;
	}

private:
	lua_State* L;
};

static std::optional<bool> EvaluateLua(const std::string& in, const json& context)
{
	Lua l;
	return l.EvaluateExpression(in, context);
}

static void DisplayNotices(fx::ServerInstanceBase* server, HttpClient* httpClient)
{
	httpClient->DoGetRequest("https://runtime.fivem.net/promotions_targeting.json", [server](bool success, const char* data, size_t length)
	{
		if (success)
		{
			json convarList = json::object();
			json resourceList = json::array();

			auto conCtx = server->GetComponent<console::Context>();
			conCtx->GetVariableManager()->ForAllVariables([&convarList](const std::string& name, int flags, const std::shared_ptr<internal::ConsoleVariableEntryBase>& variable)
			{
				convarList[name] = variable->GetValue();
			});

			auto resman = server->GetComponent<fx::ResourceManager>();
			resman->ForAllResources([&resourceList](const fwRefContainer<fx::Resource>& resource)
			{
				if (resource->GetState() == fx::ResourceState::Started)
				{
					resourceList.push_back(json::object({ { "name", resource->GetName() } }));
				}
			});

			json contextBlob = json::object();
			contextBlob["convar"] = convarList;
			contextBlob["resource"] = resourceList;

			try
			{
				json noticeBlob = json::parse(data, data + length);

				for (auto& [ noticeType, data ] : noticeBlob.get<json::object_t>())
				{
					auto& conditions = data["conditions_lua"];
					auto& actions = data["actions"];

					if (conditions.is_array())
					{
						for (auto& condition : conditions)
						{
							auto cond = condition.get<std::string>();
							if (cond.find("--[[]]") != 0)
							{
								cond = "return " + cond;
							}

							auto rv = EvaluateLua(cond, contextBlob);

							if (rv && *rv)
							{
								// evaluate actions
								if (actions.is_array())
								{
									auto noticeTypeStr = noticeType;

									gscomms_execute_callback_on_main_thread([actions, conCtx, noticeTypeStr]()
									{
										trace("^1-- [server notice: %s]^7\n", noticeTypeStr);

										se::ScopedPrincipal principalScope(se::Principal{ "system.console" });
										
										try
										{
											for (auto& action : actions)
											{
												conCtx->ExecuteSingleCommand(action.get<std::string>());
											}
										}
										catch (std::exception& e)
										{
										
										}

										trace("\n");
									});
								}
							}
						}
					}
				}
			}
			catch (std::exception& e)
			{
				trace("Notice error: %s\n", e.what());
			}
		}
	});
}

static InitFunction initFunction([]()
{
	static auto httpClient = new HttpClient();

	static ConsoleCommand printCmd("print", [](const std::string& str)
	{
		trace("%s\n", str);
	});

	fx::ServerInstanceBase::OnServerCreate.Connect([](fx::ServerInstanceBase* instance)
	{
		using namespace std::chrono_literals;

		static bool setNucleus = false;
		static bool setNucleusSuccess = false;
		static std::chrono::milliseconds setNucleusTimeout;

		instance->GetComponent<fx::GameServer>()->OnTick.Connect([instance]()
		{
			if (!setNucleusSuccess && (!setNucleus || (msec() > setNucleusTimeout)))
			{
				auto var = instance->GetComponent<console::Context>()->GetVariableManager()->FindEntryRaw("sv_licenseKeyToken");

				if (var && !var->GetValue().empty())
				{
					auto licensingComponent = instance->GetComponent<ServerLicensingComponent>();
					auto nucleusToken = licensingComponent->GetNucleusToken();

					if (!nucleusToken.empty())
					{
						auto tlm = instance->GetComponent<fx::TcpListenManager>();

						auto jsonData = nlohmann::json::object({
							{ "token", nucleusToken },
							{ "port", fmt::sprintf("%d", tlm->GetPrimaryPort()) },
							{ "ipOverride", instance->GetComponent<fx::GameServer>()->GetIpOverrideVar()->GetValue() },
						});

						static auto authDelay = 15s;

						setNucleusTimeout = msec() + authDelay;

						HttpRequestOptions opts;
						opts.ipv4 = true;

						httpClient->DoPostRequest("https://cfx.re/api/register/?v=2", jsonData.dump(), opts, [instance, tlm](bool success, const char* data, size_t length)
						{
							if (!success)
							{
								if (authDelay < 15min)
								{
									authDelay *= 2;
								}

								setNucleusTimeout = msec() + authDelay;
							}
							else
							{
								auto jsonData = nlohmann::json::parse(std::string(data, length));

								trace("^1        fff                          \n^1  cccc ff   xx  xx     rr rr    eee  \n^1cc     ffff   xx       rrr  r ee   e \n^1cc     ff     xx   ... rr     eeeee  \n^1 ccccc ff   xx  xx ... rr      eeeee \n                                     ^7\n");
								trace("^2Authenticated with cfx.re Nucleus: ^7https://%s/\n", jsonData.value("host", ""));

								fwRefContainer<net::ReverseTcpServer> rts = new net::ReverseTcpServer();
								rts->Listen("users.cfx.re:30130", jsonData.value("rpToken", ""));

								tlm->AddExternalServer(rts);

								instance->GetComponent<fx::ResourceManager>()
									->GetComponent<fx::ResourceEventManagerComponent>()
									->QueueEvent2(
										"_cfx_internal:nucleusConnected",
										{},
										fmt::sprintf("https://%s/", jsonData.value("host", ""))
									);

								StructuredTrace({ "type", "nucleus_connected" }, { "url", fmt::sprintf("https://%s/", jsonData.value("host", "")) });

								static auto webVar = instance->AddVariable<std::string>("web_baseUrl", ConVar_None, jsonData.value("host", ""));

								setNucleusSuccess = true;
							}

							DisplayNotices(instance, httpClient);
						});
					}

					setNucleus = true;
				}
			}
		});
	}, INT32_MAX);
});
