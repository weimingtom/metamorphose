/*
** $Id: lauxlib.c,v 1.159.1.3 2008/01/21 13:20:51 roberto Exp $
** Auxiliary functions for building Lua libraries
** See Copyright Notice in lua.h
*/


#include <ctype.h>
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


/* This file uses only the official API of Lua.
** Any function declared here could be written as an application function.
*/

#define lauxlib_c
#define LUA_LIB

#include "lua.h"

#include "lauxlib.h"


#define FREELIST_REF	0	/* free list of references */


/* convert a stack index to positive */
#define abs_index(L, i)		((i) > 0 || (i) <= LUA_REGISTRYINDEX ? (i) : \
					lua_gettop(L) + (i) + 1)


/*
** {======================================================
** Error-report functions
** =======================================================
*/

/*
luaL_argerror
[-0, +0, v] 
	int luaL_argerror (lua_State *L, int narg, const char *extramsg);
	Raises an error with the following message, where func is retrieved from the call stack: 
		 bad argument #<narg> to <func> (<extramsg>)
	This function never returns, but it is an idiom to use it in C functions as return luaL_argerror(args). 

luaL_argerror
[-0, +0, v] 
	int luaL_argerror (lua_State *L, int narg, const char *extramsg);
	抛出带下面的消息的错误，其中func是取自调用栈： 
		 bad argument #<narg> to <func> (<extramsg>)
	本函数决不返回，但是像return luaL_argerror(args)这样是用在C函数中的习惯用法。 
*/
LUALIB_API int luaL_argerror (lua_State *L, int narg, const char *extramsg) {
  lua_Debug ar;
  if (!lua_getstack(L, 0, &ar))  /* no stack frame? */
    return luaL_error(L, "bad argument #%d (%s)", narg, extramsg);
  lua_getinfo(L, "n", &ar);
  if (strcmp(ar.namewhat, "method") == 0) {
    narg--;  /* do not count `self' */
    if (narg == 0)  /* error is in the self argument itself? */
      return luaL_error(L, "calling " LUA_QS " on bad self (%s)",
                           ar.name, extramsg);
  }
  if (ar.name == NULL)
    ar.name = "?";
  return luaL_error(L, "bad argument #%d to " LUA_QS " (%s)",
                        narg, ar.name, extramsg);
}

/*
luaL_typerror
[-0, +0, v] 
	int luaL_typerror (lua_State *L, int narg, const char *tname);
	Generates an error with a message like the following: 
		 location: bad argument narg to 'func' (tname expected, got rt)
	where location is produced by luaL_where, func is the name of the current function, and rt is the type name of the actual argument. 

luaL_typerror
[-0, +0, v] 
	int luaL_typerror (lua_State *L, int narg, const char *tname);
	用类似下面的消息产生一个错误： 
		 location: bad argument narg to 'func' (tname expected, got rt)
	其中location由luaL_where产生，func是当前函数名，且rt是实际参数的类型名。 
*/
LUALIB_API int luaL_typerror (lua_State *L, int narg, const char *tname) {
  const char *msg = lua_pushfstring(L, "%s expected, got %s",
                                    tname, luaL_typename(L, narg));
  return luaL_argerror(L, narg, msg);
}


static void tag_error (lua_State *L, int narg, int tag) {
  luaL_typerror(L, narg, lua_typename(L, tag));
}

/*
luaL_where
[-0, +1, m] 
	void luaL_where (lua_State *L, int lvl);
	Pushes onto the stack a string identifying the current position of the control at level lvl in the call stack. Typically this string has the following format: 
		 chunkname:currentline:
	Level 0 is the running function, level 1 is the function that called the running function, etc. 
	This function is used to build a prefix for error messages. 

luaL_where
[-0, +1, m] 
	void luaL_where (lua_State *L, int lvl);
	压入一个标识当前在lvl层调用堆栈的控制位置到堆栈中。
	特别地这个字符串格式如下：
	块名:当前行:
	0层是正在运行的函数，1层是调用运行中函数的函数，如此类推。
	这个函数用于构建错误信息的前缀。
*/
LUALIB_API void luaL_where (lua_State *L, int level) {
  lua_Debug ar;
  if (lua_getstack(L, level, &ar)) {  /* check function at level */
    lua_getinfo(L, "Sl", &ar);  /* get info about it */
    if (ar.currentline > 0) {  /* is there info? */
      lua_pushfstring(L, "%s:%d: ", ar.short_src, ar.currentline);
      return;
    }
  }
  lua_pushliteral(L, "");  /* else, no information available... */
}

/*
luaL_error
[-0, +0, v] 
	int luaL_error (lua_State *L, const char *fmt, ...);
	Raises an error. The error message format is given by fmt plus any extra arguments, following the same rules of lua_pushfstring. It also adds at the beginning of the message the file name and the line number where the error occurred, if this information is available. 
	This function never returns, but it is an idiom to use it in C functions as return luaL_error(args). 

luaL_error
[-0, +0, v] 
	int luaL_error (lua_State *L, const char *fmt, ...);
	引发错误。
	错误信息的格式由fmt指定，外加额外的参数，遵循与lua_pushfstring相同的规则。
	它还会在信息的开头加上文件名和发生错误的行号，如果这类信息可用的话。
	这个函数从不返回，但习惯在C函数内以return luaL_error(args)方式使用。
*/
LUALIB_API int luaL_error (lua_State *L, const char *fmt, ...) {
  va_list argp;
  va_start(argp, fmt);
  luaL_where(L, 1);
  lua_pushvfstring(L, fmt, argp);
  va_end(argp);
  lua_concat(L, 2);
  return lua_error(L);
}

