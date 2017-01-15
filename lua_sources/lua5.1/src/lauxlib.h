/*
** $Id: lauxlib.h,v 1.88.1.1 2007/12/27 13:02:25 roberto Exp $
** Auxiliary functions for building Lua libraries
** See Copyright Notice in lua.h
*/


#ifndef lauxlib_h
#define lauxlib_h


#include <stddef.h>
#include <stdio.h>

#include "lua.h"

#if defined(LUA_COMPAT_GETN)
LUALIB_API int (luaL_getn) (lua_State *L, int t);
LUALIB_API void (luaL_setn) (lua_State *L, int t, int n);
#else
#define luaL_getn(L,i)          ((int)lua_objlen(L, i))
#define luaL_setn(L,i,j)        ((void)0)  /* no op! */
#endif

#if defined(LUA_COMPAT_OPENLIB)
#define luaI_openlib	luaL_openlib
#endif


/* extra error code for `luaL_load' */
#define LUA_ERRFILE     (LUA_ERRERR+1)

/**
 luaL_Reg

 typedef struct luaL_Reg {

 const char *name;

 lua_CFunction func;

 } luaL_Reg;

 Type for arrays of functions to be registered by luaL_register. name is the function name and func is a pointer to the function. Any array of luaL_Reg must end with an sentinel entry in which both name and func are NULL. 

 ����Ҫ��luaL_registerע��ĺ�����������͡�name�Ǻ�������func�Ǻ���ָ�롣�κ�luaL_Reg���������name��func��ΪNULL�ı�����β�� 
 */
typedef struct luaL_Reg {
  const char *name;
  lua_CFunction func;
} luaL_Reg;



LUALIB_API void (luaI_openlib) (lua_State *L, const char *libname,
                                const luaL_Reg *l, int nup);

/**
luaL_register

[-(0|1), +1, m] 

void luaL_register (lua_State *L,
						const char *libname,
						const luaL_Reg *l);

Opens a library. 

When called with libname equal to NULL, it simply registers all functions in the list l (see luaL_Reg) into the table on the top of the stack. 

When called with a non-null libname, luaL_register creates a new table t, sets it as the value of the global variable libname, sets it as the value of package.loaded[libname], and registers on it all functions in the list l. If there is a table in package.loaded[libname] or in variable libname, reuses this table instead of creating a new one. 

In any case the function leaves the table on the top of the stack. 

��һ���⡣ 

����libname����NULL����ʱ����ֻ��ע���б�l�е����к�������luaL_Reg����ջ���ı��С� 

���Էǿյ�libname����ʱ��luaL_register�����±�t��������Ϊȫ�ֱ���libname��ֵ����package.loaded[libname]��ֵ�������б�l�е����к���ע�ᵽ�ñ����package.loaded[libname]�л����libname���и��������øñ�����Ǵ���һ���µġ� 

������κ������ѱ�����ջ���� 
*/
LUALIB_API void (luaL_register) (lua_State *L, const char *libname,
                                const luaL_Reg *l);

/**
luaL_getmetafield

[-0, +(0|1), m] 

int luaL_getmetafield (lua_State *L, int obj, const char *e);

Pushes onto the stack the field e from the metatable of the object at index obj. If the object does not have a metatable, or if the metatable does not have this field, returns 0 and pushes nothing. 

����������obj���Ķ����Ԫ����ֶ�eѹջ���������û��Ԫ�����Ԫ��û�и��ֶΣ��򷵻�0�Ҳ���ѹջ�κζ����� 
*/
LUALIB_API int (luaL_getmetafield) (lua_State *L, int obj, const char *e);

/**
luaL_callmeta

[-0, +(0|1), e] 

int luaL_callmeta (lua_State *L, int obj, const char *e);

Calls a metamethod. 

If the object at index obj has a metatable and this metatable has a field e, this function calls this field and passes the object as its only argument. In this case this function returns 1 and pushes onto the stack the value returned by the call. If there is no metatable or no metamethod, this function returns 0 (without pushing any value on the stack). 

����һ��Ԫ������ 

�������obj���Ķ������Ԫ���Ҹ�Ԫ������ֶ�e�����������ø��ֶβ�����ö���Ϊ��Ψһ��������������£�����������1�����õ��÷��ص�ֵѹջ�����û��Ԫ���û��Ԫ����������������0�������κ�ֵѹջ���� 
*/
LUALIB_API int (luaL_callmeta) (lua_State *L, int obj, const char *e);

