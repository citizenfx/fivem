#include <string.h>
#include <lua.h>
#include <lauxlib.h>
#include <stdint.h>

uint32_t jenkins_one_at_a_time_hash(const char* key, size_t length) {
  size_t i = 0;
  uint32_t hash = 0;
  while (i != length) {
    hash += key[i++];
    hash += hash << 10;
    hash ^= hash >> 6;
  }
  hash += hash << 3;
  hash ^= hash >> 11;
  hash += hash << 15;
  return hash;
}

static int hash(lua_State* l)
{
	size_t len;
	const char* arg = luaL_checklstring(l, 1, &len);

	lua_pushinteger(l, jenkins_one_at_a_time_hash(arg, len));

	return 1;
}

static const struct luaL_Reg cfx[] = {
    {"hash", hash},
    {NULL, NULL}
};

int luaopen_cfx(lua_State* l)
{

    luaL_newlibtable(l, cfx);
    luaL_setfuncs(l, cfx, 0);
    return 1;
}