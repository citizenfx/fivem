#include <StdInc.h>

#include <catch_amalgamated.hpp>
#include <filesystem>

#include <lua.hpp>
#include <LuaFXLib.h>
#include <lua_cmsgpacklib.h>
#include <lua_rapidjsonlib.h>

#include "LuaScriptRuntime.h"
#include "RelativeDevice.h"
#include "ResourceManager.h"
#include "VFSManager.h"

#ifdef IS_FXSERVER
// vfs manager
#include "Manager.h"
#endif

//#undef LuaScriptRuntime
#define LUA_LIB

#if LUA_VERSION_NUM == 504
#define LUA_INTERNAL_LINKAGE "C++"
#else
#define LUA_INTERNAL_LINKAGE "C"
#endif

// Utility macro for the constexpr if statement
#define LUA_IF_CONSTEXPR if constexpr

// Inline utility
#if !defined(LUA_INLINE)
#ifdef _MSC_VER
#ifndef _DEBUG
#define LUA_INLINE __forceinline
#else
#define LUA_INLINE
#endif
#elif __has_attribute(__always_inline__)
#define LUA_INLINE inline __attribute__((__always_inline__))
#else
#define LUA_INLINE inline
#endif
#endif

#if LUA_VERSION_NUM >= 504 && defined(_WIN32)
#define LUA_USE_RPMALLOC
#endif

// tests have to use lua54 because it gets linked second so the LuaScriptRuntime is for lua54

namespace
{
// lua is static linked, lualibs array from script runtime won't work
static const luaL_Reg lualibs[] = {
	{"_G", luaopen_base},
	{LUA_TABLIBNAME, luaopen_table},
	{LUA_STRLIBNAME, luaopen_string},
	{LUA_MATHLIBNAME, luaopen_math},
	{LUA_DBLIBNAME, luaopen_debug},
	{LUA_COLIBNAME, luaopen_coroutine},
	{LUA_UTF8LIBNAME, luaopen_utf8},
#ifdef IS_FXSERVER
	{LUA_FX_IOLIBNAME, fx::lua_fx_openio},
	{LUA_FX_OSLIBNAME, fx::lua_fx_openos},
#endif
	{"msgpack", luaopen_cmsgpack},
	{"json", luaopen_rapidjson},
	{nullptr, nullptr}
};

// todo: add method to execute lua code from inside LuaScriptRuntime to prevent static link issues

void RuntimeSetup(fx::LuaStateHolder& state)
{
	// safe_openlibs
	const luaL_Reg* lib = lualibs;
	for (; lib->func; lib++)
	{
		luaL_requiref(state, lib->name, lib->func, 1);
		lua_pop(state, 1);
	}

	/*{
		// 0
		lua_getglobal(m_state, "debug");

		// 1
		lua_getfield(m_state, -1, "traceback");

		// 2
		m_dbTraceback = lua_tocfunction(m_state, -1);
		lua_pop(m_state, 2);

		// 0
	}*/

	// register the 'Citizen' library
	lua_newtable(state);
	luaL_setfuncs(state, fx::LuaScriptRuntime::GetCitizenLibs(), 0);
	lua_setglobal(state, "Citizen");

	// load the system scheduler script
	/*result_t hr;

	if (FX_FAILED(hr = LoadNativesBuild(nativesBuild)))
	{
		return hr;
	}

	if (FX_FAILED(hr = LoadSystemFile("citizen:/scripting/lua/deferred.lua")))
	{
		return hr;
	}

	if (FX_FAILED(hr = LoadSystemFile("citizen:/scripting/lua/scheduler.lua")))
	{
		return hr;
	}

	// Graph script loaded into Citizen.Graph
	// @TODO: Only load graphing utility on Lua_Require
	if (FX_FAILED(hr = LoadSystemFile("citizen:/scripting/lua/graph.lua")))
	{
		return hr;
	}*/

	lua_pushnil(state);
	lua_setglobal(state, "dofile");

	lua_pushnil(state);
	lua_setglobal(state, "loadfile");

	// todo: add print and require support

	//lua_pushcfunction(state, Lua_Print);
	//lua_setglobal(state, "print");

	//lua_pushcfunction(state, Lua_Require);
	//lua_setglobal(state, "require");
}

//void RuntimeDestroy()
//{
// we need to push the environment before closing as items may have __gc callbacks requiring a current runtime to be set
// in addition, we can't do this in the destructor due to refcounting oddities (PushEnvironment adds a reference, causing infinite deletion loops)
//LuaPushEnvironment pushed(this);
//	m_state.Close();
//}

void LoadAndRunCode(fx::LuaStateHolder& state, const std::string&& fileName, const std::string&& code,
                    bool expectExecutionError = false)
{
	// create a chunk name prefixed with @ (suppresses '[string "..."]' formatting)
	fwString chunkName("@");
	chunkName.append(fileName);

	if (luaL_loadbuffer(state, code.c_str(), code.size(), chunkName.c_str()) != 0)
	{
		// seemingly, it failed...
		trace(std::string("Could not parse lua code") + luaL_checkstring(state, -1));
		lua_pop(state, 1);
		REQUIRE((std::string("Could not parse lua code") + luaL_checkstring(state, -1)).empty());
	}

	if (lua_pcall(state, 0, 0, 0) != LUA_OK)
	{
		if (!expectExecutionError)
		{
			trace(std::string("Error executing Lua code: ") + lua_tostring(state, -1));
			REQUIRE((std::string("Error executing Lua code: ") + lua_tostring(state, -1)).empty());
			REQUIRE(false);
		}
		lua_pop(state, 1);
	}
	else
	{
		REQUIRE(!expectExecutionError);
	}
}
}