/**
luaL_typerror

[-0, +0, v] 

int luaL_typerror (lua_State *L, int narg, const char *tname);

Generates an error with a message like the following: 

location: bad argument narg to 'func' (tname expected, got rt)

where location is produced by luaL_where, func is the name of the current function, and rt is the type name of the actual argument. 

�������������Ϣ����һ������ 

location: bad argument narg to 'func' (tname expected, got rt)

����location��luaL_where������func�ǵ�ǰ����������rt��ʵ�ʲ������������� 
*/
LUALIB_API int (luaL_typerror) (lua_State *L, int narg, const char *tname);

/**
luaL_argerror

[-0, +0, v] 

int luaL_argerror (lua_State *L, int narg, const char *extramsg);

Raises an error with the following message, where func is retrieved from the call stack: 

bad argument #&lt;narg&gt; to &lt;func&gt; (&lt;extramsg&gt;)

This function never returns, but it is an idiom to use it in C functions as return luaL_argerror(args). 

�׳����������Ϣ�Ĵ�������func��ȡ�Ե���ջ�� 

bad argument #&lt;narg&gt; to &lt;func&gt; (&lt;extramsg&gt;)

�������������أ�������return luaL_argerror(args)����������C�����е�ϰ���÷��� 
*/
LUALIB_API int (luaL_argerror) (lua_State *L, int numarg, const char *extramsg);

/**
luaL_checklstring

[-0, +0, v] 

const char *luaL_checklstring (lua_State *L, int narg, size_t *l);

Checks whether the function argument narg is a string and returns this string; if l is not NULL fills *l with the string's length. 

This function uses lua_tolstring to get its result, so all conversions and caveats of that function apply here. 

��麯������narg�Ƿ���һ���ַ�����Ȼ�󷵻�����ַ��������l����NULL�����*lΪ�ַ����ĳ��ȡ�

�������ʹ��lua_tolstring��ý���������Ǹ�����������ת����ע����������ڴˡ�
*/
LUALIB_API const char *(luaL_checklstring) (lua_State *L, int numArg,
                                                          size_t *l);

/**
luaL_optlstring

[-0, +0, v] 

const char *luaL_optlstring (lua_State *L,
								 int narg,
								 const char *d,
								 size_t *l);

If the function argument narg is a string, returns this string. If this argument is absent or is nil, returns d. Otherwise, raises an error. 

If l is not NULL, fills the position *l with the results's length. 

�����������narg���ַ����򷵻���������ò��������ڻ�Ϊnil�򷵻�d�������������� 

���l��ΪNULL�����ý���ĳ������λ��*l�� 
*/
LUALIB_API const char *(luaL_optlstring) (lua_State *L, int numArg,
                                          const char *def, size_t *l);

/**
luaL_checknumber

[-0, +0, v] 

lua_Number luaL_checknumber (lua_State *L, int narg);

Checks whether the function argument narg is a number and returns this number. 

��麯������narg�Ƿ���һ���������ҷ����������
*/
LUALIB_API lua_Number (luaL_checknumber) (lua_State *L, int numArg);

/**
luaL_optnumber

[-0, +0, v] 

lua_Number luaL_optnumber (lua_State *L, int narg, lua_Number d);

If the function argument narg is a number, returns this number. If this argument is absent or is nil, returns d. Otherwise, raises an error. 

�����������narg�������򷵻���������ò��������ڻ�Ϊnil�򷵻�d�������������� 
*/
LUALIB_API lua_Number (luaL_optnumber) (lua_State *L, int nArg, lua_Number def);

/**
luaL_checkinteger

[-0, +0, v] 

lua_Integer luaL_checkinteger (lua_State *L, int narg);

Checks whether the function argument narg is a number and returns this number cast to a lua_Integer. 

��麯������narg�Ƿ���һ���������Ұ������ת��Ϊlua_IntegerȻ�󷵻ء�
*/
LUALIB_API lua_Integer (luaL_checkinteger) (lua_State *L, int numArg);

