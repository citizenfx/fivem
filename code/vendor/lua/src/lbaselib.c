/*
** $Id: lbaselib.c,v 1.310 2015/03/28 19:14:47 roberto Exp $
** Basic library
** See Copyright Notice in lua.h
*/

#define lbaselib_c
#define LUA_LIB

#include "lprefix.h"


#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lua.h"

#include "lauxlib.h"
#include "lualib.h"


static int luaB_print (lua_State *L) {
  int n = lua_gettop(L);  /* number of arguments */
  int i;
  lua_getglobal(L, "tostring");
  for (i=1; i<=n; i++) {
    const char *s;
    size_t l;
    lua_pushvalue(L, -1);  /* function to be called */
    lua_pushvalue(L, i);   /* value to print */
    lua_call(L, 1, 1);
    s = lua_tolstring(L, -1, &l);  /* get result */
    if (s == NULL)
      return luaL_error(L, "'tostring' must return a string to 'print'");
    if (i>1) lua_writestring("\t", 1);
    lua_writestring(s, l);
    lua_pop(L, 1);  /* pop result */
  }
  lua_writeline();
  return 0;
}


#define SPACECHARS	" \f\n\r\t\v"

static const char *b_str2int (const char *s, int base, lua_Integer *pn) {
  lua_Unsigned n = 0;
  int neg = 0;
  s += strspn(s, SPACECHARS);  /* skip initial spaces */
  if (*s == '-') { s++; neg = 1; }  /* handle signal */
  else if (*s == '+') s++;
  if (!isalnum((unsigned char)*s))  /* no digit? */
    return NULL;
  do {
    int digit = (isdigit((unsigned char)*s)) ? *s - '0'
                   : (toupper((unsigned char)*s) - 'A') + 10;
    if (digit >= base) return NULL;  /* invalid numeral */
    n = n * base + digit;
    s++;
  } while (isalnum((unsigned char)*s));
  s += strspn(s, SPACECHARS);  /* skip trailing spaces */
  *pn = (lua_Integer)((neg) ? (0u - n) : n);
  return s;
}


static int luaB_tonumber (lua_State *L) {
  if (lua_isnoneornil(L, 2)) {  /* standard conversion? */
    luaL_checkany(L, 1);
    if (lua_type(L, 1) == LUA_TNUMBER) {  /* already a number? */
      lua_settop(L, 1);  /* yes; return it */
      return 1;
    }
    else {
      size_t l;
      const char *s = lua_tolstring(L, 1, &l);
      if (s != NULL && lua_stringtonumber(L, s) == l + 1)
        return 1;  /* successful conversion to number */
      /* else not a number */
    }
  }
  else {
    size_t l;
    const char *s;
    lua_Integer n = 0;  /* to avoid warnings */
    lua_Integer base = luaL_checkinteger(L, 2);
    luaL_checktype(L, 1, LUA_TSTRING);  /* before 'luaL_checklstring'! */
    s = luaL_checklstring(L, 1, &l);
    luaL_argcheck(L, 2 <= base && base <= 36, 2, "base out of range");
    if (b_str2int(s, (int)base, &n) == s + l) {
      lua_pushinteger(L, n);
      return 1;
    }  /* else not a number */
  }  /* else not a number */
  lua_pushnil(L);  /* not a number */
  return 1;
}


static int luaB_error (lua_State *L) {
  int level = (int)luaL_optinteger(L, 2, 1);
  lua_settop(L, 1);
  if (lua_isstring(L, 1) && level > 0) {  /* add extra information? */
    luaL_where(L, level);
    lua_pushvalue(L, 1);
    lua_concat(L, 2);
  }
  return lua_error(L);
}


static int luaB_getmetatable (lua_State *L) {
  luaL_checkany(L, 1);
  if (!lua_getmetatable(L, 1)) {
    lua_pushnil(L);
    return 1;  /* no metatable */
  }
  luaL_getmetafield(L, 1, "__metatable");
  return 1;  /* returns either __metatable field (if present) or metatable */
}


static int luaB_setmetatable (lua_State *L) {
  int t = lua_type(L, 2);
  luaL_checktype(L, 1, LUA_TTABLE);
  luaL_argcheck(L, t == LUA_TNIL || t == LUA_TTABLE, 2,
                    "nil or table expected");
  if (luaL_getmetafield(L, 1, "__metatable") != LUA_TNIL)
    return luaL_error(L, "cannot change a protected metatable");
  lua_settop(L, 2);
  lua_setmetatable(L, 1);
  return 1;
}


