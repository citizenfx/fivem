#include "StdInc.h"

#include <lua.hpp>
#include <lua_cmsgpacklib.h>

#include "FilesystemPermissions.h"
#include "LuaFXLib.h"

#include "LuaScriptRuntime.h"
#include "VFSManager.h"

#if !defined(LUA_STRFTIMEOPTIONS)

#if defined(LUA_USE_C89)
#define LUA_STRFTIMEOPTIONS	{ "aAbBcdHIjmMpSUwWxXyYz%", "" }
#else  /* C99 specification */
#define LUA_STRFTIMEOPTIONS \
	{ "aAbBcCdDeFgGhHIjmMnprRStTuUVwWxXyYzZ%", "", \
	  "E", "cCxXyY",  \
	  "O", "deHImMSuUVwWy" }
#endif

#endif

#if !defined(l_time_t)
/*
** type to represent time_t in Lua
*/
#define l_timet			lua_Integer
#define l_pushtime(L,t)		lua_pushinteger(L,(lua_Integer)(t))
#define l_checktime(L,a)	((time_t)luaL_checkinteger(L,a))

#endif



/*
** {==================================================================
** High-Resolution Time Stamps
** ===================================================================
*/

#define LUA_SYS_RDTSC /* os_rdtsc & os_rdtscp enabled */

#include <chrono>

#if defined(_MSC_VER) && defined(_M_X64)
  #include <intrin.h>
#elif defined(__GNUC__) && defined(__has_include) && __has_include(<x86intrin.h>)
  #include <x86intrin.h>
#else
  #undef LUA_SYS_RDTSC
#endif

#include "llimits.h"

static int LuaOSDeltaTime(lua_State *L)
{
  	const lua_Unsigned end = l_castS2U(luaL_checkinteger(L, 1));
  	const lua_Unsigned start = l_castS2U(luaL_checkinteger(L, 2));
  	lua_pushinteger(L, static_cast<lua_Integer>(start <= end ? (end - start) : (start - end)));
  	return 1;
}

static int LuaOSMicroTime(lua_State *L)
{
  	namespace sc = std::chrono;
  	auto since_epoch = sc::high_resolution_clock::now().time_since_epoch();
  	auto micros = sc::duration_cast<sc::microseconds>(since_epoch);
  	l_pushtime(L, micros.count());
  	return 1;
}

static int LuaOSNanoTime(lua_State *L)
{
  	namespace sc = std::chrono;
  	auto since_epoch = sc::high_resolution_clock::now().time_since_epoch();
  	auto nanos = sc::duration_cast<sc::nanoseconds>(since_epoch);
  	l_pushtime(L, nanos.count());
  	return 1;
}

/*
** Sample the rdtsc (Read Time-Stamp Counter) instruction. Returning the process
** time stamp: cycle references since last reset.
*/
#if defined(LUA_SYS_RDTSC)
static int LuaOSRdtsc(lua_State *L)
{
  	l_pushtime(L, __rdtsc());
  	return 1;
}

static int LuaOSRdtscp(lua_State *L)
{
  	unsigned int aux;
  	l_pushtime(L, __rdtscp(&aux));
  	return 1;
}
#endif

/* }================================================================== */