/**
luaL_optinteger

[-0, +0, v] 

lua_Integer luaL_optinteger (lua_State *L,
								 int narg,
								 lua_Integer d);

If the function argument narg is a number, returns this number cast to a lua_Integer. If this argument is absent or is nil, returns d. Otherwise, raises an error. 

�����������narg�����֣���Ѹ�����ת��Ϊlua_Integer���ء�����ò��������ڻ�Ϊnil�򷵻�d�������������� 
*/
LUALIB_API lua_Integer (luaL_optinteger) (lua_State *L, int nArg,
                                          lua_Integer def);

/**
luaL_checkstack

[-0, +0, v] 

void luaL_checkstack (lua_State *L, int sz, const char *msg);

Grows the stack size to top + sz elements, raising an error if the stack cannot grow to that size. msg is an additional text to go into the error message. 

����ջ�ߴ絽top + sz��Ԫ�أ���������������Ǹ��ߴ�����������msg�Ǽ��������Ϣ�Ĳ����ı��� 
*/
LUALIB_API void (luaL_checkstack) (lua_State *L, int sz, const char *msg);

/**
luaL_checktype

[-0, +0, v] 

void luaL_checktype (lua_State *L, int narg, int t);

Checks whether the function argument narg has type t. See lua_type for the encoding of types for t. 

��麯������narg�Ƿ�ӵ������t��

�ο�lua_type���t�����ͱ��롣
*/
LUALIB_API void (luaL_checktype) (lua_State *L, int narg, int t);

/**
luaL_checkany

[-0, +0, v] 

void luaL_checkany (lua_State *L, int narg);

Checks whether the function has an argument of any type (including nil) at position narg. 

��麯���Ƿ���λ��narg���и��������ͣ�����nil���Ĳ����� 
*/
LUALIB_API void (luaL_checkany) (lua_State *L, int narg);

/**
luaL_newmetatable

[-0, +1, m] 

int luaL_newmetatable (lua_State *L, const char *tname);

If the registry already has the key tname, returns 0. Otherwise, creates a new table to be used as a metatable for userdata, adds it to the registry with key tname, and returns 1. 

In both cases pushes onto the stack the final value associated with tname in the registry. 

���ע����Ѿ����˼�tname�򷵻�0�����򣬴����������û����ݵ�Ԫ����±�����ͬ��tnameһ�����ע������ҷ���1�� 

�����������ע�������tname�����������ֵѹջ�� 
*/
LUALIB_API int   (luaL_newmetatable) (lua_State *L, const char *tname);

/**
luaL_checkudata

[-0, +0, v] 

void *luaL_checkudata (lua_State *L, int narg, const char *tname);

Checks whether the function argument narg is a userdata of the type tname (see luaL_newmetatable). 

��麯������narg�Ƿ�������tname���û��������ͣ��ο�luaL_newmetatable��
*/
LUALIB_API void *(luaL_checkudata) (lua_State *L, int ud, const char *tname);

/**
luaL_where

[-0, +1, m] 

void luaL_where (lua_State *L, int lvl);

Pushes onto the stack a string identifying the current position of the control at level lvl in the call stack. Typically this string has the following format: 

chunkname:currentline:

Level 0 is the running function, level 1 is the function that called the running function, etc. 

This function is used to build a prefix for error messages. 

ѹ��һ����ʶ��ǰ��lvl����ö�ջ�Ŀ���λ�õ���ջ�С�

�ر������ַ�����ʽ���£�

����:��ǰ��:

0�����������еĺ�����1���ǵ��������к����ĺ�����������ơ�

����������ڹ���������Ϣ��ǰ׺��
*/
LUALIB_API void (luaL_where) (lua_State *L, int lvl);

/**
luaL_error

[-0, +0, v] 

int luaL_error (lua_State *L, const char *fmt, ...);

Raises an error. The error message format is given by fmt plus any extra arguments, following the same rules of lua_pushfstring. It also adds at the beginning of the message the file name and the line number where the error occurred, if this information is available. 

This function never returns, but it is an idiom to use it in C functions as return luaL_error(args). 

��������

������Ϣ�ĸ�ʽ��fmtָ������Ӷ���Ĳ�������ѭ��lua_pushfstring��ͬ�Ĺ���

����������Ϣ�Ŀ�ͷ�����ļ����ͷ���������кţ����������Ϣ���õĻ���

��������Ӳ����أ���ϰ����C��������return luaL_error(args)��ʽʹ�á�
*/
LUALIB_API int (luaL_error) (lua_State *L, const char *fmt, ...);