static int luaB_rawequal (lua_State *L) {
  luaL_checkany(L, 1);
  luaL_checkany(L, 2);
  lua_pushboolean(L, lua_rawequal(L, 1, 2));
  return 1;
}


static int luaB_rawlen (lua_State *L) {
  int t = lua_type(L, 1);
  luaL_argcheck(L, t == LUA_TTABLE || t == LUA_TSTRING, 1,
                   "table or string expected");
  lua_pushinteger(L, lua_rawlen(L, 1));
  return 1;
}


static int luaB_rawget (lua_State *L) {
  luaL_checktype(L, 1, LUA_TTABLE);
  luaL_checkany(L, 2);
  lua_settop(L, 2);
  lua_rawget(L, 1);
  return 1;
}

static int luaB_rawset (lua_State *L) {
  luaL_checktype(L, 1, LUA_TTABLE);
  luaL_checkany(L, 2);
  luaL_checkany(L, 3);
  lua_settop(L, 3);
  lua_rawset(L, 1);
  return 1;
}


static int luaB_collectgarbage (lua_State *L) {
  static const char *const opts[] = {"stop", "restart", "collect",
    "count", "step", "setpause", "setstepmul",
    "isrunning", NULL};
  static const int optsnum[] = {LUA_GCSTOP, LUA_GCRESTART, LUA_GCCOLLECT,
    LUA_GCCOUNT, LUA_GCSTEP, LUA_GCSETPAUSE, LUA_GCSETSTEPMUL,
    LUA_GCISRUNNING};
  int o = optsnum[luaL_checkoption(L, 1, "collect", opts)];
  int ex = (int)luaL_optinteger(L, 2, 0);
  int res = lua_gc(L, o, ex);
  switch (o) {
    case LUA_GCCOUNT: {
      int b = lua_gc(L, LUA_GCCOUNTB, 0);
      lua_pushnumber(L, (lua_Number)res + ((lua_Number)b/1024));
      return 1;
    }
    case LUA_GCSTEP: case LUA_GCISRUNNING: {
      lua_pushboolean(L, res);
      return 1;
    }
    default: {
      lua_pushinteger(L, res);
      return 1;
    }
  }
}


/*
** This function has all type names as upvalues, to maximize performance.
*/
static int luaB_type (lua_State *L) {
  luaL_checkany(L, 1);
  lua_pushvalue(L, lua_upvalueindex(lua_type(L, 1) + 1));
  return 1;
}


static int pairsmeta (lua_State *L, const char *method, int iszero,
                      lua_CFunction iter) {
  if (luaL_getmetafield(L, 1, method) == LUA_TNIL) {  /* no metamethod? */
    luaL_checktype(L, 1, LUA_TTABLE);  /* argument must be a table */
    lua_pushcfunction(L, iter);  /* will return generator, */
    lua_pushvalue(L, 1);  /* state, */
    if (iszero) lua_pushinteger(L, 0);  /* and initial value */
    else lua_pushnil(L);
  }
  else {
    lua_pushvalue(L, 1);  /* argument 'self' to metamethod */
    lua_call(L, 1, 3);  /* get 3 values from metamethod */
  }
  return 3;
}


static int luaB_next (lua_State *L) {
  luaL_checktype(L, 1, LUA_TTABLE);
  lua_settop(L, 2);  /* create a 2nd argument if there isn't one */
  if (lua_next(L, 1))
    return 2;
  else {
    lua_pushnil(L);
    return 1;
  }
}


static int luaB_pairs (lua_State *L) {
  return pairsmeta(L, "__pairs", 0, luaB_next);
}


/*
** Traversal function for 'ipairs' for raw tables
*/
static int ipairsaux_raw (lua_State *L) {
  lua_Integer i = luaL_checkinteger(L, 2) + 1;
  luaL_checktype(L, 1, LUA_TTABLE);
  lua_pushinteger(L, i);
  return (lua_rawgeti(L, 1, i) == LUA_TNIL) ? 1 : 2;
}


/*
** Traversal function for 'ipairs' for tables with metamethods
*/
static int ipairsaux (lua_State *L) {
  lua_Integer i = luaL_checkinteger(L, 2) + 1;
  lua_pushinteger(L, i);
  return (lua_geti(L, 1, i) == LUA_TNIL) ? 1 : 2;
}