namespace
{
int LuaOSExecute(lua_State* L)
{
	const char* cmd = luaL_optstring(L, 1, NULL);
	if (cmd == nullptr)
	{
		// in original behavior giving a null command checks if a system shell exists
		// this sets this to always true
		lua_pushboolean(L, true);
		return 1;
	}

	// for now set every os.execute call to return EACCES
	int stat = -1;
	const char* what = "exit";
	if (stat == -1)
	{
		int en = fx::Lua_EACCES;
		lua_pushnil(L);
		// strerror(EACCES)) == "Permission denied"
		lua_pushstring(L, "Permission denied");
		lua_pushinteger(L, en);
		return 3;
	}

	if (*what == 'e' && stat == 0) /* successful termination? */
	{
		lua_pushboolean(L, true);
	}
	else
	{
		lua_pushnil(L);
	}

	lua_pushstring(L, what);
	lua_pushinteger(L, stat);
	return 3; /* return true/nil,what,code */
}

int LuaOSRemove(lua_State* L)
{
	const char* filenameString = luaL_checkstring(L, 1);
	std::string filename = filenameString;
	static auto returnErr = [](lua_State* L, const char* filename)
	{
		int en = fx::Lua_EACCES;
		lua_pushnil(L);
		if (filename)
		{
			lua_pushfstring(L, "%s: %s", filename, /*strerror(en)*/ "Permission denied");
		}
		else
		{
			lua_pushstring(L, /*strerror(en)*/ "Permission denied");
		}

		lua_pushinteger(L, en);
		return 3;
	};

	fwRefContainer<vfs::Device> device = !filename.empty() && filename[0] == '@' ? vfs::GetDevice(filename) : nullptr;
	std::string path = filename;
	if (!device.GetRef())
	{
		device = vfs::FindDevice(filename, path);
		if (!device.GetRef())
		{
			return returnErr(L, filenameString);
		}
	}

	if (fx::ScriptingFilesystemAllowWrite(path) && device->RemoveFile(path))
	{
		// success
		lua_pushboolean(L, true);
		return 1;
	}
	
	return returnErr(L, filenameString);
}

int LuaOSRename(lua_State* L)
{
	static auto returnErr = [](lua_State* L, const char* fromName, const char* toName)
	{
		int en = fx::Lua_EACCES;
		lua_pushnil(L);
		if (fromName && toName)
		{
			lua_pushfstring(L, "%s -> %s: %s", fromName, toName, /*strerror(en)*/ "Permission denied");
		}
		else
		{
			lua_pushstring(L, /*strerror(en)*/ "Permission denied");
		}

		lua_pushinteger(L, en);
		return 3;
	};

	// always return EACCES error when a file is attempted to be renamed
	const char* fromNameString = luaL_checkstring(L, 1);
	const char* toNameString = luaL_checkstring(L, 2);
	std::string fromName = fromNameString;
	std::string toName = toNameString;

	fwRefContainer<vfs::Device> fromDevice = !fromName.empty() && fromName[0] == '@' ? vfs::GetDevice(fromName) : nullptr;
	std::string fromPath = fromName;
	if (!fromDevice.GetRef())
	{
		fromDevice = vfs::FindDevice(fromName, fromPath);
		if (!fromDevice.GetRef())
		{
			return returnErr(L, fromNameString, toNameString);
		}
	}

	fwRefContainer<vfs::Device> toDevice = !toName.empty() && toName[0] == '@' ? vfs::GetDevice(toName) : nullptr;
	std::string toPath = toName;
	if (!toDevice.GetRef())
	{
		toDevice = vfs::FindDevice(toName, toPath);
		if (!fromDevice.GetRef())
		{
			return returnErr(L, fromNameString, toNameString);
		}
	}

	if (fx::ScriptingFilesystemAllowWrite(fromPath) && fx::ScriptingFilesystemAllowWrite(toPath) && fromDevice->RenameFile(fromPath, toPath))
	{
		// success
		lua_pushboolean(L, true);
		return 1;
	}

	return returnErr(L, fromNameString, toNameString);
}

int LuaOSTempName(lua_State* L)
{
	// lua_tmpnam is not security significant, but an increment is more secure
	// because it is not giving away available names inside the tmp folder
	static uint64_t increment{0};
	char buff[260];
	std::string tmpString = "tmp_" + std::to_string(++increment);
	if (tmpString.size() > 260 - 1)
	{
		tmpString.resize(260 - 1);
	}
	memcpy(buff, tmpString.c_str(), tmpString.size() + 1);
	lua_pushstring(L, buff);
	return 1;
}

int LuaOSGetEnv(lua_State* L)
{
	std::string envKey = luaL_checkstring(L, 1);
	std::transform(envKey.begin(), envKey.end(), envKey.begin(),
		[](unsigned char c){ return std::tolower(c); });
	if (envKey == "os")
	{
#ifdef WIN32
		lua_pushstring(L, "Windows");
#else
		lua_pushstring(L, "Linux");
#endif
	}
	else
	{
		lua_pushnil(L);
	}
	return 1;
}

int LuaOSClock(lua_State* L)
{
	lua_pushnumber(L, static_cast<lua_Number>(clock()) / static_cast<lua_Number>(CLOCKS_PER_SEC));
	return 1;
}

void SetField(lua_State* L, const char* key, int value)
{
	lua_pushinteger(L, value);
	lua_setfield(L, -2, key);
}

void SetBoolField(lua_State* L, const char* key, int value)
{
	if (value < 0)
	{
		/* undefined? */
		return; /* does not set field */
	}

	lua_pushboolean(L, value);
	lua_setfield(L, -2, key);
}

int GetBoolField(lua_State* L, const char* key)
{
	int res;
	res = (lua_getfield(L, -1, key) == LUA_TNIL) ? -1 : lua_toboolean(L, -1);
	lua_pop(L, 1);
	return res;
}

int GetField(lua_State* L, const char* key, int d)
{
	int res, isnum;
	lua_getfield(L, -1, key);
	res = (int)lua_tointegerx(L, -1, &isnum);
	if (!isnum)
	{
		if (d < 0)
			return luaL_error(L, "field '%s' missing in date table", key);
		res = d;
	}
	lua_pop(L, 1);
	return res;
}

const char* CheckTimeOption(lua_State* L, const char* conv, char* buff)
{
	static constexpr char* const options[] = LUA_STRFTIMEOPTIONS;
	for (unsigned int i = 0; i < std::size(options); i += 2)
	{
		if (*conv != '\0' && std::strchr(options[i], *conv) != nullptr)
		{
			buff[1] = *conv;
			if (*options[i + 1] == '\0')
			{
				/* one-char conversion specifier? */
				buff[2] = '\0'; /* end buffer */
				return conv + 1;
			}

			if (*(conv + 1) != '\0' && std::strchr(options[i + 1], *(conv + 1)) != nullptr)
			{
				buff[2] = *(conv + 1); /* valid two-char conversion specifier */
				buff[3] = '\0'; /* end buffer */
				return conv + 2;
			}
		}
	}

	luaL_argerror(L, 1, lua_pushfstring(L, "invalid conversion specifier '%%%s'", conv));
	return conv; /* to avoid warnings */
}

int LuaOSDate(lua_State* L)
{
	const char* s = luaL_optstring(L, 1, "%c");
	const time_t t = luaL_opt(L, l_checktime, 2, std::time(nullptr));
	std::tm* stm;
	if (*s == '!')
	{
		/* UTC? */
		stm = std::gmtime(&t);
		s++; /* skip '!' */
	}
	else
	{
		stm = std::localtime(&t);
	}

	if (stm == nullptr)
	{
		/* invalid date? */
		lua_pushnil(L);
		return 1;
	}

	// starts with '*t'?
	if (strcmp(s, "*t") == 0)
	{
		lua_createtable(L, 0, 9); /* 9 = number of fields */
		SetField(L, "sec", stm->tm_sec);
		SetField(L, "min", stm->tm_min);
		SetField(L, "hour", stm->tm_hour);
		SetField(L, "day", stm->tm_mday);
		SetField(L, "month", stm->tm_mon + 1);
		SetField(L, "year", stm->tm_year + 1900);
		SetField(L, "wday", stm->tm_wday + 1);
		SetField(L, "yday", stm->tm_yday + 1);
		SetBoolField(L, "isdst", stm->tm_isdst);
		return 1;
	}

	char cc[4];
	luaL_Buffer b;
	cc[0] = '%';
	luaL_buffinit(L, &b);
	while (*s)
	{
		if (*s != '%')
		{
			/* no conversion specifier? */
			luaL_addchar(&b, *s++);
			continue;
		}

		char buff[200]; /* should be big enough for any conversion result */
		s = CheckTimeOption(L, s + 1, cc);
		size_t resLen = strftime(buff, sizeof(buff), cc, stm);
		luaL_addlstring(&b, buff, resLen);
	}

	luaL_pushresult(&b);

	return 1;
}

int LuaOSTime(lua_State* L)
{
	time_t t;
	/* called without args? */
	if (lua_isnoneornil(L, 1))
	{
		/* get current time */
		t = std::time(nullptr);
	}
	else
	{
		std::tm ts;
		luaL_checktype(L, 1, LUA_TTABLE);
		lua_settop(L, 1); /* make sure table is at the top */
		ts.tm_sec = GetField(L, "sec", 0);
		ts.tm_min = GetField(L, "min", 0);
		ts.tm_hour = GetField(L, "hour", 12);
		ts.tm_mday = GetField(L, "day", -1);
		ts.tm_mon = GetField(L, "month", -1) - 1;
		ts.tm_year = GetField(L, "year", -1) - 1900;
		ts.tm_wday = 0;
		ts.tm_yday = 0;
		ts.tm_isdst = GetBoolField(L, "isdst");
		t = mktime(&ts);
	}

	if (t != (time_t)(l_timet)t)
	{
		luaL_error(L, "time result cannot be represented in this Lua installation");
	}
	else if (t == static_cast<time_t>(-1))
	{
		lua_pushnil(L);
	}
	else
	{
		l_pushtime(L, t);
	}

	return 1;
}

int LuaOSDiffTime(lua_State* L)
{
	time_t t1 = l_checktime(L, 1);
	time_t t2 = l_checktime(L, 2);
	lua_pushnumber(L, std::difftime(t1, t2));
	return 1;
}

int LuaOSSetLocale(lua_State* L)
{
	// does no longer set the locale, but returns the set locale to keep behavior
	/*static const int cat[] = {
		LC_ALL, LC_COLLATE, LC_CTYPE, LC_MONETARY,
		LC_NUMERIC, LC_TIME
	};*/
	static const char* const catNames[] = {
		"all", "collate", "ctype", "monetary",
		"numeric", "time", nullptr
	};
	const char* l = luaL_optstring(L, 1, NULL);
	/*int op = */luaL_checkoption(L, 2, "all", catNames);
	lua_pushstring(L, l);
	return 1;
}

// only none mapped function is os.exit
const luaL_Reg systemLibs[] = {
	{"clock", LuaOSClock},
	{"date", LuaOSDate},
	{"difftime", LuaOSDiffTime},
	{"execute", LuaOSExecute},
	{"getenv", LuaOSGetEnv},
	{"remove", LuaOSRemove},
	{"rename", LuaOSRename},
	{"setlocale", LuaOSSetLocale},
	{"time", LuaOSTime},
	{"tmpname", LuaOSTempName},
	{ "deltatime", LuaOSDeltaTime },
	{ "microtime", LuaOSMicroTime },
	{ "nanotime", LuaOSNanoTime },
#if defined(LUA_SYS_RDTSC)
	{ "rdtsc", LuaOSRdtsc },
	{ "rdtscp", LuaOSRdtscp },
#endif
	{nullptr, nullptr}
};
}

namespace fx
{
int lua_fx_openos(lua_State* L)
{
	luaL_newlib(L, systemLibs);
	return 1;
}
}