/**
luaL_checkoption

[-0, +0, v] 

int luaL_checkoption (lua_State *L,
						  int narg,
						  const char *def,
						  const char *const lst[]);

Checks whether the function argument narg is a string and searches for this string in the array lst (which must be NULL-terminated). Returns the index in the array where the string was found. Raises an error if the argument is not a string or if the string cannot be found. 

If def is not NULL, the function uses def as a default value when there is no argument narg or if this argument is nil. 

This is a useful function for mapping strings to C enums. (The usual convention in Lua libraries is to use strings instead of numbers to select options.) 

��麯������narg�Ƿ���һ���ַ�������������lst����������ַ�����������NULL��������

�������������ҵ��ַ�����������

�����������һ���ַ������ַ����Ҳ�������������

���def����NULL����û��narg���������������Ϊ��ʱ���������ʹ��def��ΪĬ��ֵ

���������ƥ���ַ���ΪCö��ֵʱ�����á�

����Lua�����ձ��Լ����ʹ���ַ�����������ȥѡ��ѡ�
*/
LUALIB_API int (luaL_checkoption) (lua_State *L, int narg, const char *def,
                                   const char *const lst[]);

/**
luaL_ref

[-1, +0, m] 

int luaL_ref (lua_State *L, int t);

Creates and returns a reference, in the table at index t, for the object at the top of the stack (and pops the object). 

A reference is a unique integer key. As long as you do not manually add integer keys into table t, luaL_ref ensures the uniqueness of the key it returns. You can retrieve an object referred by reference r by calling lua_rawgeti(L, t, r). Function luaL_unref frees a reference and its associated object. 

If the object at the top of the stack is nil, luaL_ref returns the constant LUA_REFNIL. The constant LUA_NOREF is guaranteed to be different from any reference returned by luaL_ref. 

������t���ı���Ϊջ���Ķ��󴴽�һ�����ã�reference�������أ����ҵ����ö��󣩡� 

������Ψһ����������ֻҪ�㲻�ֹ����t�м�����������luaL_ref��֤�����صļ���Ψһ�ԡ����ͨ������lua_rawgeti(L, t, r)ȡ�ر�r���õĶ��󡣺���luaL_unref�ͷ����ü�������Ķ��� 

���ջ���Ķ�����nil��luaL_ref���س���LUA_REFNIL������LUA_NOREF��ȷ����luaL_ref���ص��κ����ö���ͬ�� 
*/
LUALIB_API int (luaL_ref) (lua_State *L, int t);

/**
luaL_unref

[-0, +0, -] 

void luaL_unref (lua_State *L, int t, int ref);

Releases reference ref from the table at index t (see luaL_ref). The entry is removed from the table, so that the referred object can be collected. The reference ref is also freed to be used again. 

If ref is LUA_NOREF or LUA_REFNIL, luaL_unref does nothing. 

�����������t���ı������ref����luaL_ref��������ӱ���ɾ�������Ա����õĶ���ɱ����ա�����refҲ���ͷ��Ա��ٴ�ʹ�á� 

���ref��LUA_NOREF��LUA_REFNIL��luaL_unrefʲôҲ������ 
*/
LUALIB_API void (luaL_unref) (lua_State *L, int t, int ref);

/**
luaL_loadfile

[-0, +1, m] 

int luaL_loadfile (lua_State *L, const char *filename);

Loads a file as a Lua chunk. This function uses lua_load to load the chunk in the file named filename. If filename is NULL, then it loads from the standard input. The first line in the file is ignored if it starts with a #. 

This function returns the same results as lua_load, but it has an extra error code LUA_ERRFILE if it cannot open/read the file. 

As lua_load, this function only loads the chunk; it does not run it. 

�����ļ���Ϊһ��Lua��Ԫ����������lua_load��������Ϊfilename���ļ��еĵ�Ԫ�����filename��NULL����ӱ�׼�������롣�ļ��еĵ�һ�������#��ͷ�򱻺��ԡ� 

����������ͬlua_loadһ���Ľ���������и�����Ĵ������LUA_ERRFILE�����ڲ��ܴ�/��ȡ�ļ�������� 

ͬlua_loadһ����������ֻ���뵥Ԫ�������������� 
*/
LUALIB_API int (luaL_loadfile) (lua_State *L, const char *filename);