/*
** This function will use either 'ipairsaux' or 'ipairsaux_raw' to
** traverse a table, depending on whether the table has metamethods
** that can affect the traversal.
*/
static int luaB_ipairs (lua_State *L) {
  lua_CFunction iter = (luaL_getmetafield(L, 1, "__index") != LUA_TNIL)
                       ? ipairsaux : ipairsaux_raw;
#if defined(LUA_COMPAT_IPAIRS)
  return pairsmeta(L, "__ipairs", 1, iter);
#else
  luaL_checkany(L, 1);
  lua_pushcfunction(L, iter);  /* iteration function */
  lua_pushvalue(L, 1);  /* state */
  lua_pushinteger(L, 0);  /* initial value */
  return 3;
#endif
}


static int load_aux (lua_State *L, int status, int envidx) {
  if (status == LUA_OK) {
    if (envidx != 0) {  /* 'env' parameter? */
      lua_pushvalue(L, envidx);  /* environment for loaded function */
      if (!lua_setupvalue(L, -2, 1))  /* set it as 1st upvalue */
        lua_pop(L, 1);  /* remove 'env' if not used by previous call */
    }
    return 1;
  }
  else {  /* error (message is on top of the stack) */
    lua_pushnil(L);
    lua_insert(L, -2);  /* put before error message */
    return 2;  /* return nil plus error message */
  }
}


static int luaB_loadfile (lua_State *L) {
  const char *fname = luaL_optstring(L, 1, NULL);
  const char *mode = luaL_optstring(L, 2, NULL);
  int env = (!lua_isnone(L, 3) ? 3 : 0);  /* 'env' index or 0 if no 'env' */
  int status = luaL_loadfilex(L, fname, mode);
  return load_aux(L, status, env);
}


/*
** {======================================================
** Generic Read function
** =======================================================
*/


/*
** reserved slot, above all arguments, to hold a copy of the returned
** string to avoid it being collected while parsed. 'load' has four
** optional arguments (chunk, source name, mode, and environment).
*/
#define RESERVEDSLOT	5


/*
** Reader for generic 'load' function: 'lua_load' uses the
** stack for internal stuff, so the reader cannot change the
** stack top. Instead, it keeps its resulting string in a
** reserved slot inside the stack.
*/
static const char *generic_reader (lua_State *L, void *ud, size_t *size) {
  (void)(ud);  /* not used */
  luaL_checkstack(L, 2, "too many nested functions");
  lua_pushvalue(L, 1);  /* get function */
  lua_call(L, 0, 1);  /* call it */
  if (lua_isnil(L, -1)) {
    lua_pop(L, 1);  /* pop result */
    *size = 0;
    return NULL;
  }
  else if (!lua_isstring(L, -1))
    luaL_error(L, "reader function must return a string");
  lua_replace(L, RESERVEDSLOT);  /* save string in reserved slot */
  return lua_tolstring(L, RESERVEDSLOT, size);
}


static int luaB_load (lua_State *L) {
  int status;
  size_t l;
  const char *s = lua_tolstring(L, 1, &l);
  const char *mode = luaL_optstring(L, 3, "bt");
  int env = (!lua_isnone(L, 4) ? 4 : 0);  /* 'env' index or 0 if no 'env' */
  if (s != NULL) {  /* loading a string? */
    const char *chunkname = luaL_optstring(L, 2, s);
    status = luaL_loadbufferx(L, s, l, chunkname, mode);
  }
  else {  /* loading from a reader function */
    const char *chunkname = luaL_optstring(L, 2, "=(load)");
    luaL_checktype(L, 1, LUA_TFUNCTION);
    lua_settop(L, RESERVEDSLOT);  /* create reserved slot */
    status = lua_load(L, generic_reader, NULL, chunkname, mode);
  }
  return load_aux(L, status, env);
}

/* }====================================================== */


static int dofilecont (lua_State *L, int d1, lua_KContext d2) {
  (void)d1;  (void)d2;  /* only to match 'lua_Kfunction' prototype */
  return lua_gettop(L) - 1;
}


static int luaB_dofile (lua_State *L) {
  const char *fname = luaL_optstring(L, 1, NULL);
  lua_settop(L, 1);
  if (luaL_loadfile(L, fname) != LUA_OK)
    return lua_error(L);
  lua_callk(L, 0, LUA_MULTRET, 0, dofilecont);
  return dofilecont(L, 0, 0);
}


