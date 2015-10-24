/*
** $Id: lmathlib.c,v 1.115 2015/03/12 14:04:04 roberto Exp $
** Standard mathematical library
** See Copyright Notice in lua.h
*/

#define lmathlib_c
#define LUA_LIB

#include "lprefix.h"


#include <stdlib.h>
#include <math.h>

#include "lua.h"

#include "lauxlib.h"
#include "lualib.h"


#undef PI
#define PI	(l_mathop(3.141592653589793238462643383279502884))


#if !defined(l_rand)		/* { */
#if defined(LUA_USE_POSIX)
#define l_rand()	random()
#define l_srand(x)	srandom(x)
#define L_RANDMAX	2147483647	/* (2^31 - 1), following POSIX */
#else
#define l_rand()	rand()
#define l_srand(x)	srand(x)
#define L_RANDMAX	RAND_MAX
#endif
#endif				/* } */


static int math_abs (lua_State *L) {
  lua_Number v;
  float x, y, z, w;
  switch (lua_type(L,1)) {

    case LUA_TNUMBER:
    if (lua_isinteger(L, 1)) {
      lua_Integer n = lua_tointeger(L, 1);
      if (n < 0) n = (lua_Integer)(0u - n);
      lua_pushinteger(L, n);
	} else {
      v = lua_tonumber(L,1);
      lua_pushnumber(L,l_mathop(fabs)(v));
	}
    return 1;
    case LUA_TVECTOR2:
    lua_checkvector2(L,1,&x,&y);
    lua_pushvector2(L,fabsf(x),fabsf(y));
    return 1;
    case LUA_TVECTOR3:
    lua_checkvector3(L,1,&x,&y,&z);
    lua_pushvector3(L,fabsf(x),fabsf(y),fabsf(z));
    return 1;
    case LUA_TVECTOR4:
    lua_checkvector4(L,1,&x,&y,&z,&w);
    lua_pushvector4(L,fabsf(x),fabsf(y),fabsf(z),fabsf(w));
    return 1;
  }
  luaL_error(L, "abs takes a number, integer, vector2, vector3, or vector4.");
  return 1;
}

static int math_sin (lua_State *L) {
  lua_pushnumber(L, l_mathop(sin)(luaL_checknumber(L, 1)));
  return 1;
}

static int math_cos (lua_State *L) {
  lua_pushnumber(L, l_mathop(cos)(luaL_checknumber(L, 1)));
  return 1;
}

static int math_tan (lua_State *L) {
  lua_pushnumber(L, l_mathop(tan)(luaL_checknumber(L, 1)));
  return 1;
}

static int math_asin (lua_State *L) {
  lua_pushnumber(L, l_mathop(asin)(luaL_checknumber(L, 1)));
  return 1;
}

static int math_acos (lua_State *L) {
  lua_pushnumber(L, l_mathop(acos)(luaL_checknumber(L, 1)));
  return 1;
}

static int math_atan (lua_State *L) {
  lua_Number y = luaL_checknumber(L, 1);
  lua_Number x = luaL_optnumber(L, 2, 1);
  lua_pushnumber(L, l_mathop(atan2)(y, x));
  return 1;
}


static int math_toint (lua_State *L) {
  int valid;
  lua_Integer n = lua_tointegerx(L, 1, &valid);
  if (valid)
    lua_pushinteger(L, n);
  else {
    luaL_checkany(L, 1);
    lua_pushnil(L);  /* value is not convertible to integer */
  }
  return 1;
}


static void pushnumint (lua_State *L, lua_Number d) {
  lua_Integer n;
  if (lua_numbertointeger(d, &n))  /* does 'd' fit in an integer? */
    lua_pushinteger(L, n);  /* result is integer */
  else
    lua_pushnumber(L, d);  /* result is float */
}


static int math_floor (lua_State *L) {
  lua_Number v;
  float x, y, z, w;
  switch (lua_type(L,1)) {
    case LUA_TNUMBER:
	if (lua_isinteger(L, 1)) {
      lua_settop(L, 1); /* integer is its own floor */
	} else {
      v = lua_tonumber(L,1);
      pushnumint(L,floor(v));
	}
    return 1;
    case LUA_TVECTOR2:
    lua_checkvector2(L,1,&x,&y);
    lua_pushvector2(L,floorf(x),floorf(y));
    return 1;
    case LUA_TVECTOR3:
    lua_checkvector3(L,1,&x,&y,&z);
    lua_pushvector3(L,floorf(x),floorf(y),floorf(z));
    return 1;
    case LUA_TVECTOR4:
    lua_checkvector4(L,1,&x,&y,&z,&w);
    lua_pushvector4(L,floorf(x),floorf(y),floorf(z),floorf(w));
    return 1;
  }
  luaL_error(L, "floor takes a number, integer, vector2, vector3, or vector4.");
  return 1;
}