/**
luaL_loadbuffer

[-0, +1, m] 

int luaL_loadbuffer (lua_State *L,
						 const char *buff,
						 size_t sz,
						 const char *name);

Loads a buffer as a Lua chunk. This function uses lua_load to load the chunk in the buffer pointed to by buff with size sz. 

This function returns the same results as lua_load. name is the chunk name, used for debug information and error messages. 

���뻺��������Ϊһ��Lua��Ԫ����������lua_load�����ػ���������buffָ���ҳ���Ϊsz�ĵ�Ԫ�� 

����������ͬlua_loadһ���Ľ����name�ǵ�Ԫ���֣����ڵ�����Ϣ�ʹ�����Ϣ�� 
*/
LUALIB_API int (luaL_loadbuffer) (lua_State *L, const char *buff, size_t sz,
                                  const char *name);

/**
luaL_loadstring

[-0, +1, m] 

int luaL_loadstring (lua_State *L, const char *s);

Loads a string as a Lua chunk. This function uses lua_load to load the chunk in the zero-terminated string s. 

This function returns the same results as lua_load. 

Also as lua_load, this function only loads the chunk; it does not run it. 

�����ַ�����Ϊһ��Lua��Ԫ����������lua_load��������0��β���ַ���s�еĵ�Ԫ�� 

����������ͬlua_loadһ���Ľ���� 

������ֻ���뵥Ԫ����Ҳͬlua_loadһ���������������� 
*/
LUALIB_API int (luaL_loadstring) (lua_State *L, const char *s);

/**
luaL_newstate

[-0, +0, -] 

lua_State *luaL_newstate (void);

Creates a new Lua state. It calls lua_newstate with an allocator based on the standard C realloc function and then sets a panic function (see lua_atpanic) that prints an error message to the standard error output in case of fatal errors. 

Returns the new state, or NULL if there is a memory allocation error. 

�����µ�Lua״̬�������û��ڱ�׼C��realloc�����ķ���������lua_newstate��Ȼ������һ���ڷ����ش����ʱ���׼���������ӡһ��������Ϣ��Ӧ����������lua_atpanic���� 

�����µ�״̬������������ڴ��������򷵻�NULL�� 
*/
LUALIB_API lua_State *(luaL_newstate) (void);

/**
luaL_gsub

[-0, +1, m] 

const char *luaL_gsub (lua_State *L,
                       const char *s,
                       const char *p,
                       const char *r);

Creates a copy of string s by replacing any occurrence of the string p with the string r. Pushes the resulting string on the stack and returns it. 

ͨ���ѳ��ֵ��κ��ַ���p�滻Ϊ�ַ���r�������ַ���s�Ŀ������ѽ���ַ���ѹջ���������� 
*/
LUALIB_API const char *(luaL_gsub) (lua_State *L, const char *s, const char *p,
                                                  const char *r);

LUALIB_API const char *(luaL_findtable) (lua_State *L, int idx,
                                         const char *fname, int szhint);




/*
** ===============================================================
** some useful macros
** ===============================================================
*/

/**
 luaL_argcheck

 [-0, +0, v] 

 void luaL_argcheck (lua_State *L,
						int cond,
						int narg,
						const char *extramsg);

  Checks whether cond is true. If not, raises an error with the following message, where func is retrieved from the call stack: 
  
  bad argument #&lt;narg&gt; to &lt;func&gt; (&lt;extramsg&gt;)
  
  ��������Ƿ�Ϊ�档

  ������ǣ����������Ϣ����һ����������func�Ǵӵ��ö�ջ������ȡ�á�

  bad argument #&lt;narg&gt; to &lt;func&gt; (&lt;extramsg&gt;)
 */