static int luaB_assert (lua_State *L) {
  if (lua_toboolean(L, 1))  /* condition is true? */
    return lua_gettop(L);  /* return all arguments */
  else {  /* error */
    luaL_checkany(L, 1);  /* there must be a condition */
    lua_remove(L, 1);  /* remove it */
    lua_pushliteral(L, "assertion failed!");  /* default message */
    lua_settop(L, 1);  /* leave only message (default if no other one) */
    return luaB_error(L);  /* call 'error' */
  }
}


static int luaB_select (lua_State *L) {
  int n = lua_gettop(L);
  if (lua_type(L, 1) == LUA_TSTRING && *lua_tostring(L, 1) == '#') {
    lua_pushinteger(L, n-1);
    return 1;
  }
  else {
    lua_Integer i = luaL_checkinteger(L, 1);
    if (i < 0) i = n + i;
    else if (i > n) i = n;
    luaL_argcheck(L, 1 <= i, 1, "index out of range");
    return n - (int)i;
  }
}


/*
** Continuation function for 'pcall' and 'xpcall'. Both functions
** already pushed a 'true' before doing the call, so in case of success
** 'finishpcall' only has to return everything in the stack minus
** 'extra' values (where 'extra' is exactly the number of items to be
** ignored).
*/
static int finishpcall (lua_State *L, int status, lua_KContext extra) {
  if (status != LUA_OK && status != LUA_YIELD) {  /* error? */
    lua_pushboolean(L, 0);  /* first result (false) */
    lua_pushvalue(L, -2);  /* error message */
    return 2;  /* return false, msg */
  }
  else
    return lua_gettop(L) - (int)extra;  /* return all results */
}


static int luaB_pcall (lua_State *L) {
  int status;
  luaL_checkany(L, 1);
  lua_pushboolean(L, 1);  /* first result if no errors */
  lua_insert(L, 1);  /* put it in place */
  status = lua_pcallk(L, lua_gettop(L) - 2, LUA_MULTRET, 0, 0, finishpcall);
  return finishpcall(L, status, 0);
}


/*
** Do a protected call with error handling. After 'lua_rotate', the
** stack will have <f, err, true, f, [args...]>; so, the function passes
** 2 to 'finishpcall' to skip the 2 first values when returning results.
*/
static int luaB_xpcall (lua_State *L) {
  int status;
  int n = lua_gettop(L);
  luaL_checktype(L, 2, LUA_TFUNCTION);  /* check error function */
  lua_pushboolean(L, 1);  /* first result */
  lua_pushvalue(L, 1);  /* function */
  lua_rotate(L, 3, 2);  /* move them below function's arguments */
  status = lua_pcallk(L, n - 2, LUA_MULTRET, 2, 2, finishpcall);
  return finishpcall(L, status, 2);
}


static int luaB_tostring (lua_State *L) {
  luaL_checkany(L, 1);
  luaL_tolstring(L, 1, NULL);
  return 1;
}


static int luaB_vectorn (lua_State *L, int sz, float *input) {
  int counter = 0;
  int i;
  for (i=1; i<=lua_gettop(L) ; ++i) {
    switch (lua_type(L, i)) {
      case LUA_TVECTOR4:
      if (counter+4 <= sz) {
        float *x = &input[counter++];
        float *y = &input[counter++];
        float *z = &input[counter++];
        float *w = &input[counter++];
        lua_checkvector4(L, i, x,y,z,w);
      } else {
        return 0;
      }
      break;

      case LUA_TVECTOR3:
      if (counter+3 <= sz) {
        float *x = &input[counter++];
        float *y = &input[counter++];
        float *z = &input[counter++];
        lua_checkvector3(L, i, x,y,z);
      } else {
        return 0;
      }
      break;

      case LUA_TVECTOR2:
      if (counter+2 <= sz) {
        float *x = &input[counter++];
        float *y = &input[counter++];
        lua_checkvector2(L, i, x,y);
      } else {
        return 0;
      }
      break;

      case LUA_TNUMBER:
      if (counter+1 <= sz) {
        input[counter++] = lua_tonumber(L, i);
      } else {
        return 0;
      }
      break;

      default: {
        char msg[1024];
        sprintf(msg, "vector%d(...) argument %d had type %s", sz, i, lua_typename(L, lua_type(L,i)));
        luaL_error(L, msg);
      }
    }
  }
  return counter == sz;
}