static int math_ceil (lua_State *L) {
  lua_Number v;
  float x, y, z, w;
  switch (lua_type(L,1)) {
    case LUA_TNUMBER:
    if (lua_isinteger(L, 1)) {
      lua_settop(L, 1); /* integer is its own ceil */
	} else {
      v = lua_tonumber(L,1);
      pushnumint(L,ceil(v));
	}
    return 1;
    case LUA_TVECTOR2:
    lua_checkvector2(L,1,&x,&y);
    lua_pushvector2(L,ceilf(x),ceilf(y));
    return 1;
    case LUA_TVECTOR3:
    lua_checkvector3(L,1,&x,&y,&z);
    lua_pushvector3(L,ceilf(x),ceilf(y),ceilf(z));
    return 1;
    case LUA_TVECTOR4:
    lua_checkvector4(L,1,&x,&y,&z,&w);
    lua_pushvector4(L,ceilf(x),ceilf(y),ceilf(z),ceilf(w));
    return 1;
  }
  luaL_error(L, "ceil takes a number, integer, vector2, vector3, or vector4.");
  return 1;
}


static int math_fmod (lua_State *L) {
  if (lua_isinteger(L, 1) && lua_isinteger(L, 2)) {
    lua_Integer d = lua_tointeger(L, 2);
    if ((lua_Unsigned)d + 1u <= 1u) {  /* special cases: -1 or 0 */
      luaL_argcheck(L, d != 0, 2, "zero");
      lua_pushinteger(L, 0);  /* avoid overflow with 0x80000... / -1 */
    }
    else
      lua_pushinteger(L, lua_tointeger(L, 1) % d);
  }
  else
    lua_pushnumber(L, l_mathop(fmod)(luaL_checknumber(L, 1),
                                     luaL_checknumber(L, 2)));
  return 1;
}


/*
** next function does not use 'modf', avoiding problems with 'double*'
** (which is not compatible with 'float*') when lua_Number is not
** 'double'.
*/
static int math_modf (lua_State *L) {
  if (lua_isinteger(L ,1)) {
    lua_settop(L, 1);  /* number is its own integer part */
    lua_pushnumber(L, 0);  /* no fractional part */
  }
  else {
    lua_Number n = luaL_checknumber(L, 1);
    /* integer part (rounds toward zero) */
    lua_Number ip = (n < 0) ? l_mathop(ceil)(n) : l_mathop(floor)(n);
    pushnumint(L, ip);
    /* fractional part (test needed for inf/-inf) */
    lua_pushnumber(L, (n == ip) ? l_mathop(0.0) : (n - ip));
  }
  return 2;
}


static int math_sqrt (lua_State *L) {
  lua_pushnumber(L, l_mathop(sqrt)(luaL_checknumber(L, 1)));
  return 1;
}


static int math_ult (lua_State *L) {
  lua_Integer a = luaL_checkinteger(L, 1);
  lua_Integer b = luaL_checkinteger(L, 2);
  lua_pushboolean(L, (lua_Unsigned)a < (lua_Unsigned)b);
  return 1;
}

static int math_log (lua_State *L) {
  lua_Number x = luaL_checknumber(L, 1);
  lua_Number res;
  if (lua_isnoneornil(L, 2))
    res = l_mathop(log)(x);
  else {
    lua_Number base = luaL_checknumber(L, 2);
#if !defined(LUA_USE_C89)
    if (base == 2.0) res = l_mathop(log2)(x); else
#endif
    if (base == 10.0) res = l_mathop(log10)(x);
    else res = l_mathop(log)(x)/l_mathop(log)(base);
  }
  lua_pushnumber(L, res);
  return 1;
}

static int math_exp (lua_State *L) {
  lua_pushnumber(L, l_mathop(exp)(luaL_checknumber(L, 1)));
  return 1;
}

static int math_deg (lua_State *L) {
  lua_pushnumber(L, luaL_checknumber(L, 1) * (l_mathop(180.0) / PI));
  return 1;
}

static int math_rad (lua_State *L) {
  lua_pushnumber(L, luaL_checknumber(L, 1) * (PI / l_mathop(180.0)));
  return 1;
}


static int math_min (lua_State *L) {
  int n = lua_gettop(L);  /* number of arguments */
  int imin = 1;  /* index of current minimum value */
  int i;
  luaL_argcheck(L, n >= 1, 1, "value expected");
  for (i = 2; i <= n; i++) {
    if (lua_compare(L, i, imin, LUA_OPLT))
      imin = i;
  }
  lua_pushvalue(L, imin);
  return 1;
}