#define luaL_argcheck(L, cond,numarg,extramsg)	\
		((void)((cond) || luaL_argerror(L, (numarg), (extramsg))))

/**
 luaL_checkstring

 [-0, +0, v] 

 const char *luaL_checkstring (lua_State *L, int narg);

 Checks whether the function argument narg is a string and returns this string. 

 This function uses lua_tolstring to get its result, so all conversions and caveats of that function apply here. 

 ��麯������narg�Ƿ��ַ������������� 

 ��������lua_tolstringȡ�������������Ǹ����������б任�;���Ҳ�����ڴ˴��� 
 */
#define luaL_checkstring(L,n)	(luaL_checklstring(L, (n), NULL))

/**
 luaL_optstring

 [-0, +0, v] 

 const char *luaL_optstring (lua_State *L,
 								int narg,
 								const char *d);

 If the function argument narg is a string, returns this string. If this argument is absent or is nil, returns d. Otherwise, raises an error. 

 �����������narg���ַ����򷵻���������ò��������ڻ�Ϊnil�򷵻�d�������������� 
 */
#define luaL_optstring(L,n,d)	(luaL_optlstring(L, (n), (d), NULL))

/**
 luaL_checkint

 [-0, +0, v] 

 int luaL_checkint (lua_State *L, int narg);

 Checks whether the function argument narg is a number and returns this number cast to an int. 

 ��麯������narg�Ƿ�Ϊһ���������Ұ������ת��Ϊ����Ȼ�󷵻ء�
 */
#define luaL_checkint(L,n)	((int)luaL_checkinteger(L, (n)))

/**
 luaL_optint

 [-0, +0, v] 

 int luaL_optint (lua_State *L, int narg, int d);

 If the function argument narg is a number, returns this number cast to an int. If this argument is absent or is nil, returns d. Otherwise, raises an error. 

 �����������narg�����֣���Ѹ�����ת��Ϊint���ء�����ò��������ڻ�Ϊnil�򷵻�d�������������� 
 */
#define luaL_optint(L,n,d)	((int)luaL_optinteger(L, (n), (d)))

/**
 luaL_checklong

 [-0, +0, v] 

 long luaL_checklong (lua_State *L, int narg);

 Checks whether the function argument narg is a number and returns this number cast to a long. 

 ��麯������narg�Ƿ���һ���������Ұ������ת��Ϊ������Ȼ�󷵻ء�
 */
#define luaL_checklong(L,n)	((long)luaL_checkinteger(L, (n)))

/**
 luaL_optlong

 [-0, +0, v] 

 long luaL_optlong (lua_State *L, int narg, long d);

 If the function argument narg is a number, returns this number cast to a long. If this argument is absent or is nil, returns d. Otherwise, raises an error. 

 �����������narg�����֣���Ѹ�����ת��Ϊlong���ء�����ò��������ڻ�Ϊnil�򷵻�d�������������� 
 */
#define luaL_optlong(L,n,d)	((long)luaL_optinteger(L, (n), (d)))

/**
 luaL_typename

 [-0, +0, -] 

 const char *luaL_typename (lua_State *L, int index);

 Returns the name of the type of the value at the given index. 

 ���ظ�����������ֵ���������� 
 */
#define luaL_typename(L,i)	lua_typename(L, lua_type(L,(i)))

/**
 luaL_dofile

 [-0, +?, m] 

 int luaL_dofile (lua_State *L, const char *filename);

 Loads and runs the given file. It is defined as the following macro: 

 (luaL_loadfile(L, filename) || lua_pcall(L, 0, LUA_MULTRET, 0))

 It returns 0 if there are no errors or 1 in case of errors. 

 ���ز������������ļ�������Ϊ���º�

 (luaL_loadfile(L, filename) || lua_pcall(L, 0, LUA_MULTRET, 0))

 ���û�д����򷵻�0�����������1��
 */
#define luaL_dofile(L, fn) \
	(luaL_loadfile(L, fn) || lua_pcall(L, 0, LUA_MULTRET, 0))