static int luaB_vec (lua_State *L) {
  float input[4] = { 0 };
  switch (lua_gettop(L)) {
    case 4:
    input[3] = luaL_checknumber(L, 4);
    case 3:
    input[2] = luaL_checknumber(L, 3);
    case 2:
    input[1] = luaL_checknumber(L, 2);
    case 1:
    input[0] = luaL_checknumber(L, 1);
    break;
    default:
    luaL_error(L, "vec(...) takes 1 to 4 number arguments only");
  }
  switch (lua_gettop(L)) {
    case 4:
    lua_pushvector4(L, input[0], input[1], input[2], input[3]);
    break;
    case 3:
    lua_pushvector3(L, input[0], input[1], input[2]);
    break;
    case 2:
    lua_pushvector2(L, input[0], input[1]);
    break;
    case 1:
    lua_pushnumber(L, input[0]);
    break;
    default:;
  }
  return 1;
}


static int luaB_vector2 (lua_State *L) {
  float input[2];
  if (!luaB_vectorn(L, 2, input)) luaL_error(L, "vector2(...) requires exactly 2 numbers");
  lua_pushvector2(L, input[0], input[1]);
  return 1;
}


static int luaB_vector3 (lua_State *L) {
  float input[3];
  if (!luaB_vectorn(L, 3, input)) luaL_error(L, "vector3(...) requires exactly 3 numbers");
  lua_pushvector3(L, input[0], input[1], input[2]);
  return 1;
}


static int luaB_vector4 (lua_State *L) {
  float input[4];
  if (!luaB_vectorn(L, 4, input)) luaL_error(L, "vector4(...) requires exactly 4 numbers");
  lua_pushvector4(L, input[0], input[1], input[2], input[3]);
  return 1;
}


#define PI 3.1415926535897932385f


static float dot4 (float x1, float y1, float z1, float w1, float x2, float y2, float z2, float w2) {
  return x1*x2 + y1*y2 + z1*z2 + w1*w2;
}


static float dot3 (float x1, float y1, float z1, float x2, float y2, float z2) {
  return x1*x2 + y1*y2 + z1*z2;
}


static float dot2 (float x1, float y1, float x2, float y2) {
  return x1*x2 + y1*y2;
}


static void cross3 (float x1, float y1, float z1, float x2, float y2, float z2, float *xr, float *yr, float *zr) {
  *xr = y1*z2 - z1*y2;
  *yr = z1*x2 - x1*z2;
  *zr = x1*y2 - y1*x2;
}


static int luaB_dot (lua_State *L) {
  float x1, y1, z1, w1;
  float x2, y2, z2, w2;
  if (lua_gettop(L) != 2) luaL_error(L, "Invalid params, try dot(v,v)");
  if (lua_isvector4(L,1)) {
    lua_checkvector4(L, 1, &x1, &y1, &z1, &w1);
    lua_checkvector4(L, 2, &x2, &y2, &z2, &w2);
    lua_pushnumber(L,dot4(x1,y1,z1,w1, x2,y2,z2,w2));
  } else if (lua_isvector3(L,1)) {
    lua_checkvector3(L, 1, &x1, &y1, &z1);
    lua_checkvector3(L, 2, &x2, &y2, &z2);
    lua_pushnumber(L,dot3(x1,y1,z1, x2,y2,z2));
  } else if (lua_isvector2(L,1)) {
    lua_checkvector2(L, 1, &x1, &y1);
    lua_checkvector2(L, 2, &x2, &y2);
    lua_pushnumber(L,dot2(x1,y1, x2,y2));
  }
  return 1;
}


static int luaB_cross (lua_State *L) {
  float x1, y1, z1;
  float x2, y2, z2;
  float ax, ay, az;
  if (lua_gettop(L) != 2) luaL_error(L, "Invalid params, try cross(v,v)");
  lua_checkvector3(L, 1, &x1, &y1, &z1);
  lua_checkvector3(L, 2, &x2, &y2, &z2);
  cross3(x1,y1,z1, x2,y2,z2, &ax, &ay, &az);
  lua_pushvector3(L,ax,ay,az);
  return 1;
}