static int math_max (lua_State *L) {
  int n = lua_gettop(L);  /* number of arguments */
  int imax = 1;  /* index of current maximum value */
  int i;
  luaL_argcheck(L, n >= 1, 1, "value expected");
  for (i = 2; i <= n; i++) {
    if (lua_compare(L, imax, i, LUA_OPLT))
      imax = i;
  }
  lua_pushvalue(L, imax);
  return 1;
}

/*
** This function uses 'double' (instead of 'lua_Number') to ensure that
** all bits from 'l_rand' can be represented, and that 'RANDMAX + 1.0'
** will keep full precision (ensuring that 'r' is always less than 1.0.)
*/
static int math_random (lua_State *L) {
  lua_Integer low, up;
  double r = (double)l_rand() * (1.0 / ((double)L_RANDMAX + 1.0));
  switch (lua_gettop(L)) {  /* check number of arguments */
    case 0: {  /* no arguments */
      lua_pushnumber(L, (lua_Number)r);  /* Number between 0 and 1 */
      return 1;
    }
    case 1: {  /* only upper limit */
      low = 1;
      up = luaL_checkinteger(L, 1);
      break;
    }
    case 2: {  /* lower and upper limits */
      low = luaL_checkinteger(L, 1);
      up = luaL_checkinteger(L, 2);
      break;
    }
    default: return luaL_error(L, "wrong number of arguments");
  }
  /* random integer in the interval [low, up] */
  luaL_argcheck(L, low <= up, 1, "interval is empty"); 
  luaL_argcheck(L, low >= 0 || up <= LUA_MAXINTEGER + low, 1,
                   "interval too large");
  r *= (double)(up - low) + 1.0;
  lua_pushinteger(L, (lua_Integer)r + low);
  return 1;
}


static int math_randomseed (lua_State *L) {
  l_srand((unsigned int)(lua_Integer)luaL_checknumber(L, 1));
  (void)rand(); /* discard first value to avoid undesirable correlations */
  return 0;
}


static int math_type (lua_State *L) {
  if (lua_type(L, 1) == LUA_TNUMBER) {
      if (lua_isinteger(L, 1))
        lua_pushliteral(L, "integer"); 
      else
        lua_pushliteral(L, "float"); 
  }
  else {
    luaL_checkany(L, 1);
    lua_pushnil(L);
  }
  return 1;
}


/*
** {==================================================================
** Deprecated functions (for compatibility only)
** ===================================================================
*/
#if defined(LUA_COMPAT_MATHLIB)

static int math_cosh (lua_State *L) {
  lua_pushnumber(L, l_mathop(cosh)(luaL_checknumber(L, 1)));
  return 1;
}

static int math_sinh (lua_State *L) {
  lua_pushnumber(L, l_mathop(sinh)(luaL_checknumber(L, 1)));
  return 1;
}

static int math_tanh (lua_State *L) {
  lua_pushnumber(L, l_mathop(tanh)(luaL_checknumber(L, 1)));
  return 1;
}

static int math_pow (lua_State *L) {
  if (lua_gettop(L) != 2) {
    luaL_error(L, "Wrong number of arguments: use math.pow(number, number) or math.pow(quat, number)");
  }
  if (lua_type(L, 1) != LUA_TNUMBER && lua_type(L, 1) != LUA_TQUAT) {
    luaL_error(L, "Invalid type for 1st parameter, use math.pow(number, number) or math.pow(quat, number)");
  }
  if (lua_type(L, 2) != LUA_TNUMBER) {
    luaL_error(L, "math.pow second argument must be a number");
  }

  if (lua_type(L, 1) == LUA_TNUMBER) {
      lua_pushnumber(L, pow(lua_tonumber(L, 1), lua_tonumber(L, 2)));
  } else {
    float w, x, y, z,  l;
    float index;
    lua_checkquat(L, 1, &w, &x, &y, &z);
    index = (float)lua_tonumber(L, 2);
    l = sqrtf(x*x + y*y + z*z);
    if (l==0) {
      lua_pushquat(L, 1, 0, 0, 0);
    } else {
      float angle, sangle;
      float w2, x2, y2, z2;
      angle = index * acosf(w); /* without the factor of 2 */
      sangle = sinf(angle);
      w2 = cosf(angle);
      x2 = sangle * x/l;
      y2 = sangle * y/l;
      z2 = sangle * z/l;
      lua_pushquat(L, w2, x2, y2, z2);
    }
  }
  return 1;
}

static int math_frexp (lua_State *L) {
  int e;
  lua_pushnumber(L, l_mathop(frexp)(luaL_checknumber(L, 1), &e));
  lua_pushinteger(L, e);
  return 2;
}