/* }====================================================== */

/*
luaL_checkoption
[-0, +0, v] 
	int luaL_checkoption (lua_State *L,
						  int narg,
						  const char *def,
						  const char *const lst[]);
	Checks whether the function argument narg is a string and searches for this string in the array lst (which must be NULL-terminated). Returns the index in the array where the string was found. Raises an error if the argument is not a string or if the string cannot be found. 
	If def is not NULL, the function uses def as a default value when there is no argument narg or if this argument is nil. 
	This is a useful function for mapping strings to C enums. (The usual convention in Lua libraries is to use strings instead of numbers to select options.) 

luaL_checkoption
[-0, +0, v] 
	int luaL_checkoption (lua_State *L,
						  int narg,
						  const char *def,
						  const char *const lst[]);
	检查函数参数narg是否是一个字符串并且在数组lst中搜索这个字符串（必须是NULL结束）。
	返回数组中所找到字符串的索引。
	如果参数不是一个字符串或字符串找不到则引发错误。
	如果def不是NULL，当没有narg参数或者这个参数为空时，这个函数使用def作为默认值
	这个函数对匹配字符串为C枚举值时很有用。
	（在Lua库中普遍的约定是使用字符串代替数字去选择选项）
*/
LUALIB_API int luaL_checkoption (lua_State *L, int narg, const char *def,
                                 const char *const lst[]) {
  const char *name = (def) ? luaL_optstring(L, narg, def) :
                             luaL_checkstring(L, narg);
  int i;
  for (i=0; lst[i]; i++)
    if (strcmp(lst[i], name) == 0)
      return i;
  return luaL_argerror(L, narg,
                       lua_pushfstring(L, "invalid option " LUA_QS, name));
}

/*
luaL_newmetatable
[-0, +1, m] 
	int luaL_newmetatable (lua_State *L, const char *tname);
	If the registry already has the key tname, returns 0. Otherwise, creates a new table to be used as a metatable for userdata, adds it to the registry with key tname, and returns 1. 
	In both cases pushes onto the stack the final value associated with tname in the registry. 

luaL_newmetatable
[-0, +1, m] 
	int luaL_newmetatable (lua_State *L, const char *tname);
	如果注册表已经有了键tname则返回0。否则，创建将用作用户数据的元表的新表，把它同键tname一起加入注册表，并且返回1。 
	两种情况都把注册表中与tname相关联的最终值压栈。 
*/
LUALIB_API int luaL_newmetatable (lua_State *L, const char *tname) {
  lua_getfield(L, LUA_REGISTRYINDEX, tname);  /* get registry.name */
  if (!lua_isnil(L, -1))  /* name already in use? */
    return 0;  /* leave previous value on top, but return 0 */
  lua_pop(L, 1);
  lua_newtable(L);  /* create metatable */
  lua_pushvalue(L, -1);
  lua_setfield(L, LUA_REGISTRYINDEX, tname);  /* registry.name = metatable */
  return 1;
}

/*
luaL_checkudata
[-0, +0, v] 
	void *luaL_checkudata (lua_State *L, int narg, const char *tname);
	Checks whether the function argument narg is a userdata of the type tname (see luaL_newmetatable). 

luaL_checkudata
[-0, +0, v] 
	void *luaL_checkudata (lua_State *L, int narg, const char *tname);
	检查函数参数narg是否是类型tname的用户定义类型（参考luaL_newmetatable）
*/
LUALIB_API void *luaL_checkudata (lua_State *L, int ud, const char *tname) {
  void *p = lua_touserdata(L, ud);
  if (p != NULL) {  /* value is a userdata? */
    if (lua_getmetatable(L, ud)) {  /* does it have a metatable? */
      lua_getfield(L, LUA_REGISTRYINDEX, tname);  /* get correct metatable */
      if (lua_rawequal(L, -1, -2)) {  /* does it have the correct mt? */
        lua_pop(L, 2);  /* remove both metatables */
        return p;
      }
    }
  }
  luaL_typerror(L, ud, tname);  /* else error */
  return NULL;  /* to avoid warnings */
}

/*
luaL_checkstack
[-0, +0, v] 
	void luaL_checkstack (lua_State *L, int sz, const char *msg);
	Grows the stack size to top + sz elements, raising an error if the stack cannot grow to that size. msg is an additional text to go into the error message. 

luaL_checkstack
[-0, +0, v] 
	void luaL_checkstack (lua_State *L, int sz, const char *msg);
	增长栈尺寸到top + sz个元素，如果不能增长到那个尺寸则引发错误。msg是加入错误消息的补充文本。 
*/
LUALIB_API void luaL_checkstack (lua_State *L, int space, const char *mes) {
  if (!lua_checkstack(L, space))
    luaL_error(L, "stack overflow (%s)", mes);
}

