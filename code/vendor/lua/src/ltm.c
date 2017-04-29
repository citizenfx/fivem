/*
** $Id: ltm.c,v 2.34 2015/03/30 15:42:27 roberto Exp $
** Tag methods
** See Copyright Notice in lua.h
*/

#define ltm_c
#define LUA_CORE

#include "lprefix.h"


#include <string.h>

#include "lua.h"

#include "ldebug.h"
#include "ldo.h" 
#include "lobject.h"
#include "lstate.h"
#include "lstring.h"
#include "ltable.h"
#include "ltm.h"
#include "lvm.h"


static const char udatatypename[] = "userdata";

LUAI_DDEF const char *const luaT_typenames_[LUA_TOTALTAGS] = {
  "no value",
  "nil", "boolean", udatatypename, "number",
  "vector2", "vector3", "vector4", "quat",
  "string", "table", "function", udatatypename, "thread",
  "proto" /* this last case is used for tests only */
};


void luaT_init (lua_State *L) {
  static const char *const luaT_eventname[] = {  /* ORDER TM */
    "__index", "__newindex",
    "__gc", "__mode", "__len", "__eq",
    "__add", "__sub", "__mul", "__mod", "__pow",
    "__div", "__idiv",
    "__band", "__bor", "__bxor", "__shl", "__shr",
    "__unm", "__bnot", "__lt", "__le",
    "__concat", "__call"
  };
  int i;
  for (i=0; i<TM_N; i++) {
    G(L)->tmname[i] = luaS_new(L, luaT_eventname[i]);
    luaC_fix(L, obj2gco(G(L)->tmname[i]));  /* never collect these names */
  }
}


/*
** function to be used with macro "fasttm": optimized for absence of
** tag methods
*/
const TValue *luaT_gettm (Table *events, TMS event, TString *ename) {
  const TValue *tm = luaH_getstr(events, ename);
  lua_assert(event <= TM_EQ);
  if (ttisnil(tm)) {  /* no tag method? */
    events->flags |= cast_byte(1u<<event);  /* cache this fact */
    return NULL;
  }
  else return tm;
}


const TValue *luaT_gettmbyobj (lua_State *L, const TValue *o, TMS event) {
  Table *mt;
  switch (ttnov(o)) {
    case LUA_TTABLE:
      mt = hvalue(o)->metatable;
      break;
    case LUA_TUSERDATA:
      mt = uvalue(o)->metatable;
      break;
    default:
      mt = G(L)->mt[ttnov(o)];
  }
  return (mt ? luaH_getstr(mt, G(L)->tmname[event]) : luaO_nilobject);
}


void luaT_callTM (lua_State *L, const TValue *f, const TValue *p1,
                  const TValue *p2, TValue *p3, int hasres) {
  ptrdiff_t result = savestack(L, p3);
  setobj2s(L, L->top++, f);  /* push function (assume EXTRA_STACK) */
  setobj2s(L, L->top++, p1);  /* 1st argument */
  setobj2s(L, L->top++, p2);  /* 2nd argument */
  if (!hasres)  /* no result? 'p3' is third argument */
    setobj2s(L, L->top++, p3);  /* 3rd argument */
  /* metamethod may yield only when called from Lua code */
  luaD_call(L, L->top - (4 - hasres), hasres, isLua(L->ci));
  if (hasres) {  /* if has result, move it to its place */
    p3 = restorestack(L, result);
    setobjs2s(L, p3, --L->top);
  }
}


int luaT_callbinTM (lua_State *L, const TValue *p1, const TValue *p2,
                    StkId res, TMS event) {
  const TValue *tm = luaT_gettmbyobj(L, p1, event);  /* try first operand */
  if (ttisnil(tm))
    tm = luaT_gettmbyobj(L, p2, event);  /* try second operand */
  if (ttisnil(tm)) return 0;
  luaT_callTM(L, tm, p1, p2, res, 1);
  return 1;
}