static int math_ldexp (lua_State *L) {
  lua_Number x = luaL_checknumber(L, 1);
  int ep = (int)luaL_checkinteger(L, 2);
  lua_pushnumber(L, l_mathop(ldexp)(x, ep));
  return 1;
}

static int math_log10 (lua_State *L) {
  lua_pushnumber(L, l_mathop(log10)(luaL_checknumber(L, 1)));
  return 1;
}

#endif
/* }================================================================== */

static float do_clamp (float a, float b, float c)
{
  if (a<b) return b;
  if (a>c) return c;
  return a;
}

static int math_clamp (lua_State *L) {
  if (lua_gettop(L)!=3) return luaL_error(L, "wrong number of arguments");
  switch (lua_type(L,1)) {
    case LUA_TNUMBER: {
      lua_Number a, b, c;
      a = luaL_checknumber(L,1);
      b = luaL_checknumber(L,2);
      c = luaL_checknumber(L,3);
      if (a<b) a = b;
      if (a>c) a = c;
      lua_pushnumber(L, a);
    } break;
    case LUA_TVECTOR2: {
      float xa, ya;
      float xb, yb;
      float xc, yc;
      lua_checkvector2(L, 1, &xa, &ya);
      lua_checkvector2(L, 2, &xb, &yb);
      lua_checkvector2(L, 3, &xc, &yc);
      lua_pushvector2(L, do_clamp(xa,xb,xc), do_clamp(ya,yb,yc));
    } break;
    case LUA_TVECTOR3: {
      float xa, ya, za;
      float xb, yb, zb;
      float xc, yc, zc;
      lua_checkvector3(L, 1, &xa, &ya, &za);
      lua_checkvector3(L, 2, &xb, &yb, &zb);
      lua_checkvector3(L, 3, &xc, &yc, &zc);
      lua_pushvector3(L, do_clamp(xa,xb,xc), do_clamp(ya,yb,yc), do_clamp(za,zb,zc));
    } break;
    case LUA_TVECTOR4: {
      float xa, ya, za, wa;
      float xb, yb, zb, wb;
      float xc, yc, zc, wc;
      lua_checkvector4(L, 1, &xa, &ya, &za, &wa);
      lua_checkvector4(L, 2, &xb, &yb, &zb, &wb);
      lua_checkvector4(L, 3, &xc, &yc, &zc, &wc);
      lua_pushvector4(L, do_clamp(xa,xb,xc), do_clamp(ya,yb,yc), do_clamp(za,zb,zc), do_clamp(wa,wb,wc));
    } break;
    default: return luaL_error(L, "clamp only works on number, vector2, vector3, vector4");
  }
  return 1;
}

static const luaL_Reg mathlib[] = {
  {"abs",   math_abs},
  {"acos",  math_acos},
  {"asin",  math_asin},
  {"atan",  math_atan},
  {"ceil",  math_ceil},
  {"cos",   math_cos},
  {"deg",   math_deg},
  {"exp",   math_exp},
  {"tointeger", math_toint},
  {"floor", math_floor},
  {"fmod",   math_fmod},
  {"ult",   math_ult},
  {"log",   math_log},
  {"max",   math_max},
  {"min",   math_min},
  {"modf",   math_modf},
  {"rad",   math_rad},
  {"random",     math_random},
  {"randomseed", math_randomseed},
  {"sin",   math_sin},
  {"sqrt",  math_sqrt},
  {"tan",   math_tan},
  {"type", math_type},

  { "clamp", math_clamp },

#if defined(LUA_COMPAT_MATHLIB)
  {"atan2", math_atan},
  {"cosh",   math_cosh},
  {"sinh",   math_sinh},
  {"tanh",   math_tanh},
  {"pow",   math_pow},
  {"frexp", math_frexp},
  {"ldexp", math_ldexp},
  {"log10", math_log10},
#endif
  /* placeholders */
  {"pi", NULL},
  {"huge", NULL},
  {"maxinteger", NULL},
  {"mininteger", NULL},
  {NULL, NULL}
};


/*
** Open math library
*/
LUAMOD_API int luaopen_math(lua_State *L) {
	luaL_newlib(L, mathlib);
	lua_pushnumber(L, PI);
	lua_setfield(L, -2, "pi");
	lua_pushnumber(L, (lua_Number)HUGE_VAL);
	lua_setfield(L, -2, "huge");
	lua_pushinteger(L, LUA_MAXINTEGER);
	lua_setfield(L, -2, "maxinteger");
	lua_pushinteger(L, LUA_MININTEGER);
	lua_setfield(L, -2, "mininteger");

	return 1;
}