/*
luaL_checktype
[-0, +0, v] 
	void luaL_checktype (lua_State *L, int narg, int t);
	Checks whether the function argument narg has type t. See lua_type for the encoding of types for t. 

luaL_checktype
[-0, +0, v] 
	void luaL_checktype (lua_State *L, int narg, int t);
	检查函数参数narg是否拥有类型t。
	参考lua_type获得t的类型编码。
*/
LUALIB_API void luaL_checktype (lua_State *L, int narg, int t) {
  if (lua_type(L, narg) != t)
    tag_error(L, narg, t);
}

/*
luaL_checkany
[-0, +0, v] 
	void luaL_checkany (lua_State *L, int narg);
	Checks whether the function has an argument of any type (including nil) at position narg. 

luaL_checkany
[-0, +0, v] 
	void luaL_checkany (lua_State *L, int narg);
	检查函数是否在位置narg处有个任意类型（包括nil）的参数。 
*/
LUALIB_API void luaL_checkany (lua_State *L, int narg) {
  if (lua_type(L, narg) == LUA_TNONE)
    luaL_argerror(L, narg, "value expected");
}

/*
luaL_checklstring
[-0, +0, v] 
	const char *luaL_checklstring (lua_State *L, int narg, size_t *l);
	Checks whether the function argument narg is a string and returns this string; if l is not NULL fills *l with the string's length. 
	This function uses lua_tolstring to get its result, so all conversions and caveats of that function apply here. 

luaL_checklstring
[-0, +0, v] 
	const char *luaL_checklstring (lua_State *L, int narg, size_t *l);
	检查函数参数narg是否是一个字符串，然后返回这个字符串；如果l不是NULL则填充*l为字符串的长度。
	这个函数使用lua_tolstring获得结果，所以那个函数的所有转换和注意事项都适用于此。
*/
LUALIB_API const char *luaL_checklstring (lua_State *L, int narg, size_t *len) {
  const char *s = lua_tolstring(L, narg, len);
  if (!s) tag_error(L, narg, LUA_TSTRING);
  return s;
}

/*
luaL_optlstring
[-0, +0, v] 
	const char *luaL_optlstring (lua_State *L,
								 int narg,
								 const char *d,
								 size_t *l);
	If the function argument narg is a string, returns this string. If this argument is absent or is nil, returns d. Otherwise, raises an error. 
	If l is not NULL, fills the position *l with the results's length. 

luaL_optlstring
[-0, +0, v] 
	const char *luaL_optlstring (lua_State *L,
								 int narg,
								 const char *d,
								 size_t *l);
	如果函数参数narg是字符串则返回它。如果该参数不存在或为nil则返回d。否则引发错误。 
	如果l不为NULL，则用结果的长度填充位置*l。 
*/
LUALIB_API const char *luaL_optlstring (lua_State *L, int narg,
                                        const char *def, size_t *len) {
  if (lua_isnoneornil(L, narg)) {
    if (len)
      *len = (def ? strlen(def) : 0);
    return def;
  }
  else return luaL_checklstring(L, narg, len);
}

/*
luaL_checknumber
[-0, +0, v] 
	lua_Number luaL_checknumber (lua_State *L, int narg);
	Checks whether the function argument narg is a number and returns this number. 

luaL_checknumber
[-0, +0, v] 
	lua_Number luaL_checknumber (lua_State *L, int narg);
	检查函数参数narg是否是一个数，并且返回这个数。
*/
LUALIB_API lua_Number luaL_checknumber (lua_State *L, int narg) {
  lua_Number d = lua_tonumber(L, narg);
  if (d == 0 && !lua_isnumber(L, narg))  /* avoid extra test when d is not 0 */
    tag_error(L, narg, LUA_TNUMBER);
  return d;
}

/*
luaL_optnumber
[-0, +0, v] 
	lua_Number luaL_optnumber (lua_State *L, int narg, lua_Number d);
	If the function argument narg is a number, returns this number. If this argument is absent or is nil, returns d. Otherwise, raises an error. 

luaL_optnumber
[-0, +0, v] 
	lua_Number luaL_optnumber (lua_State *L, int narg, lua_Number d);
	如果函数参数narg是数字则返回它。如果该参数不存在或为nil则返回d。否则引发错误。 
*/
LUALIB_API lua_Number luaL_optnumber (lua_State *L, int narg, lua_Number def) {
  return luaL_opt(L, luaL_checknumber, narg, def);
}

/*
luaL_checkinteger
[-0, +0, v] 
	lua_Integer luaL_checkinteger (lua_State *L, int narg);
	Checks whether the function argument narg is a number and returns this number cast to a lua_Integer. 

luaL_checkinteger
[-0, +0, v] 
	lua_Integer luaL_checkinteger (lua_State *L, int narg);
	检查函数参数narg是否是一个数，并且把这个数转换为lua_Integer然后返回。
*/
LUALIB_API lua_Integer luaL_checkinteger (lua_State *L, int narg) {
  lua_Integer d = lua_tointeger(L, narg);
  if (d == 0 && !lua_isnumber(L, narg))  /* avoid extra test when d is not 0 */
    tag_error(L, narg, LUA_TNUMBER);
  return d;
}

