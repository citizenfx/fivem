#include "StdInc.h"
#include "ResourceScripting.h"
#include "scrEngine.h"

static int magicReturnValueInt;
static int magicReturnValueFloat;
static int magicDontReturnBool;
static int magicForceReturnBool;
static int magicReturnResultString;
static int magicReturnResultFloat;

static DWORD exceptionAddress;

static int ExceptionStuff(LPEXCEPTION_POINTERS info)
{
	exceptionAddress = (DWORD)info->ExceptionRecord->ExceptionAddress;
	return EXCEPTION_EXECUTE_HANDLER;
}

// TODO: add pointer safety checks!
LUA_FUNCTION(CallNative)
{
	int numArgs = lua_gettop(L);
	NativeContext ctx;

	int numReturnValues = 0;
	uint32_t retvals[8];
	int rettypes[8];

	bool returnValueDontConvert = false;
	bool returnBoolAnyway = false;
	bool returnStringResult = false;
	bool returnFloatResult = false;

	uint32_t hash = luaL_checkinteger(L, 1);

	for (int i = 2; i <= numArgs; i++)
	{
		int type = lua_type(L, i);

		if (type == LUA_TNIL)
		{
			ctx.Push(0);
		}
		else if (type == LUA_TNUMBER)
		{
			double number = luaL_checknumber(L, i);

			if (number == (uint32_t)number)
			{
				ctx.Push((uint32_t)number);
			}
			else
			{
				if (number < 0 && number == (int)number)
				{
					ctx.Push((int)number);
				}
				else
				{
					ctx.Push((float)number);
				}
			}
		}
		else if (type == LUA_TBOOLEAN)
		{
			ctx.Push(lua_toboolean(L, i));
		}
		else if (type == LUA_TLIGHTUSERDATA)
		{
			void* userData = lua_touserdata(L, i);

			if (userData == &magicReturnValueInt || userData == &magicReturnValueFloat)
			{
				if (numReturnValues >= _countof(retvals))
				{
					lua_pushstring(L, "too many arguments");
					lua_error(L);

					return 0;
				}

				if (userData == &magicReturnValueFloat)
				{
					rettypes[numReturnValues] = 0; // magic
				}
				else
				{
					rettypes[numReturnValues] = 1;
				}

				ctx.Push(&retvals[numReturnValues]);
				numReturnValues++;
			}
			else if (userData == &magicForceReturnBool)
			{
				returnBoolAnyway = true;	
			}
			else if (userData == &magicDontReturnBool)
			{
				returnValueDontConvert = true;
			}
			else if (userData == &magicReturnResultString)
			{
				returnStringResult = true;
			}
			else if (userData == &magicReturnResultFloat)
			{
				returnFloatResult = true;
			}
			else
			{
				ctx.Push(userData);
			}
		}
		else if (type == LUA_TTABLE)
		{
			lua_pushstring(L, "__data");
			lua_rawget(L, i);

			if (lua_type(L, -1) == LUA_TNUMBER)
			{
				ctx.Push(lua_tointeger(L, -1));
				lua_pop(L, 1);
			}
			else
			{
				lua_pushstring(L, "Invalid Lua type in __data");
				lua_error(L);
			}
		}
		else if (type == LUA_TSTRING)
		{
			ctx.Push(luaL_checkstring(L, i));
		}
		else if (type == 10)
		{
			// cdata, apparently
			ctx.Push(lua_topointer(L, i));
		}
		else
		{
			lua_pushstring(L, va("Invalid Lua type: %i", type));
			lua_error(L);
		}
	}
	

	auto fn = rage::scrEngine::GetNativeHandler(hash);

	if (fn != 0)
	{
		exceptionAddress = 0;

		__try
		{
			fn(&ctx);
		}
		__except(ExceptionStuff(GetExceptionInformation()))
		{
			
		}

		if (exceptionAddress)
		{
			lua_pushstring(L, va("error executing hash %08x - exception address %08x", hash, exceptionAddress));
			lua_error(L);
		}
	}
	else
	{
		lua_pushstring(L, va("unknown hash %08x!", hash));
		lua_error(L);
	}

	int realNumReturnValues = numReturnValues;

	if (numReturnValues == 0 || returnBoolAnyway)
	{
		if (returnStringResult)
		{
			lua_pushstring(L, ctx.GetResult<const char*>());
		}
		else if (returnFloatResult)
		{
			lua_pushnumber(L, ctx.GetResult<float>());
		}
		else if (returnValueDontConvert || ctx.GetResult<uint32_t>() != 0)
		{
			lua_pushnumber(L, ctx.GetResult<uint32_t>());
		}
		else
		{
			lua_pushboolean(L, false);
		}

		numReturnValues++;
	}

	for (int i = 0; i < realNumReturnValues; i++)
	{
		if (rettypes[i] == 0)
		{
			lua_pushnumber(L, *(float*)&retvals[i]);
		}
		else if (rettypes[i] == 2)
		{
			if (retvals[i] == 0)
			{
				lua_pushboolean(L, false);
			}
			else
			{
				lua_pushnumber(L, retvals[i]);
			}
		}
		else
		{
			lua_pushnumber(L, retvals[i]);
		}
	}

	return numReturnValues;
}

LUA_FUNCTION(ptr)
{
	static int ptrs[32];
	static int ptrI;

	ptrI = (ptrI++) % 32;

	ptrs[ptrI] = lua_tointeger(L, 1);

	lua_pushlightuserdata(L, &ptrs[ptrI]);

	return 1;
}

LUA_FUNCTION(rf)
{
	lua_pushlightuserdata(L, &magicReturnValueFloat);

	return 1;
}

LUA_FUNCTION(ri)
{
	lua_pushlightuserdata(L, &magicReturnValueInt);

	return 1;
}

LUA_FUNCTION(rnb)
{
	lua_pushlightuserdata(L, &magicDontReturnBool);

	return 1;
}

LUA_FUNCTION(rfb)
{
	lua_pushlightuserdata(L, &magicForceReturnBool);

	return 1;
}

LUA_FUNCTION(rrs)
{
	lua_pushlightuserdata(L, &magicReturnResultString);

	return 1;
}

LUA_FUNCTION(rrf)
{
	lua_pushlightuserdata(L, &magicReturnResultFloat);

	return 1;
}