static int luaB_quat (lua_State *L) {
  if (lua_gettop(L)==4 && lua_isnumber(L,1) && lua_isnumber(L,2) && lua_isnumber(L,3) && lua_isnumber(L,4)) {
    float w,x,y,z;
    w = (float)lua_tonumber(L, 1);
    x = (float)lua_tonumber(L, 2);
    y = (float)lua_tonumber(L, 3);
    z = (float)lua_tonumber(L, 4);
    lua_pushquat(L, w,x,y,z);
    return 1;
  } else if (lua_gettop(L)==2 && lua_isnumber(L,1) && lua_isvector3(L,2)) {
    float angle,x,y,z,ha,s;
    angle = (float)lua_tonumber(L, 1);
    lua_checkvector3(L, 2, &x, &y, &z);
    ha = angle*(0.5f*PI/180.0f);
    s = sinf(ha);
    lua_pushquat(L, cosf(ha),s*x,s*y,s*z);
    return 1;
  } else if (lua_gettop(L)==2 && lua_isvector3(L,1) && lua_isvector3(L,2)) {
    float x1, y1, z1;
    float x2, y2, z2;
    float l1,l2,d;
    lua_checkvector3(L, 1, &x1, &y1, &z1);
    lua_checkvector3(L, 2, &x2, &y2, &z2);

    /* Based on Stan Melax's article in Game Programming Gems */
    l1 = sqrtf(x1*x1 + y1*y1 + z1*z1);
    l2 = sqrtf(x2*x2 + y2*y2 + z2*z2);
    x1/=l1; y1/=l1; z1/=l1; 
    x2/=l2; y2/=l2; z2/=l2; 

    d = dot3(x1,y1,z1, x2,y2,z2);

    /* If dot == 1, vectors are the same */
    if (d >= 1.0f) {
      lua_pushquat(L, 1,0,0,0);
      return 1;
    }

    if (d < (1e-6f - 1.0f)) {

      float ax, ay, az;
      float len2, len;

      cross3(1,0,0, x1,y1,z1, &ax, &ay, &az);
      len2 = ax*ax + ay*ay + az*az;
      if (len2 == 0) {
        cross3(0,1,0, x1,y1,z1, &ax, &ay, &az);
        len2 = ax*ax + ay*ay + az*az;
      }
      len = sqrtf(len2);
      ax/=len; ay/=len; az/=len; 
      lua_pushquat(L, 0,ax,ay,az);
      return 1;

    } else {

      float s = sqrtf((1+d)*2);
      float ax, ay, az;
      float qw, qlen;

      cross3(x1,y1,z1, x2,y2,z2, &ax, &ay, &az);
      ax/=s; ay/=s; az/=s; 
      qw = s*0.5f;
      qlen = sqrtf(qw*qw + ax*ax + ay*ay + az*az);
      lua_pushquat(L, qw/qlen,ax/qlen,ay/qlen,az/qlen);
      return 1;

    }

  } else {
    luaL_error(L, "Invalid params, try quat(n,n,n,n) quat(n,v3) quat(v3,v3)");
    return 0;
  }
}


static int luaB_inv (lua_State *L) {
  float w, x, y, z;
  if (lua_gettop(L) != 1) luaL_error(L, "Invalid params, try inv(q)");
  lua_checkquat(L, 1, &w, &x, &y, &z);
  /* don't invert w, as that would mean inv(Q_ID) would flip the polarity of w */
  lua_pushquat(L,w,-x,-y,-z);
  return 1;
}


static int luaB_slerp (lua_State *L) {
  float w1, x1, y1, z1;
  float w2, x2, y2, z2;
  float t, theta, dot;
  if (lua_gettop(L) != 3) luaL_error(L, "Invalid params, try slerp(q1,q2,a)");
  lua_checkquat(L, 1, &w1, &x1, &y1, &z1);
  lua_checkquat(L, 2, &w2, &x2, &y2, &z2);
  t = (float)lua_tonumber(L, 3);

  dot = w1*w2 + x1*x2 + y1*y2 + z1*z2;
  if (dot < 0) {
    /* flip one of them */
    w2 *= -1;
    x2 *= -1;
    y2 *= -1;
    z2 *= -1;
    /* update dot to match */
    dot *= -1;
  }

  /* dot > 0 now */
    
  theta = acosf(dot);
  if (dot != 1) {
    float d = 1.0f / sinf(theta);
    float s0 = sinf((1.0f - t) * theta);
    float s1 = sinf(t * theta);
    lua_pushquat(L,
        d * (w1*s0 + w2*s1),
        d * (x1*s0 + x2*s1),
        d * (y1*s0 + y2*s1),
        d * (z1*s0 + z2*s1)
    );
  } else {
    lua_pushquat(L,w1,x1,y1,z1);
  }

  return 1;
}