static float addf(float x, float y) { return x + y; }
static float subf(float x, float y) { return x - y; }
static float mulf(float x, float y) { return x * y; }
static float divf(float x, float y) { return x / y; }

#define PW2(f)      r.x = f(nb.x, nc.x); r.y = f(nb.y, nc.y);
#define SCALAR2(f)  r.x = f(nb.x, nc); r.y = f(nb.y, nc);
#define SCALAR2B(f) r.x = f(nc, nb.x); r.y = f(nc, nb.y);
#define PW3(f)      r.x = f(nb.x, nc.x); r.y = f(nb.y, nc.y); r.z = f(nb.z, nc.z);
#define SCALAR3(f)  r.x = f(nb.x, nc); r.y = f(nb.y, nc); r.z = f(nb.z, nc);
#define SCALAR3B(f) r.x = f(nc, nb.x); r.y = f(nc, nb.y); r.z = f(nc, nb.z);
#define PW4(f)      r.x = f(nb.x, nc.x); r.y = f(nb.y, nc.y); r.z = f(nb.z, nc.z);  r.w = f(nb.w, nc.w);
#define SCALAR4(f)  r.x = f(nb.x, nc); r.y = f(nb.y, nc); r.z = f(nb.z, nc);  r.w = f(nb.w, nc);
#define SCALAR4B(f) r.x = f(nc, nb.x); r.y = f(nc, nb.y); r.z = f(nc, nb.z);  r.w = f(nc, nb.w);