// todo: emulate threading

TEST_CASE("lua run")
{
	REQUIRE(LUA_VERSION_NUM == 504);
	fx::LuaStateHolder state;
	RuntimeSetup(state);

	LoadAndRunCode(state, "main.lua", R""""(
		local test = {}
		msgpack.pack(test)
		print("Hello Test")
		globalTest = {}
		globalTest.arg1 = "123"
		--Citizen.CreateThread(function()
		--    Citizen.Wait(1000)
		--end)
		
)"""");

	{
		// 0
		lua_getglobal(state, "globalTest");
		REQUIRE(lua_istable(state, -1) == true);
		// 1
		lua_getfield(state, -1, "arg1");
		REQUIRE(lua_isstring(state, -1) == true);
		// 2
		size_t l{0};
		const char* str = lua_tolstring(state, -1, &l);
		REQUIRE(l == 3);
		REQUIRE(std::string_view(str, l) == "123");
		lua_pop(state, 2);
	}
}

TEST_CASE("read none existing file")
{
	fx::LuaStateHolder state;
	RuntimeSetup(state);

	// todo: emulate file system

	// todo: switch to resource api for file reads

	LoadAndRunCode(state, "main.lua", R""""(
		local file = io.open("test.txt", "rb")
		if not file then 
			result = "noFile"
			return
		end
		result = file:read "*a"
		file:close()
)"""");

	{
		lua_getglobal(state, "result");
		REQUIRE(lua_isstring(state, -1) == true);
		size_t l{0};
		const char* str = lua_tolstring(state, -1, &l);
		REQUIRE(std::string_view(str, l) == "noFile");
	}
}

TEST_CASE("remove a file")
{
	fx::LuaStateHolder state;
	RuntimeSetup(state);

	// todo: emulate file system

	// todo: switch to resource api for file reads

	// todo: allow removes for files inside the resource directory

	LoadAndRunCode(state, "main.lua", R""""(
		noResult, errMessage, errNumber = os.remove("test.txt")
)"""");

	{
		lua_getglobal(state, "errNumber");
		REQUIRE(lua_isnumber(state, -1) == true);
		int isNum{0};
		lua_Number num = lua_tonumberx(state, -1, &isNum);
		REQUIRE(isNum);
		REQUIRE(static_cast<uint8_t>(num) == fx::Lua_EACCES);
	}
}

TEST_CASE("rename a file")
{
	fx::LuaStateHolder state;
	RuntimeSetup(state);

	std::filesystem::path currentPath = std::filesystem::current_path();
	currentPath = currentPath.append("tests");
	if (!std::filesystem::exists(currentPath))
	{
		if (std::filesystem::create_directory(currentPath))
		{
			REQUIRE(true);
		}
	}

	fwRefContainer relativeDevice = new vfs::RelativeDevice(currentPath.string() + "/");
	std::string mountPath = "@test/";
	vfs::Unmount(mountPath);
	vfs::Mount(relativeDevice, mountPath);

	vfs::Create("@test/rename.txt", false);

	WHEN ("a file is trying to be renamed that does not exists")
	{
		LoadAndRunCode(state, "main.lua", R""""(
		noResult, errMessage, errNumber = os.rename("404.txt", "503.txt")
)"""");

		THEN("EACCES is returned")
		{
			lua_getglobal(state, "errNumber");
			REQUIRE(lua_isnumber(state, -1) == true);
			int isNum{0};
			int64_t num = static_cast<int64_t>(lua_tonumberx(state, -1, &isNum));
			REQUIRE(isNum);
			REQUIRE(num == EACCES);
			lua_pop(state, 1);
		}
	}

	/*WHEN ("a file is being renamed that does exists")
	{
		LoadAndRunCode(state, "main.lua", std::string(R""""(
		noResult, errMessage, errNumber = os.rename("@test/rename.txt", "@test/rename2.txt")
		noResult, errMessage, errNumber = os.rename("@test/rename2.txt", "@test/rename.txt")
)""""));

		THEN("true is returned")
		{
			lua_getglobal(state, "noResult");
			REQUIRE(lua_isnil(state, -1) == true);
			lua_pop(state, 1);
		}
	}*/
}

TEST_CASE("generate temp file name")
{
	fx::LuaStateHolder state;
	RuntimeSetup(state);

	LoadAndRunCode(state, "main.lua", R""""(
		result = os.tmpname()
)"""");

	{
		lua_getglobal(state, "result");
		REQUIRE(lua_isstring(state, -1) == true);
		size_t l{0};
		const char* str = lua_tolstring(state, -1, &l);
		REQUIRE(std::string_view(str, l).rfind("tmp_", 0) == 0);
	}
}

TEST_CASE("lua os.getenv")
{
	fx::LuaStateHolder state;
	RuntimeSetup(state);

	LoadAndRunCode(state, "main.lua", R""""(
		result = os.getenv("OS")
		noResult = os.getenv("%PATH%")
)"""");

	{
		lua_getglobal(state, "result");
		REQUIRE(lua_isstring(state, -1) == true);
		size_t l{0};
		const char* str = lua_tolstring(state, -1, &l);
#ifdef WIN32
		REQUIRE(std::string_view(str, l) == "Windows");
#else
		REQUIRE(std::string_view(str, l) == "Linux");
#endif
		lua_pop(state, 1);
	}
	
	LoadAndRunCode(state, "main.lua", R""""(
		result = os.getenv("Os")
		noResult = os.getenv("%PATH%")
)"""");

	{
		lua_getglobal(state, "result");
		REQUIRE(lua_isstring(state, -1) == true);
		size_t l{0};
		const char* str = lua_tolstring(state, -1, &l);
#ifdef WIN32
		REQUIRE(std::string_view(str, l) == "Windows");
#else
		REQUIRE(std::string_view(str, l) == "Linux");
#endif
		lua_pop(state, 1);
	}

	{
		lua_getglobal(state, "noResult");
		REQUIRE(lua_isnil(state, -1) == true);
	}
}

TEST_CASE("lua os.setlocale")
{
	fx::LuaStateHolder state;
	RuntimeSetup(state);
	LoadAndRunCode(state, "main.lua", R""""(
		result = os.setlocale("en-US", "all")
)"""");

	{
		lua_getglobal(state, "result");
		REQUIRE(lua_isstring(state, -1) == true);
		size_t l{0};
		const char* str = lua_tolstring(state, -1, &l);
		REQUIRE(std::string_view(str, l) == "en-US");
		lua_pop(state, 1);
	}
}

TEST_CASE("lua os.exit")
{
	fx::LuaStateHolder state;
	RuntimeSetup(state);
	LoadAndRunCode(state, "main.lua", R""""(
		os.exit(1)
)"""", true);
}

TEST_CASE("lua os.time")
{
	fx::LuaStateHolder state;
	RuntimeSetup(state);
	
	WHEN ("os.time is called with no args")
	{
		auto timeStart = time(nullptr);
		LoadAndRunCode(state, "main.lua", R""""(
		result = os.time()
)"""");
		auto timeEnd = time(nullptr);

		THEN("it is returning the timestamp")
		{
			lua_getglobal(state, "result");
			REQUIRE(lua_isstring(state, -1) == true);
			size_t l{0};
			const char* str = lua_tolstring(state, -1, &l);
			int ts = std::stoi(str);
			REQUIRE(timeStart <= ts);
			REQUIRE(ts <= timeEnd);
			lua_pop(state, 1);
		}
	}

	WHEN ("os.time is called with args")
	{
		LoadAndRunCode(state, "main.lua", R""""(
		result = os.time({year=2000, month=1, day=1, hour=0, min=0, sec=0, isdst=false})
)"""");

		THEN("it is returning the timestamp for the date")
		{
			lua_getglobal(state, "result");
			REQUIRE(lua_isnil(state, -1) == false);
			REQUIRE(lua_isnumber(state, -1) == true);
			std::time_t timestamp = static_cast<std::time_t>(lua_tonumber(state, -1));
			std::tm tm {};
			static constexpr int64_t yearOffset = 1900;
			static constexpr int64_t monthOffset = 1;
			tm.tm_year = 2000 - yearOffset;
			tm.tm_mon = 1 - monthOffset;
			tm.tm_mday = 1;
			const std::time_t calculatedTimestamp = std::mktime(&tm);
			REQUIRE(calculatedTimestamp == timestamp);
			lua_pop(state, 1);
		}
	}
}

TEST_CASE("lua os.date")
{
	fx::LuaStateHolder state;
	RuntimeSetup(state);

	WHEN ("os.date is called with *t")
	{
		LoadAndRunCode(state, "main.lua", R""""(
		result = os.date("*t")
)"""");

		THEN ("result is a table with the date fields in local time")
		{
			// comparison with current time would result in a possible mismatch when time changes in execution
			const time_t t = std::time(nullptr);
			const std::tm* stm = std::localtime(&t);
			
			lua_getglobal(state, "result");
			REQUIRE(lua_istable(state, -1) == true);
			lua_getfield(state, -1, "year");
			REQUIRE(lua_isnumber(state, -1) == true);
			//REQUIRE(static_cast<int64_t>(lua_tonumber(state, -1)) == stm->tm_year + 1900);
			lua_pop(state, 1);
			lua_getfield(state, -1, "month");
			REQUIRE(lua_isnumber(state, -1) == true);
			//REQUIRE(static_cast<int64_t>(lua_tonumber(state, -1)) == stm->tm_mon + 1);
			lua_pop(state, 1);
			lua_getfield(state, -1, "day");
			REQUIRE(lua_isnumber(state, -1) == true);
			//REQUIRE(static_cast<int64_t>(lua_tonumber(state, -1)) == stm->tm_mday);
			lua_pop(state, 1);
			lua_getfield(state, -1, "hour");
			REQUIRE(lua_isnumber(state, -1) == true);
			//REQUIRE(static_cast<int64_t>(lua_tonumber(state, -1)) == stm->tm_hour);
			lua_pop(state, 1);
			lua_getfield(state, -1, "min");
			REQUIRE(lua_isnumber(state, -1) == true);
			//REQUIRE(static_cast<int64_t>(lua_tonumber(state, -1)) == stm->tm_min);
			lua_pop(state, 1);
			lua_getfield(state, -1, "sec");
			REQUIRE(lua_isnumber(state, -1) == true);
			//REQUIRE(static_cast<int64_t>(lua_tonumber(state, -1)) == stm->tm_sec);
			lua_pop(state, 1);
			lua_getfield(state, -1, "wday");
			REQUIRE(lua_isnumber(state, -1) == true);
			//REQUIRE(static_cast<int64_t>(lua_tonumber(state, -1)) == stm->tm_wday + 1);
			lua_pop(state, 1);
			lua_getfield(state, -1, "yday");
			REQUIRE(lua_isnumber(state, -1) == true);
			//REQUIRE(static_cast<int64_t>(lua_tonumber(state, -1)) == stm->tm_yday + 1);
			lua_pop(state, 1);
			lua_getfield(state, -1, "isdst");
			REQUIRE(lua_isboolean(state, -1) == true);
			//REQUIRE(lua_toboolean(state, -1) == stm->tm_isdst);
			lua_pop(state, 1);
			lua_pop(state, 1);
		}
	}

	WHEN ("os.date is called with !*t")
	{
		LoadAndRunCode(state, "main.lua", R""""(
		result = os.date("!*t")
)"""");

		THEN ("result is a table with the date fields in gm time")
		{
			// comparison with current time would result in a possible mismatch when time changes in execution
			const time_t t = std::time(nullptr);
			const std::tm* stm = std::gmtime(&t);
			
			lua_getglobal(state, "result");
			REQUIRE(lua_istable(state, -1) == true);
			lua_getfield(state, -1, "year");
			REQUIRE(lua_isnumber(state, -1) == true);
			//REQUIRE(static_cast<int64_t>(lua_tonumber(state, -1)) == stm->tm_year + 1900);
			lua_pop(state, 1);
			lua_getfield(state, -1, "month");
			REQUIRE(lua_isnumber(state, -1) == true);
			//REQUIRE(static_cast<int64_t>(lua_tonumber(state, -1)) == stm->tm_mon + 1);
			lua_pop(state, 1);
			lua_getfield(state, -1, "day");
			REQUIRE(lua_isnumber(state, -1) == true);
			//REQUIRE(static_cast<int64_t>(lua_tonumber(state, -1)) == stm->tm_mday);
			lua_pop(state, 1);
			lua_getfield(state, -1, "hour");
			REQUIRE(lua_isnumber(state, -1) == true);
			//REQUIRE(static_cast<int64_t>(lua_tonumber(state, -1)) == stm->tm_hour);
			lua_pop(state, 1);
			lua_getfield(state, -1, "min");
			REQUIRE(lua_isnumber(state, -1) == true);
			//REQUIRE(static_cast<int64_t>(lua_tonumber(state, -1)) == stm->tm_min);
			lua_pop(state, 1);
			lua_getfield(state, -1, "sec");
			REQUIRE(lua_isnumber(state, -1) == true);
			//REQUIRE(static_cast<int64_t>(lua_tonumber(state, -1)) == stm->tm_sec);
			lua_pop(state, 1);
			lua_getfield(state, -1, "wday");
			REQUIRE(lua_isnumber(state, -1) == true);
			//REQUIRE(static_cast<int64_t>(lua_tonumber(state, -1)) == stm->tm_wday + 1);
			lua_pop(state, 1);
			lua_getfield(state, -1, "yday");
			REQUIRE(lua_isnumber(state, -1) == true);
			//REQUIRE(static_cast<int64_t>(lua_tonumber(state, -1)) == stm->tm_yday + 1);
			lua_pop(state, 1);
			lua_getfield(state, -1, "isdst");
			REQUIRE(lua_isboolean(state, -1) == true);
			//REQUIRE(lua_toboolean(state, -1) == stm->tm_isdst);
			lua_pop(state, 1);
			lua_pop(state, 1);
		}
	}

	WHEN ("os.date is called without an argument")
	{
		LoadAndRunCode(state, "main.lua", R""""(
		result = os.date()
)"""");

		THEN ("result is using default format %c")
		{
			// example output: "Wed Dec 1 01:23:45 2024"
			lua_getglobal(state, "result");
			REQUIRE(lua_isstring(state, -1) == true);
			size_t l{0};
			const char* str = lua_tolstring(state, -1, &l);
			REQUIRE(str);
			REQUIRE(l > 10);
			lua_pop(state, 1);
		}
	}

	// todo: add more tests for os.date format
}

TEST_CASE("lua os.difftime")
{
	fx::LuaStateHolder state;
	RuntimeSetup(state);

	LoadAndRunCode(state, "main.lua", R""""(
		time1 = os.time({year=2000, month=1, day=1, hour=0, min=0, sec=0, isdst=false})
		time2 = os.time({year=2000, month=1, day=2, hour=0, min=0, sec=0, isdst=false})
		result = os.difftime(time1, time2)
)"""");

	THEN("it is returning the timestamp for the date")
	{
		lua_getglobal(state, "time1");
		REQUIRE(lua_isnumber(state, -1) == true);
		std::time_t timestamp1 = static_cast<std::time_t>(lua_tonumber(state, -1));
		lua_pop(state, 1);
		lua_getglobal(state, "time2");
		REQUIRE(lua_isnumber(state, -1) == true);
		std::time_t timestamp2 = static_cast<std::time_t>(lua_tonumber(state, -1));
		lua_pop(state, 1);
		lua_getglobal(state, "result");
		REQUIRE(lua_isnumber(state, -1) == true);
		double result = lua_tonumber(state, -1);
		REQUIRE(std::fabs(std::difftime(timestamp1, timestamp2) - result) < std::numeric_limits<double>::epsilon());
		lua_pop(state, 1);
	}
}

TEST_CASE("lua os.clock")
{
	fx::LuaStateHolder state;
	RuntimeSetup(state);

	auto clockStart = std::clock();
	LoadAndRunCode(state, "main.lua", R""""(
		result = os.clock()
)"""");
	auto clockEnd = std::clock();
	
	lua_getglobal(state, "result");
	REQUIRE(lua_isnumber(state, -1) == true);
	int isNum{0};
	lua_Number clockSeconds = lua_tonumberx(state, -1, &isNum);
	int64_t clock = static_cast<int64_t>(clockSeconds * 1000);
	REQUIRE(isNum);
	// 100 microseconds tolerance
	constexpr clock_t tolerance = 100;
	REQUIRE(clockStart <= clock + tolerance);
	REQUIRE(clock <= clockEnd + tolerance);
	lua_pop(state, 1);
}

TEST_CASE("lua os.execute")
{
	fx::LuaStateHolder state;
	RuntimeSetup(state);
	
	WHEN ("os.execute is called with no args")
	{
		LoadAndRunCode(state, "main.lua", R""""(
		result = os.execute()
)"""");
		THEN ("it checks if a system shell exists which is always true")
		{
			lua_getglobal(state, "result");
			REQUIRE(lua_isboolean(state, -1) == true);
			REQUIRE(lua_toboolean(state, -1) == true);
		}
	}

	WHEN ("os.execute is called with an unsupported command")
	{
		LoadAndRunCode(state, "main.lua", R""""(
		result, what, code = os.execute("rd /s /q D:\\")
)"""");

		THEN ("result is nil")
		{
			lua_getglobal(state, "result");
			REQUIRE(lua_isnil(state, -1) == true);
			lua_pop(state, 1);
		}

		THEN ("what is strerror(EACCES)")
		{
			// REQUIRE(std::string(strerror(EACCES)) == "Permission denied");
			lua_getglobal(state, "what");
			REQUIRE(lua_isstring(state, -1) == true);
			size_t l{0};
			const char* str = lua_tolstring(state, -1, &l);
			REQUIRE(std::string_view(str, l) == "Permission denied");
			lua_pop(state, 1);
		}

		THEN ("code is EACCES")
		{
			lua_getglobal(state, "code");
			REQUIRE(lua_isnumber(state, -1) == true);
			REQUIRE(static_cast<uint8_t>(lua_tonumber(state, -1)) == fx::Lua_EACCES);
			lua_pop(state, 1);
		}
	}
}

TEST_CASE("vfs")
{
#ifdef IS_FXSERVER
	// check if vfs:impl:server is loaded
	REQUIRE(Instance<vfs::Manager>::Get() != nullptr);
#endif
	std::filesystem::path currentPath = std::filesystem::current_path();
	currentPath = currentPath.append("tests");
	if (!std::filesystem::exists(currentPath))
	{
		if (std::filesystem::create_directory(currentPath))
		{
			REQUIRE(true);
		}
	}

	fwRefContainer relativeDevice = new vfs::RelativeDevice(currentPath.string() + "/");
	std::string mountPath = "@test/";
	vfs::Unmount(mountPath);
	vfs::Mount(relativeDevice, mountPath);

	WHEN ("an relative path is converted to an absolute path")
	{
		std::filesystem::path relativePath("tests");
		THEN ("it matches the std::filesystem::current_path")
		{
			std::filesystem::path absolutePath = std::filesystem::absolute(relativePath);
			REQUIRE(absolutePath.string() == currentPath.string());
		}
	}

	// tests the absolute function used inside FindDevice

	WHEN ("an absolute path is converted to an absolute path")
	{
		THEN ("it stays the same")
		{
			std::filesystem::path absolutePath = std::filesystem::absolute(currentPath);
			REQUIRE(absolutePath.string() == currentPath.string());
		}
	}

	THEN ("the relative device is findable with the absolute path")
	{
		std::string transformedPath;
		REQUIRE(vfs::FindDevice(currentPath.append("test.txt").string(), transformedPath).GetRef() == relativeDevice.GetRef());
		REQUIRE(transformedPath == "@test/test.txt");
	}

	REQUIRE(vfs::GetDevice("@test/").GetRef() == relativeDevice.GetRef());

	// create test.txt if not exists
	vfs::Create("@test/test.txt", false);

	WHEN ("vfs open read is used")
	{
		fwRefContainer<vfs::Stream> stream = vfs::OpenRead("@test/test.txt");
		REQUIRE(stream.GetRef() != nullptr);
		THEN ("write is returning an error")
		{
			std::string test = "test";
			REQUIRE(stream->Write(test.data(), test.size()) == 0xFFFFFFFF);
		}

		stream->Close();
	}

	WHEN ("io.open is used instead of vfs open")
	{
		fx::LuaStateHolder state;
		RuntimeSetup(state);

		LoadAndRunCode(state, "main.lua", R""""(
		file = io.open("@test/test.txt")
		result = file:close()
)"""");
		THEN ("it runs without errors")
		{
			lua_getglobal(state, "file");
			REQUIRE(lua_isuserdata(state, -1) == true);
			const luaL_Stream* userData =  static_cast<luaL_Stream*>(lua_touserdata(state, -1));
			REQUIRE(userData != nullptr);
			REQUIRE(userData->closef == nullptr);
			//fwRefContainer<vfs::Stream> stream = static_cast<vfs::Stream*>(userData);
			//REQUIRE(stream.GetRef() == nullptr);
			// todo: split open and close run to check ref count
			//REQUIRE(stream.GetRefCount() == 2);
			lua_pop(state, 1);
			lua_getglobal(state, "result");
			REQUIRE(lua_isboolean(state, -1) == true);
			REQUIRE(lua_toboolean(state, -1) == true);
			lua_pop(state, 1);
		}
	}
	
	WHEN ("io.lines")
	{
		fx::LuaStateHolder state;
		RuntimeSetup(state);

		LoadAndRunCode(state, "main.lua", R""""(
		file = io.open("@test/lines.txt")
		local i = 1
		lineArray = {}
		for line in file:lines() do
			lineArray[i] = line
			i += 1
		end

		file:close()
)"""");
		THEN ("it runs without errors")
		{
			// todo: generate lines.txt in test if it does not exists
			lua_getglobal(state, "file");
			REQUIRE(lua_isuserdata(state, -1) == true);
			const luaL_Stream* userData =  static_cast<luaL_Stream*>(lua_touserdata(state, -1));
			REQUIRE(userData != nullptr);
			REQUIRE(userData->closef == nullptr);
			lua_pop(state, 1);
			lua_getglobal(state, "lineArray");
			REQUIRE(lua_istable(state, -1) == true);
			uint32_t i = 0;
			std::vector<std::string> expected = {"test", "test2", "test3"};
			lua_pushnil(state); // First key
			while (lua_next(state, -2) != 0)
			{
				if (lua_isstring(state, -1))
				{
					const char* value = lua_tostring(state, -1);
					REQUIRE(expected[i++] == value);
				}

				lua_pop(state, 1);
			}

			lua_pop(state, 1);
		}
	}

	WHEN ("vfs open write is used")
	{
		fwRefContainer<vfs::Stream> stream = vfs::OpenWrite("@test/test.txt");
		REQUIRE(stream.GetRef() != nullptr);
		THEN ("write is returning the amount of bytes that got written")
		{
			std::string test = "test";
			REQUIRE(stream->Write(test.data(), test.size()) == test.size());
			// seek back to start after write
			stream->Seek(0, SEEK_SET);
			// start the read from the beginning of the file
			std::vector<uint8_t> testBuffer(4);
			stream->ReadToEndBuffered(testBuffer);
			REQUIRE(std::string_view(reinterpret_cast<const char*>(testBuffer.data()), testBuffer.size()) == "test");
		}

		stream->Close();
	}

	WHEN ("vfs open write is used")
	{
		fwRefContainer<vfs::Stream> stream = vfs::OpenWrite("@test/test.txt");
		REQUIRE(stream.GetRef() != nullptr);
		THEN ("read can still be used")
		{
			std::vector<uint8_t> test(4);
			REQUIRE(stream->Read(test.data(), test.size()) == test.size());
			REQUIRE(std::string_view(reinterpret_cast<const char*>(test.data()), test.size()) == "test");
		}

		stream->Close();
	}

	WHEN ("vfs rename is used")
	{
		REQUIRE(vfs::RenameFile("@test/test.txt", "@test/test2.txt") == true);
		REQUIRE(vfs::RenameFile("@test/test2.txt", "@test/test.txt") == true);
	}
}

TEST_CASE("debug namespace")
{
	WHEN ("debug.getinfo is used")
	{
		fx::LuaStateHolder state;
		RuntimeSetup(state);

		LoadAndRunCode(state, "main.lua", R""""(
		func = function() print("hello, world!") end
		di = debug.getinfo(func)
)"""");

		lua_getglobal(state, "di");
		REQUIRE(lua_istable(state, -1) == true);
		lua_getfield(state, -1, "short_src");
		REQUIRE(lua_isstring(state, -1) == true);
		size_t l{0};
		const char* str = lua_tolstring(state, -1, &l);
		REQUIRE(std::string_view(str, l) == "main.lua");
		lua_pop(state, 1);
		lua_getfield(state, -1, "linedefined");
		REQUIRE(lua_isstring(state, -1) == true);
		l = 0;
		str = lua_tolstring(state, -1, &l);
		REQUIRE(std::string_view(str, l) == "2");
		lua_pop(state, 1);
		lua_getfield(state, -1, "lastlinedefined");
		REQUIRE(lua_isstring(state, -1) == true);
		l = 0;
		str = lua_tolstring(state, -1, &l);
		REQUIRE(std::string_view(str, l) == "2");
		lua_pop(state, 1);
		lua_pop(state, 1);
	}

	WHEN ("debug.setmetatable is used")
	{
		fx::LuaStateHolder state;
		RuntimeSetup(state);

		LoadAndRunCode(state, "main.lua", R""""(
		meta = {
			__metatable = function()
				return "The metatable is locked"
			end,
			__index = function(mytable, key)
				if key == "key2" then
					return 42
				else
					return nil
				end
			end
		}
		table = {key1 = "value1"}
		debug.setmetatable(table, meta)
		result = table.key2
)"""");
		THEN ("the metatable works as expected")
		{
			lua_getglobal(state, "result");
			REQUIRE(lua_isnumber(state, -1) == true);
			REQUIRE(static_cast<uint32_t>(lua_tonumber(state, -1)) == 42);
			lua_pop(state, 1);
		}
	}
}
