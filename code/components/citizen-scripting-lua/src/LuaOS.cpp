#include "StdInc.h"

#include <filesystem>
#include <lua.hpp>
#include <lua_cmsgpacklib.h>

#include "FilesystemPermissions.h"
#include "LuaFXLib.h"

#include "LuaScriptRuntime.h"
#include "VFSManager.h"

/* maximum size for an individual 'strftime' item */
#define SIZETIMEFMT	250

#if !defined(LUA_STRFTIMEOPTIONS)

/* options for ANSI C 89 (only 1-char options) */
#define L_STRFTIMEC89		"aAbBcdHIjmMpSUwWxXyYZ%"

/* options for ISO C 99 and POSIX */
#define L_STRFTIMEC99 "aAbBcCdDeFgGhHIjmMnprRStTuUVwWxXyYzZ%" \
"||" "EcECExEXEyEY" "OdOeOHOIOmOMOSOuOUOVOwOWOy"  /* two-char options */

/* options for Windows */
#define L_STRFTIMEWIN "aAbBcdHIjmMpSUwWxXyYzZ%" \
"||" "#c#x#d#H#I#j#m#M#S#U#w#W#y#Y"  /* two-char options */

#if defined(LUA_USE_WINDOWS)
#define LUA_STRFTIMEOPTIONS	L_STRFTIMEWIN
#elif defined(LUA_USE_C89)
#define LUA_STRFTIMEOPTIONS	L_STRFTIMEC89
#else  /* C99 specification */
#define LUA_STRFTIMEOPTIONS	L_STRFTIMEC99
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

static int LuaOSDeltaTime(lua_State* L)
{
	const lua_Unsigned end = l_castS2U(luaL_checkinteger(L, 1));
	const lua_Unsigned start = l_castS2U(luaL_checkinteger(L, 2));
	lua_pushinteger(L, static_cast<lua_Integer>(start <= end ? (end - start) : (start - end)));
	return 1;
}

static int LuaOSMicroTime(lua_State* L)
{
	namespace sc = std::chrono;
	auto since_epoch = sc::high_resolution_clock::now().time_since_epoch();
	auto micros = sc::duration_cast<sc::microseconds>(since_epoch);
	l_pushtime(L, micros.count());
	return 1;
}

static int LuaOSNanoTime(lua_State* L)
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
static int LuaOSRdtsc(lua_State* L)
{
	l_pushtime(L, __rdtsc());
	return 1;
}

static int LuaOSRdtscp(lua_State* L)
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

int LuaOSCreateDir(lua_State* L)
{
	const char* directoryPathString = luaL_checkstring(L, 1);
	std::filesystem::path directoryPath = directoryPathString;
	// ensure that the path ends with a path separator
	directoryPath /= "";

	std::string path = directoryPath.generic_string();
	fwRefContainer<vfs::Device> device = vfs::GetDevice(path);
	if (!device.GetRef())
	{
		std::string directoryAbsolutePath = std::filesystem::absolute(std::filesystem::path(directoryPath)).string();
		std::string transformedPath;
		device = vfs::FindDevice(directoryAbsolutePath, transformedPath);
		if (device.GetRef())
		{
			path = transformedPath;
		}
	}

	if (!device.GetRef())
	{
		return luaL_fileresult(L, 0, directoryPathString);
	}

	vfs::FindData fd;
	auto handle = device->FindFirst(path, &fd);
	if (handle != INVALID_DEVICE_HANDLE)
	{
		lua_pushnil(L);
		lua_pushfstring(L, "%s: %s", directoryPathString, "Directory already exists.");
		lua_pushinteger(L, 1);
		return 3;
	}

	if (!device->CreateDirectory(path))
	{
		lua_pushnil(L);
		lua_pushfstring(L, "%s: %s", directoryPathString, "Failed to create directory");
		lua_pushinteger(L, 1);
		device->FindClose(handle);
		return 3;
	}

	lua_pushboolean(L, true);
	device->FindClose(handle);
	return 1;
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

	fwRefContainer<vfs::Device> fromDevice = !fromName.empty() && fromName[0] == '@'
		                                         ? vfs::GetDevice(fromName)
		                                         : nullptr;
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

	if (fx::ScriptingFilesystemAllowWrite(fromPath) && fx::ScriptingFilesystemAllowWrite(toPath) && fromDevice->
		RenameFile(fromPath, toPath))
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
	               [](unsigned char c) { return std::tolower(c); });
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