static int luaB_norm (lua_State *L) {
  if (lua_gettop(L)==1 && lua_isvector2(L,1)) {
    float x, y;
    float len;
    lua_checkvector2(L, 1, &x, &y);
    len = sqrtf(x*x + y*y);
    if (len == 0)
        luaL_error(L, "Cannot normalise vector2(0,0)");
    lua_pushvector2(L,x/len,y/len);
    return 1;
  } else if (lua_gettop(L)==1 && lua_isvector3(L,1)) {
    float x, y, z;
    float len;
    lua_checkvector3(L, 1, &x, &y, &z);
    len = sqrtf(x*x + y*y + z*z);
    if (len == 0)
        luaL_error(L, "Cannot normalise vector3(0,0,0)");
    lua_pushvector3(L,x/len,y/len,z/len);
    return 1;
  } else if (lua_gettop(L)==1 && lua_isvector4(L,1)) {
    float x, y, z, w;
    float len;
    lua_checkvector4(L, 1, &x, &y, &z, &w);
    len = sqrtf(x*x + y*y + z*z + w*w);
    if (len == 0)
        luaL_error(L, "Cannot normalise vector4(0,0,0,0)");
    lua_pushvector4(L,x/len,y/len,z/len,w/len);
    return 1;
  } else if (lua_gettop(L)==1 && lua_isquat(L,1)) {
    float w, x, y, z;
    float qlen;
    lua_checkquat(L, 1, &w, &x, &y, &z);
    qlen = sqrtf(w*w + x*x + y*y + z*z);
    if (qlen == 0)
        luaL_error(L, "Cannot normalise quat(0,0,0,0)");
    lua_pushquat(L,w/qlen,x/qlen,y/qlen,z/qlen);
    return 1;
  } else {
    return luaL_error(L, "Invalid arguments, try norm(v) or norm(q).");
  }
}


static const luaL_Reg base_funcs[] = {
  {"assert", luaB_assert},
  {"collectgarbage", luaB_collectgarbage},
  {"dofile", luaB_dofile},
  {"error", luaB_error},
  {"getmetatable", luaB_getmetatable},
  {"ipairs", luaB_ipairs},
  {"loadfile", luaB_loadfile},
  {"load", luaB_load},
#if defined(LUA_COMPAT_LOADSTRING)
  {"loadstring", luaB_load},
#endif
  {"next", luaB_next},
  {"pairs", luaB_pairs},
  {"pcall", luaB_pcall},
  {"print", luaB_print},
  {"rawequal", luaB_rawequal},
  {"rawlen", luaB_rawlen},
  {"rawget", luaB_rawget},
  {"rawset", luaB_rawset},
  {"select", luaB_select},
  {"setmetatable", luaB_setmetatable},
  {"tonumber", luaB_tonumber},
  {"tostring", luaB_tostring},
  {"xpcall", luaB_xpcall},
  /* placeholders */
  
  {"vec", luaB_vec},
  {"vec4", luaB_vector4},
  {"vec3", luaB_vector3},
  {"vec2", luaB_vector2},
  {"vector4", luaB_vector4},
  {"vector3", luaB_vector3},
  {"vector2", luaB_vector2},
  {"quat", luaB_quat},
  {"dot", luaB_dot},
  {"cross", luaB_cross},
  {"inv", luaB_inv},
  {"slerp", luaB_slerp},
  {"norm", luaB_norm},
  
  {"type", NULL},
  {"_G", NULL},
  {"_VERSION", NULL},
  {NULL, NULL}
};


LUAMOD_API int luaopen_base (lua_State *L) {
  int i;
  /* open lib into global table */
  lua_pushglobaltable(L);
  luaL_setfuncs(L, base_funcs, 0);
  /* set global _G */
  lua_pushvalue(L, -1);
  lua_setfield(L, -2, "_G");
  /* set global _VERSION */
  lua_pushliteral(L, LUA_VERSION);
  lua_setfield(L, -2, "_VERSION");
  /* set function 'type' with proper upvalues */
  for (i = 0; i < LUA_NUMTAGS; i++)  /* push all type names as upvalues */
    lua_pushstring(L, lua_typename(L, i));
  lua_pushcclosure(L, luaB_type, LUA_NUMTAGS);
  lua_setfield(L, -2, "type");
  return 1;
}