/*
luaL_optinteger
[-0, +0, v] 
	lua_Integer luaL_optinteger (lua_State *L,
								 int narg,
								 lua_Integer d);
	If the function argument narg is a number, returns this number cast to a lua_Integer. If this argument is absent or is nil, returns d. Otherwise, raises an error. 

luaL_optinteger
[-0, +0, v] 
	lua_Integer luaL_optinteger (lua_State *L,
								 int narg,
								 lua_Integer d);
	如果函数参数narg是数字，则把该数字转型为lua_Integer返回。如果该参数不存在或为nil则返回d。否则引发错误。 
*/
LUALIB_API lua_Integer luaL_optinteger (lua_State *L, int narg,
                                                      lua_Integer def) {
  return luaL_opt(L, luaL_checkinteger, narg, def);
}

/*
luaL_getmetafield
[-0, +(0|1), m] 
	int luaL_getmetafield (lua_State *L, int obj, const char *e);
	Pushes onto the stack the field e from the metatable of the object at index obj. If the object does not have a metatable, or if the metatable does not have this field, returns 0 and pushes nothing. 

luaL_getmetafield
[-0, +(0|1), m] 
	int luaL_getmetafield (lua_State *L, int obj, const char *e);
	把来自索引obj处的对象的元表的字段e压栈。如果对象没有元表或其元表没有该字段，则返回0且不会压栈任何东西。 
*/
LUALIB_API int luaL_getmetafield (lua_State *L, int obj, const char *event) {
  if (!lua_getmetatable(L, obj))  /* no metatable? */
    return 0;
  lua_pushstring(L, event);
  lua_rawget(L, -2);
  if (lua_isnil(L, -1)) {
    lua_pop(L, 2);  /* remove metatable and metafield */
    return 0;
  }
  else {
    lua_remove(L, -2);  /* remove only metatable */
    return 1;
  }
}

/*
luaL_callmeta
[-0, +(0|1), e] 
	int luaL_callmeta (lua_State *L, int obj, const char *e);
	Calls a metamethod. 
	If the object at index obj has a metatable and this metatable has a field e, this function calls this field and passes the object as its only argument. In this case this function returns 1 and pushes onto the stack the value returned by the call. If there is no metatable or no metamethod, this function returns 0 (without pushing any value on the stack). 

luaL_callmeta
[-0, +(0|1), e] 
	int luaL_callmeta (lua_State *L, int obj, const char *e);
	调用一个元方法。 
	如果索引obj处的对象具有元表且该元表具有字段e，本函数调用该字段并传入该对象为其唯一参数。这种情况下，本函数返回1并将该调用返回的值压栈。如果没有元表或没有元方法，本函数返回0（不将任何值压栈）。 
*/
LUALIB_API int luaL_callmeta (lua_State *L, int obj, const char *event) {
  obj = abs_index(L, obj);
  if (!luaL_getmetafield(L, obj, event))  /* no metafield? */
    return 0;
  lua_pushvalue(L, obj);
  lua_call(L, 1, 1);
  return 1;
}

/*
luaL_register
[-(0|1), +1, m] 
	void luaL_register (lua_State *L,
						const char *libname,
						const luaL_Reg *l);
	Opens a library. 
	When called with libname equal to NULL, it simply registers all functions in the list l (see luaL_Reg) into the table on the top of the stack. 
	When called with a non-null libname, luaL_register creates a new table t, sets it as the value of the global variable libname, sets it as the value of package.loaded[libname], and registers on it all functions in the list l. If there is a table in package.loaded[libname] or in variable libname, reuses this table instead of creating a new one. 
	In any case the function leaves the table on the top of the stack. 

luaL_register
[-(0|1), +1, m] 
	void luaL_register (lua_State *L,
						const char *libname,
						const luaL_Reg *l);
	打开一个库。 
	当以libname等于NULL调用时，它只是注册列表l中的所有函数（见luaL_Reg）到栈顶的表中。 
	当以非空的libname调用时，luaL_register创建新表t，把它设为全局变量libname的值，和package.loaded[libname]的值，并把列表l中的所有函数注册到该表。如果package.loaded[libname]中或变量libname中有个表，则重用该表而不是创建一个新的。 
	无论如何函数都把表留在栈顶。 
*/
LUALIB_API void (luaL_register) (lua_State *L, const char *libname,
                                const luaL_Reg *l) {
  luaI_openlib(L, libname, l, 0);
}


static int libsize (const luaL_Reg *l) {
  int size = 0;
  for (; l->name; l++) size++;
  return size;
}


LUALIB_API void luaI_openlib (lua_State *L, const char *libname,
                              const luaL_Reg *l, int nup) {
  if (libname) {
    int size = libsize(l);
    /* check whether lib already exists */
    luaL_findtable(L, LUA_REGISTRYINDEX, "_LOADED", 1);
    lua_getfield(L, -1, libname);  /* get _LOADED[libname] */
    if (!lua_istable(L, -1)) {  /* not found? */
      lua_pop(L, 1);  /* remove previous result */
      /* try global variable (and create one if it does not exist) */
      if (luaL_findtable(L, LUA_GLOBALSINDEX, libname, size) != NULL)
        luaL_error(L, "name conflict for module " LUA_QS, libname);
      lua_pushvalue(L, -1);
      lua_setfield(L, -3, libname);  /* _LOADED[libname] = new table */
    }
    lua_remove(L, -2);  /* remove _LOADED table */
    lua_insert(L, -(nup+1));  /* move library table to below upvalues */
  }
  for (; l->name; l++) {
    int i;
    for (i=0; i<nup; i++)  /* copy upvalues to the top */
      lua_pushvalue(L, -nup);
    lua_pushcclosure(L, l->func, nup);
    lua_setfield(L, -(nup+2), l->name);
  }
  lua_pop(L, nup);  /* remove upvalues */
}



