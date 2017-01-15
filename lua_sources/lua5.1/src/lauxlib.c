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
	�׳����������Ϣ�Ĵ�������func��ȡ�Ե���ջ�� 
		 bad argument #<narg> to <func> (<extramsg>)
	�������������أ�������return luaL_argerror(args)����������C�����е�ϰ���÷��� 
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
	�������������Ϣ����һ������ 
		 location: bad argument narg to 'func' (tname expected, got rt)
	����location��luaL_where������func�ǵ�ǰ����������rt��ʵ�ʲ������������� 
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
	ѹ��һ����ʶ��ǰ��lvl����ö�ջ�Ŀ���λ�õ���ջ�С�
	�ر������ַ�����ʽ���£�
	����:��ǰ��:
	0�����������еĺ�����1���ǵ��������к����ĺ�����������ơ�
	����������ڹ���������Ϣ��ǰ׺��
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
	��������
	������Ϣ�ĸ�ʽ��fmtָ������Ӷ���Ĳ�������ѭ��lua_pushfstring��ͬ�Ĺ���
	����������Ϣ�Ŀ�ͷ�����ļ����ͷ���������кţ����������Ϣ���õĻ���
	��������Ӳ����أ���ϰ����C��������return luaL_error(args)��ʽʹ�á�
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
	��麯������narg�Ƿ���һ���ַ�������������lst����������ַ�����������NULL��������
	�������������ҵ��ַ�����������
	�����������һ���ַ������ַ����Ҳ�������������
	���def����NULL����û��narg���������������Ϊ��ʱ���������ʹ��def��ΪĬ��ֵ
	���������ƥ���ַ���ΪCö��ֵʱ�����á�
	����Lua�����ձ��Լ����ʹ���ַ�����������ȥѡ��ѡ�
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
	���ע����Ѿ����˼�tname�򷵻�0�����򣬴����������û����ݵ�Ԫ����±�����ͬ��tnameһ�����ע������ҷ���1�� 
	�����������ע�������tname�����������ֵѹջ�� 
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
	��麯������narg�Ƿ�������tname���û��������ͣ��ο�luaL_newmetatable��
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
	����ջ�ߴ絽top + sz��Ԫ�أ���������������Ǹ��ߴ�����������msg�Ǽ��������Ϣ�Ĳ����ı��� 
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
	��麯������narg�Ƿ�ӵ������t��
	�ο�lua_type���t�����ͱ��롣
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
	��麯���Ƿ���λ��narg���и��������ͣ�����nil���Ĳ����� 
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
	��麯������narg�Ƿ���һ���ַ�����Ȼ�󷵻�����ַ��������l����NULL�����*lΪ�ַ����ĳ��ȡ�
	�������ʹ��lua_tolstring��ý���������Ǹ�����������ת����ע����������ڴˡ�
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
	�����������narg���ַ����򷵻���������ò��������ڻ�Ϊnil�򷵻�d�������������� 
	���l��ΪNULL�����ý���ĳ������λ��*l�� 
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
	��麯������narg�Ƿ���һ���������ҷ����������
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
	�����������narg�������򷵻���������ò��������ڻ�Ϊnil�򷵻�d�������������� 
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
	��麯������narg�Ƿ���һ���������Ұ������ת��Ϊlua_IntegerȻ�󷵻ء�
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
	�����������narg�����֣���Ѹ�����ת��Ϊlua_Integer���ء�����ò��������ڻ�Ϊnil�򷵻�d�������������� 
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
	����������obj���Ķ����Ԫ����ֶ�eѹջ���������û��Ԫ�����Ԫ��û�и��ֶΣ��򷵻�0�Ҳ���ѹջ�κζ����� 
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
	����һ��Ԫ������ 
	�������obj���Ķ������Ԫ���Ҹ�Ԫ������ֶ�e�����������ø��ֶβ�����ö���Ϊ��Ψһ��������������£�����������1�����õ��÷��ص�ֵѹջ�����û��Ԫ���û��Ԫ����������������0�������κ�ֵѹջ���� 
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
	��һ���⡣ 
	����libname����NULL����ʱ����ֻ��ע���б�l�е����к�������luaL_Reg����ջ���ı��С� 
	���Էǿյ�libname����ʱ��luaL_register�����±�t��������Ϊȫ�ֱ���libname��ֵ����package.loaded[libname]��ֵ�������б�l�е����к���ע�ᵽ�ñ����package.loaded[libname]�л����libname���и��������øñ�����Ǵ���һ���µġ� 
	������κ������ѱ�����ջ���� 
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
ͨ���ѳ��ֵ��κ��ַ���p�滻Ϊ�ַ���r�������ַ���s�Ŀ������ѽ���ַ���ѹջ���������� 
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
	���سߴ�ΪLUAL_BUFFERSIZE�Ŀռ�ĵ�ַ�����ܰ�Ҫ�����뻺����B���ַ������������У���luaL_Buffer�����ڰ��ַ����������ÿռ����Ժ���������ַ����ĳߴ����luaL_addsize���������뻺�����С� 
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
	��ָ��s�ĳ���l���ַ������뻺����B���ο�luaL_Buffer����
	����ַ������԰���Ƕ���0��
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
	��sָ���0��β���ַ�����ӵ�������B����luaL_Buffer�����ַ������ɰ�����Ƕ��0�� 
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
	�����Ի�����B��ʹ�ã��������ַ�������ջ���� 
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
	��ջ����ֵ��ӵ�������B����luaL_Buffer����������ֵ�� 
	���ǽ����ܣ��ұ��룩��ջ�ϵ�һ������Ԫ�ص��õĹ����ַ����������ĺ�������Ԫ����Ҫ����ӵ���������ֵ�� 
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
	��ʼ��������B���������������κοռ䣻�����������ѱ�����Ϊ��������luaL_Buffer���� 
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
	������t���ı���Ϊջ���Ķ��󴴽�һ�����ã�reference�������أ����ҵ����ö��󣩡� 
	������Ψһ����������ֻҪ�㲻�ֹ����t�м�����������luaL_ref��֤�����صļ���Ψһ�ԡ����ͨ������lua_rawgeti(L, t, r)ȡ�ر�r���õĶ��󡣺���luaL_unref�ͷ����ü�������Ķ��� 
	���ջ���Ķ�����nil��luaL_ref���س���LUA_REFNIL������LUA_NOREF��ȷ����luaL_ref���ص��κ����ö���ͬ�� 
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
	�����������t���ı������ref����luaL_ref��������ӱ���ɾ�������Ա����õĶ���ɱ����ա�����refҲ���ͷ��Ա��ٴ�ʹ�á� 
	���ref��LUA_NOREF��LUA_REFNIL��luaL_unrefʲôҲ������ 
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
	�����ļ���Ϊһ��Lua��Ԫ����������lua_load��������Ϊfilename���ļ��еĵ�Ԫ�����filename��NULL����ӱ�׼�������롣�ļ��еĵ�һ�������#��ͷ�򱻺��ԡ� 
	����������ͬlua_loadһ���Ľ���������и�����Ĵ������LUA_ERRFILE�����ڲ��ܴ�/��ȡ�ļ�������� 
	ͬlua_loadһ����������ֻ���뵥Ԫ�������������� 
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
	���뻺��������Ϊһ��Lua��Ԫ����������lua_load�����ػ���������buffָ���ҳ���Ϊsz�ĵ�Ԫ�� 
	����������ͬlua_loadһ���Ľ����name�ǵ�Ԫ���֣����ڵ�����Ϣ�ʹ�����Ϣ�� 
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
	�����ַ�����Ϊһ��Lua��Ԫ����������lua_load��������0��β���ַ���s�еĵ�Ԫ�� 
	����������ͬlua_loadһ���Ľ���� 
	������ֻ���뵥Ԫ����Ҳͬlua_loadһ���������������� 
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
	�����µ�Lua״̬�������û��ڱ�׼C��realloc�����ķ���������lua_newstate��Ȼ������һ���ڷ����ش����ʱ���׼���������ӡһ��������Ϣ��Ӧ����������lua_atpanic���� 
	�����µ�״̬������������ڴ��������򷵻�NULL�� 
*/
LUALIB_API lua_State *luaL_newstate (void) {
  lua_State *L = lua_newstate(l_alloc, NULL);
  if (L) lua_atpanic(L, &panic);
  return L;
}