/**
 luaL_dostring

 [-0, +?, m] 

 int luaL_dostring (lua_State *L, const char *str);

 Loads and runs the given string. It is defined as the following macro: 

 (luaL_loadstring(L, str) || lua_pcall(L, 0, LUA_MULTRET, 0))

 It returns 0 if there are no errors or 1 in case of errors. 

 ���ز������������ַ���������Ϊ���º꣺

 (luaL_loadstring(L, str) || lua_pcall(L, 0, LUA_MULTRET, 0))

 ���û�д����򷵻�0�����������1��
 */
#define luaL_dostring(L, s) \
	(luaL_loadstring(L, s) || lua_pcall(L, 0, LUA_MULTRET, 0))

/**
  luaL_getmetatable

  [-0, +1, -] 

  void luaL_getmetatable (lua_State *L, const char *tname);

  Pushes onto the stack the metatable associated with name tname in the registry (see luaL_newmetatable). 

  ����ע����е�����tname�������Ԫ��ѹջ������luaL_newmetatable���� 
 */
#define luaL_getmetatable(L,n)	(lua_getfield(L, LUA_REGISTRYINDEX, (n)))

#define luaL_opt(L,f,n,d)	(lua_isnoneornil(L,(n)) ? (d) : f(L,(n)))

/*
** {======================================================
** Generic Buffer manipulation
** =======================================================
*/


/**
  luaL_Buffer

  typedef struct luaL_Buffer luaL_Buffer;

  Type for a string buffer. 

  A string buffer allows C code to build Lua strings piecemeal. Its pattern of use is as follows: 

  First you declare a variable b of type luaL_Buffer. 

  Then you initialize it with a call luaL_buffinit(L, &b). 

  Then you add string pieces to the buffer calling any of the luaL_add* functions. 

  You finish by calling luaL_pushresult(&b). This call leaves the final string on the top of the stack. 

  During its normal operation, a string buffer uses a variable number of stack slots. So, while using a buffer, you cannot assume that you know where the top of the stack is. You can use the stack between successive calls to buffer operations as long as that use is balanced; that is, when you call a buffer operation, the stack is at the same level it was immediately after the previous buffer operation. (The only exception to this rule is luaL_addvalue.) After calling luaL_pushresult the stack is back to its level when the buffer was initialized, plus the final string on its top. 

  �ַ������������͡� 

  �ַ�������������C������εع���Lua�ַ�������ʹ��ģʽ���£� 

  ��������luaL_Buffer���͵ı���b�� 

  ���ŵ���luaL_buffinit(L, &b)��ʼ������ 

  Ȼ�����luaL_add*�������ַ���Ƭ����ӵ��������� 

  ͨ������luaL_pushresult(&b)�������õ��ð����յ��ַ�������ջ���� 

  ���ַ����������ĳ�������ڼ䣬��ռ�õ�ջ���������̶������ԣ�ʹ�û�����ʱ���㲻�ܼٶ�֪��ջ�����Ķ���ֻҪ�÷��ǶԳƵģ�������������ĶԻ����������ĵ���֮��ʹ��ջ������˵��������û���������ʱ��ջ����ǰһ�����������������̵õ���ˮƽ�����ù����Ψһ������luaL_addvalue��������luaL_pushresult֮��ջ�ص�����������ʼ��ʱ��ˮƽ������ջ���������ַ����� 
 */
typedef struct luaL_Buffer {
  char *p;			/* current position in buffer */
  int lvl;  /* number of strings in the stack (level) */
  lua_State *L;
  char buffer[LUAL_BUFFERSIZE];
} luaL_Buffer;

/**
  luaL_addchar

 	[-0, +0, m] 
  
 	void luaL_addchar (luaL_Buffer *B, char c);
  
  	Adds the character c to the buffer B (see luaL_Buffer).
 
    ���ַ�c���뻺����B���ο�luaL_Buffer����
 */
#define luaL_addchar(B,c) \
  ((void)((B)->p < ((B)->buffer+LUAL_BUFFERSIZE) || luaL_prepbuffer(B)), \
   (*(B)->p++ = (char)(c)))

/* compatibility only */
#define luaL_putchar(B,c)	luaL_addchar(B,c)