/*
** {======================================================
** getn-setn: size for arrays
** =======================================================
*/

#if defined(LUA_COMPAT_GETN)

static int checkint (lua_State *L, int topop) {
  int n = (lua_type(L, -1) == LUA_TNUMBER) ? lua_tointeger(L, -1) : -1;
  lua_pop(L, topop);
  return n;
}


static void getsizes (lua_State *L) {
  lua_getfield(L, LUA_REGISTRYINDEX, "LUA_SIZES");
  if (lua_isnil(L, -1)) {  /* no `size' table? */
    lua_pop(L, 1);  /* remove nil */
    lua_newtable(L);  /* create it */
    lua_pushvalue(L, -1);  /* `size' will be its own metatable */
    lua_setmetatable(L, -2);
    lua_pushliteral(L, "kv");
    lua_setfield(L, -2, "__mode");  /* metatable(N).__mode = "kv" */
    lua_pushvalue(L, -1);
    lua_setfield(L, LUA_REGISTRYINDEX, "LUA_SIZES");  /* store in register */
  }
}


LUALIB_API void luaL_setn (lua_State *L, int t, int n) {
  t = abs_index(L, t);
  lua_pushliteral(L, "n");
  lua_rawget(L, t);
  if (checkint(L, 1) >= 0) {  /* is there a numeric field `n'? */
    lua_pushliteral(L, "n");  /* use it */
    lua_pushinteger(L, n);
    lua_rawset(L, t);
  }
  else {  /* use `sizes' */
    getsizes(L);
    lua_pushvalue(L, t);
    lua_pushinteger(L, n);
    lua_rawset(L, -3);  /* sizes[t] = n */
    lua_pop(L, 1);  /* remove `sizes' */
  }
}


LUALIB_API int luaL_getn (lua_State *L, int t) {
  int n;
  t = abs_index(L, t);
  lua_pushliteral(L, "n");  /* try t.n */
  lua_rawget(L, t);
  if ((n = checkint(L, 1)) >= 0) return n;
  getsizes(L);  /* else try sizes[t] */
  lua_pushvalue(L, t);
  lua_rawget(L, -2);
  if ((n = checkint(L, 2)) >= 0) return n;
  return (int)lua_objlen(L, t);
}

#endif

/* }====================================================== */


/*
luaL_gsub
[-0, +1, m] 
const char *luaL_gsub (lua_State *L,
                       const char *s,
                       const char *p,
                       const char *r);
Creates a copy of string s by replacing any occurrence of the string p with the string r. Pushes the resulting string on the stack and returns it. 

luaL_gsub
[-0, +1, m] 
const char *luaL_gsub (lua_State *L,
                       const char *s,
                       const char *p,
                       const char *r);
通过把出现的任何字符串p替换为字符串r来创建字符串s的拷贝。把结果字符串压栈并返回它。 
*/
LUALIB_API const char *luaL_gsub (lua_State *L, const char *s, const char *p,
                                                               const char *r) {
  const char *wild;
  size_t l = strlen(p);
  luaL_Buffer b;
  luaL_buffinit(L, &b);
  while ((wild = strstr(s, p)) != NULL) {
    luaL_addlstring(&b, s, wild - s);  /* push prefix */
    luaL_addstring(&b, r);  /* push replacement in place of pattern */
    s = wild + l;  /* continue after `p' */
  }
  luaL_addstring(&b, s);  /* push last suffix */
  luaL_pushresult(&b);
  return lua_tostring(L, -1);
}


LUALIB_API const char *luaL_findtable (lua_State *L, int idx,
                                       const char *fname, int szhint) {
  const char *e;
  lua_pushvalue(L, idx);
  do {
    e = strchr(fname, '.');
    if (e == NULL) e = fname + strlen(fname);
    lua_pushlstring(L, fname, e - fname);
    lua_rawget(L, -2);
    if (lua_isnil(L, -1)) {  /* no such field? */
      lua_pop(L, 1);  /* remove this nil */
      lua_createtable(L, 0, (*e == '.' ? 1 : szhint)); /* new table for field */
      lua_pushlstring(L, fname, e - fname);
      lua_pushvalue(L, -2);
      lua_settable(L, -4);  /* set new table into field */
    }
    else if (!lua_istable(L, -1)) {  /* field has a non-table value? */
      lua_pop(L, 2);  /* remove table and value */
      return fname;  /* return problematic part of the name */
    }
    lua_remove(L, -2);  /* remove previous table */
    fname = e + 1;
  } while (*e == '.');
  return NULL;
}



/*
** {======================================================
** Generic Buffer manipulation
** =======================================================
*/


#define bufflen(B)	((B)->p - (B)->buffer)
#define bufffree(B)	((size_t)(LUAL_BUFFERSIZE - bufflen(B)))

#define LIMIT	(LUA_MINSTACK/2)


static int emptybuffer (luaL_Buffer *B) {
  size_t l = bufflen(B);
  if (l == 0) return 0;  /* put nothing on stack */
  else {
    lua_pushlstring(B->L, B->buffer, l);
    B->p = B->buffer;
    B->lvl++;
    return 1;
  }
}