const char* CheckTimeOption(lua_State* L, const char* conv, ptrdiff_t convlen, char* buff)
{
	const char* option = LUA_STRFTIMEOPTIONS;
	int oplen = 1; /* length of options being checked */
	for (; *option != '\0' && oplen <= convlen; option += oplen)
	{
		/* next block? */
		if (*option == '|')
		{
			oplen++; /* will check options with next length (+1) */
		}
		else if (memcmp(conv, option, oplen) == 0)
		{
			/* match? */
			memcpy(buff, conv, oplen); /* copy valid option to buffer */
			buff[oplen] = '\0';
			return conv + oplen; /* return next item */
		}
	}

	luaL_argerror(L, 1, lua_pushfstring(L, "invalid conversion specifier '%%%s'", conv));
	return conv; /* to avoid warnings */
}

int LuaOSDate(lua_State* L)
{
	size_t slen;
	const char* s = luaL_optlstring(L, 1, "%c", &slen);
	const time_t t = luaL_opt(L, l_checktime, 2, std::time(nullptr));
	std::tm stm;
	std::tm* stm_ptr = nullptr;

	if (*s == '!')
	{
		/* UTC? */
#ifdef WIN32
		if (gmtime_s(&stm, &t) == 0)
		{
			stm_ptr = &stm;
		}
#else
		if (gmtime_r(&t, &stm) != nullptr)
		{
			stm_ptr = &stm;
		}
#endif
		s++; /* skip '!' */
	}
	else
	{
#ifdef WIN32
		if (localtime_s(&stm, &t) == 0)
		{
			stm_ptr = &stm;
		}
#else
		if (localtime_r(&t, &stm) != nullptr)
		{
			stm_ptr = &stm;
		}
#endif
	}

	if (stm_ptr == nullptr)
	{
		/* invalid date? */
		lua_pushnil(L);
		return 1;
	}

	// starts with '*t'?
	if (strcmp(s, "*t") == 0)
	{
		lua_createtable(L, 0, 9); /* 9 = number of fields */
		SetField(L, "sec", stm_ptr->tm_sec);
		SetField(L, "min", stm_ptr->tm_min);
		SetField(L, "hour", stm_ptr->tm_hour);
		SetField(L, "day", stm_ptr->tm_mday);
		SetField(L, "month", stm_ptr->tm_mon + 1);
		SetField(L, "year", stm_ptr->tm_year + 1900);
		SetField(L, "wday", stm_ptr->tm_wday + 1);
		SetField(L, "yday", stm_ptr->tm_yday + 1);
		SetBoolField(L, "isdst", stm_ptr->tm_isdst);
		return 1;
	}

	char cc[4]{}; /* buffer for individual conversion specifiers */
	luaL_Buffer b;
	cc[0] = '%';
	luaL_buffinit(L, &b);
	const char* se = s + slen; /* 's' end */
	while (s < se)
	{
		/* null terminator ends string */
		if (*s == '\0')
		{
			break;
		}

		/* not a conversion specifier? */
		if (*s != '%')
		{
			luaL_addchar(&b, *s++);
		}
		else
		{
			char* buff = luaL_prepbuffsize(&b, SIZETIMEFMT);
			s++; /* skip '%' */
			if (s == se)
			{
				// % last symbol
				break;
			}

			s = CheckTimeOption(L, s, se - s, &cc[1]); /* copy specifier to 'cc' */
			size_t resLen = strftime(buff, SIZETIMEFMT, cc, stm_ptr);
			luaL_addsize(&b, resLen);
		}
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
	/*int op = */
	luaL_checkoption(L, 2, "all", catNames);
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
	{"createdir", LuaOSCreateDir},
	{"remove", LuaOSRemove},
	{"rename", LuaOSRename},
	{"setlocale", LuaOSSetLocale},
	{"time", LuaOSTime},
	{"tmpname", LuaOSTempName},
	{"deltatime", LuaOSDeltaTime},
	{"microtime", LuaOSMicroTime},
	{"nanotime", LuaOSNanoTime},
#if defined(LUA_SYS_RDTSC)
	{"rdtsc", LuaOSRdtsc},
	{"rdtscp", LuaOSRdtscp},
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