/**
  luaL_addsize

  [-0, +0, m] 

  	void luaL_addsize (luaL_Buffer *B, size_t n);

  	Adds to the buffer B (see luaL_Buffer) a string of length n previously copied to the buffer area (see luaL_prepbuffer). 

  	��ǰһ�����Ƶ����������ο�luaL_prepbuffer���ĳ���n���ַ������뻺��B���ο�luaL_Buffer��
 */
#define luaL_addsize(B,n)	((B)->p += (n))

/**
luaL_buffinit

[-0, +0, -] 

void luaL_buffinit (lua_State *L, luaL_Buffer *B);

Initializes a buffer B. This function does not allocate any space; the buffer must be declared as a variable (see luaL_Buffer). 

��ʼ��������B���������������κοռ䣻�����������ѱ�����Ϊ��������luaL_Buffer���� 
*/
LUALIB_API void (luaL_buffinit) (lua_State *L, luaL_Buffer *B);

/**
luaL_prepbuffer

[-0, +0, -] 

char *luaL_prepbuffer (luaL_Buffer *B);

Returns an address to a space of size LUAL_BUFFERSIZE where you can copy a string to be added to buffer B (see luaL_Buffer). After copying the string into this space you must call luaL_addsize with the size of the string to actually add it to the buffer. 

���سߴ�ΪLUAL_BUFFERSIZE�Ŀռ�ĵ�ַ�����ܰ�Ҫ�����뻺����B���ַ������������У���luaL_Buffer�����ڰ��ַ����������ÿռ����Ժ���������ַ����ĳߴ����luaL_addsize���������뻺�����С� 
*/
LUALIB_API char *(luaL_prepbuffer) (luaL_Buffer *B);

/**
luaL_addlstring

[-0, +0, m] 

void luaL_addlstring (luaL_Buffer *B, const char *s, size_t l);

Adds the string pointed to by s with length l to the buffer B (see luaL_Buffer). The string may contain embedded zeros. 

��ָ��s�ĳ���l���ַ������뻺����B���ο�luaL_Buffer����

����ַ������԰���Ƕ���0��
*/
LUALIB_API void (luaL_addlstring) (luaL_Buffer *B, const char *s, size_t l);

/**
luaL_addstring

[-0, +0, m] 

void luaL_addstring (luaL_Buffer *B, const char *s);

Adds the zero-terminated string pointed to by s to the buffer B (see luaL_Buffer). The string may not contain embedded zeros. 

��sָ���0��β���ַ�����ӵ�������B����luaL_Buffer�����ַ������ɰ�����Ƕ��0�� 
*/
LUALIB_API void (luaL_addstring) (luaL_Buffer *B, const char *s);

/**
luaL_addvalue

[-1, +0, m] 

void luaL_addvalue (luaL_Buffer *B);

Adds the value at the top of the stack to the buffer B (see luaL_Buffer). Pops the value. 

This is the only function on string buffers that can (and must) be called with an extra element on the stack, which is the value to be added to the buffer. 

��ջ����ֵ��ӵ�������B����luaL_Buffer����������ֵ�� 

���ǽ����ܣ��ұ��룩��ջ�ϵ�һ������Ԫ�ص��õĹ����ַ����������ĺ�������Ԫ����Ҫ����ӵ���������ֵ�� 
*/
LUALIB_API void (luaL_addvalue) (luaL_Buffer *B);

/**
luaL_pushresult

[-?, +1, m] 

void luaL_pushresult (luaL_Buffer *B);

Finishes the use of buffer B leaving the final string on the top of the stack. 

�����Ի�����B��ʹ�ã��������ַ�������ջ���� 
*/
LUALIB_API void (luaL_pushresult) (luaL_Buffer *B);


/* }====================================================== */


/* compatibility with ref system */

/* pre-defined references */
#define LUA_NOREF       (-2)
#define LUA_REFNIL      (-1)

#define lua_ref(L,lock) ((lock) ? luaL_ref(L, LUA_REGISTRYINDEX) : \
      (lua_pushstring(L, "unlocked references are obsolete"), lua_error(L), 0))

#define lua_unref(L,ref)        luaL_unref(L, LUA_REGISTRYINDEX, (ref))

#define lua_getref(L,ref)       lua_rawgeti(L, LUA_REGISTRYINDEX, (ref))


#define luaL_reg	luaL_Reg

#endif