static void adjuststack (luaL_Buffer *B) {
  if (B->lvl > 1) {
    lua_State *L = B->L;
    int toget = 1;  /* number of levels to concat */
    size_t toplen = lua_strlen(L, -1);
    do {
      size_t l = lua_strlen(L, -(toget+1));
      if (B->lvl - toget + 1 >= LIMIT || toplen > l) {
        toplen += l;
        toget++;
      }
      else break;
    } while (toget < B->lvl);
    lua_concat(L, toget);
    B->lvl = B->lvl - toget + 1;
  }
}

/*
luaL_prepbuffer
[-0, +0, -] 
	char *luaL_prepbuffer (luaL_Buffer *B);
	Returns an address to a space of size LUAL_BUFFERSIZE where you can copy a string to be added to buffer B (see luaL_Buffer). After copying the string into this space you must call luaL_addsize with the size of the string to actually add it to the buffer. 

luaL_prepbuffer
[-0, +0, -] 
	char *luaL_prepbuffer (luaL_Buffer *B);
	返回尺寸为LUAL_BUFFERSIZE的空间的地址，你能把要被加入缓冲器B的字符串拷贝到其中（见luaL_Buffer）。在把字符串拷贝到该空间中以后，你必须用字符串的尺寸调用luaL_addsize来把它加入缓冲器中。 
*/
LUALIB_API char *luaL_prepbuffer (luaL_Buffer *B) {
  if (emptybuffer(B))
    adjuststack(B);
  return B->buffer;
}

/*
luaL_addlstring
[-0, +0, m] 
	void luaL_addlstring (luaL_Buffer *B, const char *s, size_t l);
	Adds the string pointed to by s with length l to the buffer B (see luaL_Buffer). The string may contain embedded zeros. 

luaL_addlstring
[-0, +0, m] 
	void luaL_addlstring (luaL_Buffer *B, const char *s, size_t l);
	把指向s的长度l的字符串加入缓冲区B（参考luaL_Buffer）。
	这个字符串可以包含嵌入的0。
*/
LUALIB_API void luaL_addlstring (luaL_Buffer *B, const char *s, size_t l) {
  while (l--)
    luaL_addchar(B, *s++);
}

/*
luaL_addstring
[-0, +0, m] 
	void luaL_addstring (luaL_Buffer *B, const char *s);
	Adds the zero-terminated string pointed to by s to the buffer B (see luaL_Buffer). The string may not contain embedded zeros. 

luaL_addstring
[-0, +0, m] 
	void luaL_addstring (luaL_Buffer *B, const char *s);
	把s指向的0结尾的字符串添加到缓冲器B（见luaL_Buffer）。字符串不可包含内嵌的0。 
*/
LUALIB_API void luaL_addstring (luaL_Buffer *B, const char *s) {
  luaL_addlstring(B, s, strlen(s));
}

/*
luaL_pushresult
[-?, +1, m] 
	void luaL_pushresult (luaL_Buffer *B);
	Finishes the use of buffer B leaving the final string on the top of the stack. 

luaL_pushresult
[-?, +1, m] 
	void luaL_pushresult (luaL_Buffer *B);
	结束对缓冲器B的使用，把最终字符串留在栈顶。 
*/
LUALIB_API void luaL_pushresult (luaL_Buffer *B) {
  emptybuffer(B);
  lua_concat(B->L, B->lvl);
  B->lvl = 1;
}

/*
luaL_addvalue
[-1, +0, m] 
	void luaL_addvalue (luaL_Buffer *B);
	Adds the value at the top of the stack to the buffer B (see luaL_Buffer). Pops the value. 
	This is the only function on string buffers that can (and must) be called with an extra element on the stack, which is the value to be added to the buffer. 

luaL_addvalue
[-1, +0, m] 
	void luaL_addvalue (luaL_Buffer *B);
	把栈顶的值添加到缓冲器B（见luaL_Buffer）。弹出该值。 
	这是仅有能（且必须）用栈上的一个额外元素调用的关于字符串缓冲器的函数，该元素是要被添加到缓冲器的值。 
*/
LUALIB_API void luaL_addvalue (luaL_Buffer *B) {
  lua_State *L = B->L;
  size_t vl;
  const char *s = lua_tolstring(L, -1, &vl);
  if (vl <= bufffree(B)) {  /* fit into buffer? */
    memcpy(B->p, s, vl);  /* put it there */
    B->p += vl;
    lua_pop(L, 1);  /* remove from stack */
  }
  else {
    if (emptybuffer(B))
      lua_insert(L, -2);  /* put buffer before new value */
    B->lvl++;  /* add new value into B stack */
    adjuststack(B);
  }
}

/*
luaL_buffinit
[-0, +0, -] 
	void luaL_buffinit (lua_State *L, luaL_Buffer *B);
	Initializes a buffer B. This function does not allocate any space; the buffer must be declared as a variable (see luaL_Buffer). 

luaL_buffinit
[-0, +0, -] 
	void luaL_buffinit (lua_State *L, luaL_Buffer *B);
	初始化缓冲器B。本函数不分配任何空间；缓冲器必须已被声明为变量（见luaL_Buffer）。 
*/
LUALIB_API void luaL_buffinit (lua_State *L, luaL_Buffer *B) {
  B->L = L;
  B->p = B->buffer;
  B->lvl = 0;
}

