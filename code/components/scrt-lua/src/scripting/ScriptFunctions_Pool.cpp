/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include "ResourceScripting.h"

#if 0
static int ReturnPoolHandles(lua_State* L, CPool* pool)
{
	std::vector<int> handles;

	for (int i = 0; i < pool->GetCount(); i++)
	{
		if (pool->m_pFlags[i] >= 0)
		{
			handles.push_back((i << 8) | pool->m_pFlags[i]);
		}
	}

	// push them to the lua stack
	lua_createtable(L, handles.size(), 0);

	int table = lua_gettop(L);

	// loop through the handle list
	for (int i = 0; i < handles.size(); i++)
	{
		lua_pushinteger(L, handles[i]);
		lua_rawseti(L, table, i);
	}

	// return the table
	return 1;
}

LUA_FUNCTION(GetAllPeds)
{
	CPool* pedPool = *(CPool**)0x18A82AC;

	return ReturnPoolHandles(L, pedPool);
}
#endif