void luaT_trybinTM (lua_State *L, const TValue *p1, const TValue *p2,
                    StkId res, TMS event) {
	TMS op = event;
	const StkId rb = p1;
	const StkId rc = p2;
	StkId ra = res;

  if (ttisvector2(rb) && ttisvector2(rc)) {
    lua_Float4 nb = v2value(rb), nc = v2value(rc);
    lua_Float4 r;
    switch (op) {
      case TM_ADD: PW2(addf); break;
      case TM_SUB: PW2(subf); break;
      case TM_MUL: PW2(mulf); break;
      case TM_DIV:
        if (nc.x==0.0 || nc.y==0.0) {
          luaG_runerror(L, "division by zero");
        }
        PW2(divf);
        break;
      case TM_MOD: PW2(fmodf); break;
      case TM_POW: PW2(powf); break;
      case TM_UNM: r.x = -nb.x; r.y = -nb.y; break;
      default: lua_assert(0); break;
    }
    setv2value(ra, r);

	return;
  }
  else if (ttisvector3(rb) && ttisvector3(rc)) {
    lua_Float4 nb = v3value(rb), nc = v3value(rc);
    lua_Float4 r;
    switch (op) {
      case TM_ADD: PW3(addf); break;
      case TM_SUB: PW3(subf); break;
      case TM_MUL: PW3(mulf); break;
      case TM_DIV:
        if (nc.x==0.0 || nc.y==0.0 || nc.z==0.0) {
          luaG_runerror(L, "division by zero");
        }
        PW3(divf);
        break;
      case TM_MOD: PW3(fmodf); break;
      case TM_POW: PW3(powf); break;
      case TM_UNM: r.x = -nb.x; r.y = -nb.y; r.z = -nb.z; break;
      default: lua_assert(0); break;
    }
    setv3value(ra, r);

	return;
  }
  else if (ttisvector4(rb) && ttisvector4(rc)) {
    lua_Float4 nb = v4value(rb), nc = v4value(rc);
    lua_Float4 r;
    switch (op) {
      case TM_ADD: PW4(addf); break;
      case TM_SUB: PW4(subf); break;
      case TM_MUL: PW4(mulf); break;
      case TM_DIV:
        if (nc.x==0.0 || nc.y==0.0 || nc.z==0.0 || nc.w==0.0) {
          luaG_runerror(L, "division by zero");
        }
        PW4(divf);
        break;
      case TM_MOD: PW4(fmodf); break;
      case TM_POW: PW4(powf); break;
      case TM_UNM: r.x = -nb.x; r.y = -nb.y; r.z = -nb.z; r.w = -nb.w; break;
      default: lua_assert(0); break;
    }
    setv4value(ra, r);

	return;
  }
  else if (ttisvector2(rb) && ttisnumber(rc)) {
    lua_Float4 nb = v2value(rb);
    float nc = (float)nvalue(rc);
    lua_Float4 r;
    switch (op) {
      case TM_MUL: SCALAR2(mulf); break;
      case TM_ADD: SCALAR2(addf); break;
      case TM_SUB: SCALAR2(subf); break;
      case TM_DIV:
        if (nc==0.0) {
          luaG_runerror(L, "division by zero");
        }
        SCALAR2(divf);
        break;
      case TM_MOD: SCALAR2(fmodf); break;
      case TM_POW: SCALAR2(powf); break;
      default: luaG_runerror(L, "Cannot use that op with vector2 and number");
    }
    setv2value(ra, r);

	return;
  }
  else if (ttisvector3(rb) && ttisnumber(rc)) {
    lua_Float4 nb = v3value(rb);
    float nc = (float)nvalue(rc);
    lua_Float4 r;
    switch (op) {
      case TM_MUL: SCALAR3(mulf); break;
      case TM_ADD: SCALAR3(addf); break;
      case TM_SUB: SCALAR3(subf); break;
      case TM_DIV:
        if (nc==0.0) {
          luaG_runerror(L, "division by zero");
        }
        SCALAR3(divf);
        break;
      case TM_MOD: SCALAR3(fmodf); break;
      case TM_POW: SCALAR3(powf); break;
      default: luaG_runerror(L, "Cannot use that op with vector3 and number");
    }
    setv3value(ra, r);

	return;
  }
  else if (ttisvector4(rb) && ttisnumber(rc)) {
    lua_Float4 nb = v4value(rb);
    float nc = (float)nvalue(rc);
    lua_Float4 r;
    switch (op) {
      case TM_MUL: SCALAR4(mulf); break;
      case TM_ADD: SCALAR4(addf); break;
      case TM_SUB: SCALAR4(subf); break;
      case TM_DIV:
        if (nc==0.0) {
          luaG_runerror(L, "division by zero");
        }
        SCALAR4(divf);
        break;
      case TM_MOD: SCALAR4(fmodf); break;
      case TM_POW: SCALAR4(powf); break;
      default: luaG_runerror(L, "Cannot use that op with vector4 and number");
    }
    setv4value(ra, r);

	return;
  }
  else if (ttisnumber(rb) && ttisvector2(rc)) {
    lua_Float4 nb = v2value(rc);
    float nc = (float)nvalue(rb);
    lua_Float4 r;
    switch (op) {
      case TM_MUL: SCALAR2B(mulf); break;
      case TM_ADD: SCALAR2B(addf); break;
      case TM_SUB: SCALAR2B(subf); break;
      case TM_DIV:
        if (nb.x==0.0 || nb.y==0.0) {
          luaG_runerror(L, "division by zero");
        }
        SCALAR2B(divf);
        break;
      case TM_POW: SCALAR2B(powf); break;
      default: luaG_runerror(L, "Cannot use that op with number and vector2");
    }
    setv2value(ra, r);

	return;
  }
  else if (ttisnumber(rb) && ttisvector3(rc)) {
    lua_Float4 nb = v3value(rc);
    float nc = (float)nvalue(rb);
    lua_Float4 r;
    switch (op) {
      case TM_MUL: SCALAR3B(mulf); break;
      case TM_ADD: SCALAR3B(addf); break;
      case TM_SUB: SCALAR3B(subf); break;
      case TM_DIV:
        if (nb.x==0.0 || nb.y==0.0 || nb.z==0.0) {
          luaG_runerror(L, "division by zero");
        }
        SCALAR3B(divf);
        break;
      case TM_POW: SCALAR3B(powf); break;
      default: luaG_runerror(L, "Cannot use that op with number and vector3");
    }
    setv3value(ra, r);

	return;
  }
  else if (ttisnumber(rb) && ttisvector4(rc)) {
    lua_Float4 nb = v4value(rc);
    float nc = (float)nvalue(rb);
    lua_Float4 r;
    switch (op) {
      case TM_MUL: SCALAR4B(mulf); break;
      case TM_ADD: SCALAR4B(addf); break;
      case TM_SUB: SCALAR4B(subf); break;
      case TM_DIV:
        if (nb.x==0.0 || nb.y==0.0 || nb.z==0.0 || nb.w==0.0) {
          luaG_runerror(L, "division by zero");
        }
        SCALAR4B(divf);
        break;
      case TM_POW: SCALAR4B(powf); break;
      default: luaG_runerror(L, "Cannot use that op with number and vector4");
    }
    setv4value(ra, r);

	return;
  }
  else if (ttisquat(rb) && ttisquat(rc)) {
    lua_Float4 nb = qvalue(rb), nc = qvalue(rc);
    lua_Float4 r;
    switch (op) {
      case TM_MUL:
      r.w = nb.w*nc.w - nb.x*nc.x - nb.y*nc.y - nb.z*nc.z;
      r.x = nb.w*nc.x + nb.x*nc.w + nb.y*nc.z - nb.z*nc.y;
      r.y = nb.w*nc.y + nb.y*nc.w + nb.z*nc.x - nb.x*nc.z;
      r.z = nb.w*nc.z + nb.z*nc.w + nb.x*nc.y - nb.y*nc.x;
      break;
      default: luaG_runerror(L, "Cannot use that op with quat and quat");
    }
    setqvalue(ra, r);

	return;
  }
  else if (ttisquat(rb) && ttisvector3(rc)) {
    lua_Float4 nb = qvalue(rb), nc = v3value(rc);
    lua_Float4 r;
    switch (op) {
      case TM_MUL: {
        float a=nb.w, b=nb.x, c=nb.y, d=nb.z;
        float mat[3][3] = { /* row major */
            { a*a+b*b-c*c-d*d, 2*b*c-2*a*d    , 2*b*d+2*a*c         },
            { 2*b*c+2*a*d    , a*a-b*b+c*c-d*d, 2*c*d-2*a*b         },
            { 2*b*d-2*a*c    , 2*c*d+2*a*b    , a*a-b*b-c*c+d*d },
        };
        r.x = mat[0][0]*nc.x + mat[0][1]*nc.y + mat[0][2]*nc.z;
        r.y = mat[1][0]*nc.x + mat[1][1]*nc.y + mat[1][2]*nc.z;
        r.z = mat[2][0]*nc.x + mat[2][1]*nc.y + mat[2][2]*nc.z;
      }
      break;
      default: luaG_runerror(L, "Cannot use that op with quat and vector3");
    }
    setv3value(ra, r);

	return;
  }

  if (!luaT_callbinTM(L, p1, p2, res, event)) {
    switch (event) {
      case TM_CONCAT:
        luaG_concaterror(L, p1, p2);
      /* call never returns, but to avoid warnings: *//* FALLTHROUGH */
      case TM_BAND: case TM_BOR: case TM_BXOR:
      case TM_SHL: case TM_SHR: case TM_BNOT: {
        lua_Number dummy;
        if (tonumber(p1, &dummy) && tonumber(p2, &dummy))
          luaG_tointerror(L, p1, p2);
        else
          luaG_opinterror(L, p1, p2, "perform bitwise operation on");
      }
      /* calls never return, but to avoid warnings: *//* FALLTHROUGH */
      default:
        luaG_opinterror(L, p1, p2, "perform arithmetic on");
    }
  }
}


int luaT_callorderTM (lua_State *L, const TValue *p1, const TValue *p2,
                      TMS event) {
  if (!luaT_callbinTM(L, p1, p2, L->top, event))
    return -1;  /* no metamethod */
  else
    return !l_isfalse(L->top);
}