/* }====================================================== */

/*
luaL_ref
[-1, +0, m] 
	int luaL_ref (lua_State *L, int t);
	Creates and returns a reference, in the table at index t, for the object at the top of the stack (and pops the object). 
	A reference is a unique integer key. As long as you do not manually add integer keys into table t, luaL_ref ensures the uniqueness of the key it returns. You can retrieve an object referred by reference r by calling lua_rawgeti(L, t, r). Function luaL_unref frees a reference and its associated object. 
	If the object at the top of the stack is nil, luaL_ref returns the constant LUA_REFNIL. The constant LUA_NOREF is guaranteed to be different from any reference returned by luaL_ref. 

luaL_ref
[-1, +0, m] 
	int luaL_ref (lua_State *L, int t);
	在索引t处的表中为栈顶的对象创建一个引用（reference）并返回（而且弹出该对象）。 
	引用是唯一的整数键。只要你不手工向表t中加入整数键，luaL_ref保证它返回的键的唯一性。你可通过调用lua_rawgeti(L, t, r)取回被r引用的对象。函数luaL_unref释放引用及其关联的对象。 
	如果栈顶的对象是nil，luaL_ref返回常量LUA_REFNIL。常量LUA_NOREF被确保与luaL_ref返回的任何引用都不同。 
*/
LUALIB_API int luaL_ref (lua_State *L, int t) {
  int ref;
  t = abs_index(L, t);
  if (lua_isnil(L, -1)) {
    lua_pop(L, 1);  /* remove from stack */
    return LUA_REFNIL;  /* `nil' has a unique fixed reference */
  }
  lua_rawgeti(L, t, FREELIST_REF);  /* get first free element */
  ref = (int)lua_tointeger(L, -1);  /* ref = t[FREELIST_REF] */
  lua_pop(L, 1);  /* remove it from stack */
  if (ref != 0) {  /* any free element? */
    lua_rawgeti(L, t, ref);  /* remove it from list */
    lua_rawseti(L, t, FREELIST_REF);  /* (t[FREELIST_REF] = t[ref]) */
  }
  else {  /* no free elements */
    ref = (int)lua_objlen(L, t);
    ref++;  /* create new reference */
  }
  lua_rawseti(L, t, ref);
  return ref;
}

/*
luaL_unref
[-0, +0, -] 
	void luaL_unref (lua_State *L, int t, int ref);
	Releases reference ref from the table at index t (see luaL_ref). The entry is removed from the table, so that the referred object can be collected. The reference ref is also freed to be used again. 
	If ref is LUA_NOREF or LUA_REFNIL, luaL_unref does nothing. 

luaL_unref
[-0, +0, -] 
	void luaL_unref (lua_State *L, int t, int ref);
	解除来自索引t处的表的引用ref（见luaL_ref）。该项从表中删除，所以被引用的对象可被回收。引用ref也被释放以备再次使用。 
	如果ref是LUA_NOREF或LUA_REFNIL，luaL_unref什么也不做。 
*/
LUALIB_API void luaL_unref (lua_State *L, int t, int ref) {
  if (ref >= 0) {
    t = abs_index(L, t);
    lua_rawgeti(L, t, FREELIST_REF);
    lua_rawseti(L, t, ref);  /* t[ref] = t[FREELIST_REF] */
    lua_pushinteger(L, ref);
    lua_rawseti(L, t, FREELIST_REF);  /* t[FREELIST_REF] = ref */
  }
}



/*
** {======================================================
** Load functions
** =======================================================
*/

typedef struct LoadF {
  int extraline;
  FILE *f;
  char buff[LUAL_BUFFERSIZE];
} LoadF;


static const char *getF (lua_State *L, void *ud, size_t *size) {
  LoadF *lf = (LoadF *)ud;
  (void)L;
  if (lf->extraline) {
    lf->extraline = 0;
    *size = 1;
    return "\n";
  }
  if (feof(lf->f)) return NULL;
  *size = fread(lf->buff, 1, sizeof(lf->buff), lf->f);
  return (*size > 0) ? lf->buff : NULL;
}


static int errfile (lua_State *L, const char *what, int fnameindex) {
  const char *serr = strerror(errno);
  const char *filename = lua_tostring(L, fnameindex) + 1;
  lua_pushfstring(L, "cannot %s %s: %s", what, filename, serr);
  lua_remove(L, fnameindex);
  return LUA_ERRFILE;
}

/*
luaL_loadfile
[-0, +1, m] 
	int luaL_loadfile (lua_State *L, const char *filename);
	Loads a file as a Lua chunk. This function uses lua_load to load the chunk in the file named filename. If filename is NULL, then it loads from the standard input. The first line in the file is ignored if it starts with a #. 
	This function returns the same results as lua_load, but it has an extra error code LUA_ERRFILE if it cannot open/read the file. 
	As lua_load, this function only loads the chunk; it does not run it. 

luaL_loadfile
[-0, +1, m] 
	int luaL_loadfile (lua_State *L, const char *filename);
	载入文件作为一个Lua单元。本函数用lua_load来载入名为filename的文件中的单元。如果filename是NULL，则从标准输入载入。文件中的第一行如果以#开头则被忽略。 
	本函数返回同lua_load一样的结果，除了有个额外的错误代码LUA_ERRFILE，用于不能打开/读取文件的情况。 
	同lua_load一样，本函数只载入单元；不会运行它。 
*/
LUALIB_API int luaL_loadfile (lua_State *L, const char *filename) {
  LoadF lf;
  int status, readstatus;
  int c;
  int fnameindex = lua_gettop(L) + 1;  /* index of filename on the stack */
  lf.extraline = 0;
  if (filename == NULL) {
    lua_pushliteral(L, "=stdin");
    lf.f = stdin;
  }
  else {
    lua_pushfstring(L, "@%s", filename);
    lf.f = fopen(filename, "r");
    if (lf.f == NULL) return errfile(L, "open", fnameindex);
  }
  c = getc(lf.f);
  if (c == '#') {  /* Unix exec. file? */
    lf.extraline = 1;
    while ((c = getc(lf.f)) != EOF && c != '\n') ;  /* skip first line */
    if (c == '\n') c = getc(lf.f);
  }
  if (c == LUA_SIGNATURE[0] && filename) {  /* binary file? */
    lf.f = freopen(filename, "rb", lf.f);  /* reopen in binary mode */
    if (lf.f == NULL) return errfile(L, "reopen", fnameindex);
    /* skip eventual `#!...' */
   while ((c = getc(lf.f)) != EOF && c != LUA_SIGNATURE[0]) ;
    lf.extraline = 0;
  }
  ungetc(c, lf.f);
  status = lua_load(L, getF, &lf, lua_tostring(L, -1));
  readstatus = ferror(lf.f);
  if (filename) fclose(lf.f);  /* close file (even in case of errors) */
  if (readstatus) {
    lua_settop(L, fnameindex);  /* ignore results from `lua_load' */
    return errfile(L, "read", fnameindex);
  }
  lua_remove(L, fnameindex);
  return status;
}


typedef struct LoadS {
  const char *s;
  size_t size;
} LoadS;


static const char *getS (lua_State *L, void *ud, size_t *size) {
  LoadS *ls = (LoadS *)ud;
  (void)L;
  if (ls->size == 0) return NULL;
  *size = ls->size;
  ls->size = 0;
  return ls->s;
}

/*
luaL_loadbuffer
[-0, +1, m] 
	int luaL_loadbuffer (lua_State *L,
						 const char *buff,
						 size_t sz,
						 const char *name);
	Loads a buffer as a Lua chunk. This function uses lua_load to load the chunk in the buffer pointed to by buff with size sz. 
	This function returns the same results as lua_load. name is the chunk name, used for debug information and error messages. 

luaL_loadbuffer
[-0, +1, m] 
	载入缓冲器并作为一个Lua单元。本函数用lua_load来加载缓冲器中由buff指向且长度为sz的单元。 
	本函数返回同lua_load一样的结果。name是单元名字，用于调试信息和错误消息。 
*/
LUALIB_API int luaL_loadbuffer (lua_State *L, const char *buff, size_t size,
                                const char *name) {
  LoadS ls;
  ls.s = buff;
  ls.size = size;
  return lua_load(L, getS, &ls, name);
}

/*
luaL_loadstring
[-0, +1, m] 
	int luaL_loadstring (lua_State *L, const char *s);
	Loads a string as a Lua chunk. This function uses lua_load to load the chunk in the zero-terminated string s. 
	This function returns the same results as lua_load. 
	Also as lua_load, this function only loads the chunk; it does not run it. 

luaL_loadstring
[-0, +1, m] 
	int luaL_loadstring (lua_State *L, const char *s);
	载入字符串作为一个Lua单元。本函数用lua_load来载入以0结尾的字符串s中的单元。 
	本函数返回同lua_load一样的结果。 
	本函数只载入单元，这也同lua_load一样；不会运行它。 
*/
LUALIB_API int (luaL_loadstring) (lua_State *L, const char *s) {
  return luaL_loadbuffer(L, s, strlen(s), s);
}



/* }====================================================== */


static void *l_alloc (void *ud, void *ptr, size_t osize, size_t nsize) {
  (void)ud;
  (void)osize;
  if (nsize == 0) {
    free(ptr);
    return NULL;
  }
  else
    return realloc(ptr, nsize);
}


static int panic (lua_State *L) {
  (void)L;  /* to avoid warnings */
  fprintf(stderr, "PANIC: unprotected error in call to Lua API (%s)\n",
                   lua_tostring(L, -1));
  return 0;
}

/*
luaL_newstate
[-0, +0, -] 
	lua_State *luaL_newstate (void);
	Creates a new Lua state. It calls lua_newstate with an allocator based on the standard C realloc function and then sets a panic function (see lua_atpanic) that prints an error message to the standard error output in case of fatal errors. 
	Returns the new state, or NULL if there is a memory allocation error. 

luaL_newstate
[-0, +0, -] 
	lua_State *luaL_newstate (void);
	创建新的Lua状态机。它用基于标准C的realloc函数的分配器调用lua_newstate，然后设置一个在发生重大错误时向标准错误输出打印一条错误消息的应急函数（见lua_atpanic）。 
	返回新的状态机，如果发生内存分配错误则返回NULL。 
*/
LUALIB_API lua_State *luaL_newstate (void) {
  lua_State *L = lua_newstate(l_alloc, NULL);
  if (L) lua_atpanic(L, &panic);
  return L;
}

