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

 用于要被luaL_register注册的函数数组的类型。name是函数名，func是函数指针。任何luaL_Reg数组必须以name和func都为NULL的标记项结尾。 
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

打开一个库。 

当以libname等于NULL调用时，它只是注册列表l中的所有函数（见luaL_Reg）到栈顶的表中。 

当以非空的libname调用时，luaL_register创建新表t，把它设为全局变量libname的值，和package.loaded[libname]的值，并把列表l中的所有函数注册到该表。如果package.loaded[libname]中或变量libname中有个表，则重用该表而不是创建一个新的。 

无论如何函数都把表留在栈顶。 
*/
LUALIB_API void (luaL_register) (lua_State *L, const char *libname,
                                const luaL_Reg *l);

/**
luaL_getmetafield

[-0, +(0|1), m] 

int luaL_getmetafield (lua_State *L, int obj, const char *e);

Pushes onto the stack the field e from the metatable of the object at index obj. If the object does not have a metatable, or if the metatable does not have this field, returns 0 and pushes nothing. 

把来自索引obj处的对象的元表的字段e压栈。如果对象没有元表或其元表没有该字段，则返回0且不会压栈任何东西。 
*/
LUALIB_API int (luaL_getmetafield) (lua_State *L, int obj, const char *e);

/**
luaL_callmeta

[-0, +(0|1), e] 

int luaL_callmeta (lua_State *L, int obj, const char *e);

Calls a metamethod. 

If the object at index obj has a metatable and this metatable has a field e, this function calls this field and passes the object as its only argument. In this case this function returns 1 and pushes onto the stack the value returned by the call. If there is no metatable or no metamethod, this function returns 0 (without pushing any value on the stack). 

调用一个元方法。 

如果索引obj处的对象具有元表且该元表具有字段e，本函数调用该字段并传入该对象为其唯一参数。这种情况下，本函数返回1并将该调用返回的值压栈。如果没有元表或没有元方法，本函数返回0（不将任何值压栈）。 
*/
LUALIB_API int (luaL_callmeta) (lua_State *L, int obj, const char *e);

/**
luaL_typerror

[-0, +0, v] 

int luaL_typerror (lua_State *L, int narg, const char *tname);

Generates an error with a message like the following: 

location: bad argument narg to 'func' (tname expected, got rt)

where location is produced by luaL_where, func is the name of the current function, and rt is the type name of the actual argument. 

用类似下面的消息产生一个错误： 

location: bad argument narg to 'func' (tname expected, got rt)

其中location由luaL_where产生，func是当前函数名，且rt是实际参数的类型名。 
*/
LUALIB_API int (luaL_typerror) (lua_State *L, int narg, const char *tname);

/**
luaL_argerror

[-0, +0, v] 

int luaL_argerror (lua_State *L, int narg, const char *extramsg);

Raises an error with the following message, where func is retrieved from the call stack: 

bad argument #&lt;narg&gt; to &lt;func&gt; (&lt;extramsg&gt;)

This function never returns, but it is an idiom to use it in C functions as return luaL_argerror(args). 

抛出带下面的消息的错误，其中func是取自调用栈： 

bad argument #&lt;narg&gt; to &lt;func&gt; (&lt;extramsg&gt;)

本函数决不返回，但是像return luaL_argerror(args)这样是用在C函数中的习惯用法。 
*/
LUALIB_API int (luaL_argerror) (lua_State *L, int numarg, const char *extramsg);

/**
luaL_checklstring

[-0, +0, v] 

const char *luaL_checklstring (lua_State *L, int narg, size_t *l);

Checks whether the function argument narg is a string and returns this string; if l is not NULL fills *l with the string's length. 

This function uses lua_tolstring to get its result, so all conversions and caveats of that function apply here. 

检查函数参数narg是否是一个字符串，然后返回这个字符串；如果l不是NULL则填充*l为字符串的长度。

这个函数使用lua_tolstring获得结果，所以那个函数的所有转换和注意事项都适用于此。
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

如果函数参数narg是字符串则返回它。如果该参数不存在或为nil则返回d。否则引发错误。 

如果l不为NULL，则用结果的长度填充位置*l。 
*/
LUALIB_API const char *(luaL_optlstring) (lua_State *L, int numArg,
                                          const char *def, size_t *l);

/**
luaL_checknumber

[-0, +0, v] 

lua_Number luaL_checknumber (lua_State *L, int narg);

Checks whether the function argument narg is a number and returns this number. 

检查函数参数narg是否是一个数，并且返回这个数。
*/
LUALIB_API lua_Number (luaL_checknumber) (lua_State *L, int numArg);

/**
luaL_optnumber

[-0, +0, v] 

lua_Number luaL_optnumber (lua_State *L, int narg, lua_Number d);

If the function argument narg is a number, returns this number. If this argument is absent or is nil, returns d. Otherwise, raises an error. 

如果函数参数narg是数字则返回它。如果该参数不存在或为nil则返回d。否则引发错误。 
*/
LUALIB_API lua_Number (luaL_optnumber) (lua_State *L, int nArg, lua_Number def);

/**
luaL_checkinteger

[-0, +0, v] 

lua_Integer luaL_checkinteger (lua_State *L, int narg);

Checks whether the function argument narg is a number and returns this number cast to a lua_Integer. 

检查函数参数narg是否是一个数，并且把这个数转换为lua_Integer然后返回。
*/
LUALIB_API lua_Integer (luaL_checkinteger) (lua_State *L, int numArg);

/**
luaL_optinteger

[-0, +0, v] 

lua_Integer luaL_optinteger (lua_State *L,
								 int narg,
								 lua_Integer d);

If the function argument narg is a number, returns this number cast to a lua_Integer. If this argument is absent or is nil, returns d. Otherwise, raises an error. 

如果函数参数narg是数字，则把该数字转型为lua_Integer返回。如果该参数不存在或为nil则返回d。否则引发错误。 
*/
LUALIB_API lua_Integer (luaL_optinteger) (lua_State *L, int nArg,
                                          lua_Integer def);

/**
luaL_checkstack

[-0, +0, v] 

void luaL_checkstack (lua_State *L, int sz, const char *msg);

Grows the stack size to top + sz elements, raising an error if the stack cannot grow to that size. msg is an additional text to go into the error message. 

增长栈尺寸到top + sz个元素，如果不能增长到那个尺寸则引发错误。msg是加入错误消息的补充文本。 
*/
LUALIB_API void (luaL_checkstack) (lua_State *L, int sz, const char *msg);

/**
luaL_checktype

[-0, +0, v] 

void luaL_checktype (lua_State *L, int narg, int t);

Checks whether the function argument narg has type t. See lua_type for the encoding of types for t. 

检查函数参数narg是否拥有类型t。

参考lua_type获得t的类型编码。
*/
LUALIB_API void (luaL_checktype) (lua_State *L, int narg, int t);

/**
luaL_checkany

[-0, +0, v] 

void luaL_checkany (lua_State *L, int narg);

Checks whether the function has an argument of any type (including nil) at position narg. 

检查函数是否在位置narg处有个任意类型（包括nil）的参数。 
*/
LUALIB_API void (luaL_checkany) (lua_State *L, int narg);

/**
luaL_newmetatable

[-0, +1, m] 

int luaL_newmetatable (lua_State *L, const char *tname);

If the registry already has the key tname, returns 0. Otherwise, creates a new table to be used as a metatable for userdata, adds it to the registry with key tname, and returns 1. 

In both cases pushes onto the stack the final value associated with tname in the registry. 

如果注册表已经有了键tname则返回0。否则，创建将用作用户数据的元表的新表，把它同键tname一起加入注册表，并且返回1。 

两种情况都把注册表中与tname相关联的最终值压栈。 
*/
LUALIB_API int   (luaL_newmetatable) (lua_State *L, const char *tname);

/**
luaL_checkudata

[-0, +0, v] 

void *luaL_checkudata (lua_State *L, int narg, const char *tname);

Checks whether the function argument narg is a userdata of the type tname (see luaL_newmetatable). 

检查函数参数narg是否是类型tname的用户定义类型（参考luaL_newmetatable）
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

压入一个标识当前在lvl层调用堆栈的控制位置到堆栈中。

特别地这个字符串格式如下：

块名:当前行:

0层是正在运行的函数，1层是调用运行中函数的函数，如此类推。

这个函数用于构建错误信息的前缀。
*/
LUALIB_API void (luaL_where) (lua_State *L, int lvl);

/**
luaL_error

[-0, +0, v] 

int luaL_error (lua_State *L, const char *fmt, ...);

Raises an error. The error message format is given by fmt plus any extra arguments, following the same rules of lua_pushfstring. It also adds at the beginning of the message the file name and the line number where the error occurred, if this information is available. 

This function never returns, but it is an idiom to use it in C functions as return luaL_error(args). 

引发错误。

错误信息的格式由fmt指定，外加额外的参数，遵循与lua_pushfstring相同的规则。

它还会在信息的开头加上文件名和发生错误的行号，如果这类信息可用的话。

这个函数从不返回，但习惯在C函数内以return luaL_error(args)方式使用。
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

检查函数参数narg是否是一个字符串并且在数组lst中搜索这个字符串（必须是NULL结束）。

返回数组中所找到字符串的索引。

如果参数不是一个字符串或字符串找不到则引发错误。

如果def不是NULL，当没有narg参数或者这个参数为空时，这个函数使用def作为默认值

这个函数对匹配字符串为C枚举值时很有用。

（在Lua库中普遍的约定是使用字符串代替数字去选择选项）
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

在索引t处的表中为栈顶的对象创建一个引用（reference）并返回（而且弹出该对象）。 

引用是唯一的整数键。只要你不手工向表t中加入整数键，luaL_ref保证它返回的键的唯一性。你可通过调用lua_rawgeti(L, t, r)取回被r引用的对象。函数luaL_unref释放引用及其关联的对象。 

如果栈顶的对象是nil，luaL_ref返回常量LUA_REFNIL。常量LUA_NOREF被确保与luaL_ref返回的任何引用都不同。 
*/
LUALIB_API int (luaL_ref) (lua_State *L, int t);

/**
luaL_unref

[-0, +0, -] 

void luaL_unref (lua_State *L, int t, int ref);

Releases reference ref from the table at index t (see luaL_ref). The entry is removed from the table, so that the referred object can be collected. The reference ref is also freed to be used again. 

If ref is LUA_NOREF or LUA_REFNIL, luaL_unref does nothing. 

解除来自索引t处的表的引用ref（见luaL_ref）。该项从表中删除，所以被引用的对象可被回收。引用ref也被释放以备再次使用。 

如果ref是LUA_NOREF或LUA_REFNIL，luaL_unref什么也不做。 
*/
LUALIB_API void (luaL_unref) (lua_State *L, int t, int ref);

/**
luaL_loadfile

[-0, +1, m] 

int luaL_loadfile (lua_State *L, const char *filename);

Loads a file as a Lua chunk. This function uses lua_load to load the chunk in the file named filename. If filename is NULL, then it loads from the standard input. The first line in the file is ignored if it starts with a #. 

This function returns the same results as lua_load, but it has an extra error code LUA_ERRFILE if it cannot open/read the file. 

As lua_load, this function only loads the chunk; it does not run it. 

载入文件作为一个Lua单元。本函数用lua_load来载入名为filename的文件中的单元。如果filename是NULL，则从标准输入载入。文件中的第一行如果以#开头则被忽略。 

本函数返回同lua_load一样的结果，除了有个额外的错误代码LUA_ERRFILE，用于不能打开/读取文件的情况。 

同lua_load一样，本函数只载入单元；不会运行它。 
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

载入缓冲器并作为一个Lua单元。本函数用lua_load来加载缓冲器中由buff指向且长度为sz的单元。 

本函数返回同lua_load一样的结果。name是单元名字，用于调试信息和错误消息。 
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

载入字符串作为一个Lua单元。本函数用lua_load来载入以0结尾的字符串s中的单元。 

本函数返回同lua_load一样的结果。 

本函数只载入单元，这也同lua_load一样；不会运行它。 
*/
LUALIB_API int (luaL_loadstring) (lua_State *L, const char *s);

/**
luaL_newstate

[-0, +0, -] 

lua_State *luaL_newstate (void);

Creates a new Lua state. It calls lua_newstate with an allocator based on the standard C realloc function and then sets a panic function (see lua_atpanic) that prints an error message to the standard error output in case of fatal errors. 

Returns the new state, or NULL if there is a memory allocation error. 

创建新的Lua状态机。它用基于标准C的realloc函数的分配器调用lua_newstate，然后设置一个在发生重大错误时向标准错误输出打印一条错误消息的应急函数（见lua_atpanic）。 

返回新的状态机，如果发生内存分配错误则返回NULL。 
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

通过把出现的任何字符串p替换为字符串r来创建字符串s的拷贝。把结果字符串压栈并返回它。 
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
  
  检查条件是否为真。

  如果不是，用下面的信息引发一个错误，其中func是从调用堆栈中重新取得。

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

 检查函数参数narg是否字符串并返回它。 

 本函数用lua_tolstring取得其结果，所以那个函数的所有变换和警告也适用于此处。 
 */
#define luaL_checkstring(L,n)	(luaL_checklstring(L, (n), NULL))

/**
 luaL_optstring

 [-0, +0, v] 

 const char *luaL_optstring (lua_State *L,
 								int narg,
 								const char *d);

 If the function argument narg is a string, returns this string. If this argument is absent or is nil, returns d. Otherwise, raises an error. 

 如果函数参数narg是字符串则返回它。如果该参数不存在或为nil则返回d。否则引发错误。 
 */
#define luaL_optstring(L,n,d)	(luaL_optlstring(L, (n), (d), NULL))

/**
 luaL_checkint

 [-0, +0, v] 

 int luaL_checkint (lua_State *L, int narg);

 Checks whether the function argument narg is a number and returns this number cast to an int. 

 检查函数参数narg是否为一个数，并且把这个数转换为整数然后返回。
 */
#define luaL_checkint(L,n)	((int)luaL_checkinteger(L, (n)))

/**
 luaL_optint

 [-0, +0, v] 

 int luaL_optint (lua_State *L, int narg, int d);

 If the function argument narg is a number, returns this number cast to an int. If this argument is absent or is nil, returns d. Otherwise, raises an error. 

 如果函数参数narg是数字，则把该数字转型为int返回。如果该参数不存在或为nil则返回d。否则引发错误。 
 */
#define luaL_optint(L,n,d)	((int)luaL_optinteger(L, (n), (d)))

/**
 luaL_checklong

 [-0, +0, v] 

 long luaL_checklong (lua_State *L, int narg);

 Checks whether the function argument narg is a number and returns this number cast to a long. 

 检查函数参数narg是否是一个数，并且把这个数转换为长整型然后返回。
 */
#define luaL_checklong(L,n)	((long)luaL_checkinteger(L, (n)))

/**
 luaL_optlong

 [-0, +0, v] 

 long luaL_optlong (lua_State *L, int narg, long d);

 If the function argument narg is a number, returns this number cast to a long. If this argument is absent or is nil, returns d. Otherwise, raises an error. 

 如果函数参数narg是数字，则把该数字转型为long返回。如果该参数不存在或为nil则返回d。否则引发错误。 
 */
#define luaL_optlong(L,n,d)	((long)luaL_optinteger(L, (n), (d)))

/**
 luaL_typename

 [-0, +0, -] 

 const char *luaL_typename (lua_State *L, int index);

 Returns the name of the type of the value at the given index. 

 返回给定索引处的值的类型名。 
 */
#define luaL_typename(L,i)	lua_typename(L, lua_type(L,(i)))

/**
 luaL_dofile

 [-0, +?, m] 

 int luaL_dofile (lua_State *L, const char *filename);

 Loads and runs the given file. It is defined as the following macro: 

 (luaL_loadfile(L, filename) || lua_pcall(L, 0, LUA_MULTRET, 0))

 It returns 0 if there are no errors or 1 in case of errors. 

 加载并运行所给的文件，定义为如下宏

 (luaL_loadfile(L, filename) || lua_pcall(L, 0, LUA_MULTRET, 0))

 如果没有错误则返回0，如果出错返回1。
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

 加载并运行所给的字符串。定义为如下宏：

 (luaL_loadstring(L, str) || lua_pcall(L, 0, LUA_MULTRET, 0))

 如果没有错误则返回0，如果出错返回1。
 */
#define luaL_dostring(L, s) \
	(luaL_loadstring(L, s) || lua_pcall(L, 0, LUA_MULTRET, 0))

/**
  luaL_getmetatable

  [-0, +1, -] 

  void luaL_getmetatable (lua_State *L, const char *tname);

  Pushes onto the stack the metatable associated with name tname in the registry (see luaL_newmetatable). 

  把与注册表中的名字tname相关联的元表压栈。（见luaL_newmetatable）。 
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

  字符串缓冲器类型。 

  字符串缓冲器允许C代码逐段地构建Lua字符串。其使用模式如下： 

  首先声明luaL_Buffer类型的变量b。 

  接着调用luaL_buffinit(L, &b)初始化它。 

  然后调用luaL_add*函数把字符串片断添加到缓冲器。 

  通过调用luaL_pushresult(&b)结束。该调用把最终的字符串放在栈顶。 

  在字符串缓冲器的常规操作期间，它占用的栈槽数量不固定。所以，使用缓冲器时，你不能假定知道栈顶在哪儿。只要用法是对称的，你就能在连续的对缓冲器操作的调用之间使用栈；就是说，当你调用缓冲器操作时，栈处于前一个缓冲器操作后立刻得到的水平。（该规则的唯一例外是luaL_addvalue。）调用luaL_pushresult之后，栈回到缓冲器被初始化时的水平，加上栈顶的最终字符串。 
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
 
    把字符c加入缓冲区B（参考luaL_Buffer）。
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

  	把前一个复制到缓冲区（参考luaL_prepbuffer）的长度n的字符串加入缓冲B（参考luaL_Buffer）
 */
#define luaL_addsize(B,n)	((B)->p += (n))

/**
luaL_buffinit

[-0, +0, -] 

void luaL_buffinit (lua_State *L, luaL_Buffer *B);

Initializes a buffer B. This function does not allocate any space; the buffer must be declared as a variable (see luaL_Buffer). 

初始化缓冲器B。本函数不分配任何空间；缓冲器必须已被声明为变量（见luaL_Buffer）。 
*/
LUALIB_API void (luaL_buffinit) (lua_State *L, luaL_Buffer *B);

/**
luaL_prepbuffer

[-0, +0, -] 

char *luaL_prepbuffer (luaL_Buffer *B);

Returns an address to a space of size LUAL_BUFFERSIZE where you can copy a string to be added to buffer B (see luaL_Buffer). After copying the string into this space you must call luaL_addsize with the size of the string to actually add it to the buffer. 

返回尺寸为LUAL_BUFFERSIZE的空间的地址，你能把要被加入缓冲器B的字符串拷贝到其中（见luaL_Buffer）。在把字符串拷贝到该空间中以后，你必须用字符串的尺寸调用luaL_addsize来把它加入缓冲器中。 
*/
LUALIB_API char *(luaL_prepbuffer) (luaL_Buffer *B);

/**
luaL_addlstring

[-0, +0, m] 

void luaL_addlstring (luaL_Buffer *B, const char *s, size_t l);

Adds the string pointed to by s with length l to the buffer B (see luaL_Buffer). The string may contain embedded zeros. 

把指向s的长度l的字符串加入缓冲区B（参考luaL_Buffer）。

这个字符串可以包含嵌入的0。
*/
LUALIB_API void (luaL_addlstring) (luaL_Buffer *B, const char *s, size_t l);

/**
luaL_addstring

[-0, +0, m] 

void luaL_addstring (luaL_Buffer *B, const char *s);

Adds the zero-terminated string pointed to by s to the buffer B (see luaL_Buffer). The string may not contain embedded zeros. 

把s指向的0结尾的字符串添加到缓冲器B（见luaL_Buffer）。字符串不可包含内嵌的0。 
*/
LUALIB_API void (luaL_addstring) (luaL_Buffer *B, const char *s);

/**
luaL_addvalue

[-1, +0, m] 

void luaL_addvalue (luaL_Buffer *B);

Adds the value at the top of the stack to the buffer B (see luaL_Buffer). Pops the value. 

This is the only function on string buffers that can (and must) be called with an extra element on the stack, which is the value to be added to the buffer. 

把栈顶的值添加到缓冲器B（见luaL_Buffer）。弹出该值。 

这是仅有能（且必须）用栈上的一个额外元素调用的关于字符串缓冲器的函数，该元素是要被添加到缓冲器的值。 
*/
LUALIB_API void (luaL_addvalue) (luaL_Buffer *B);

/**
luaL_pushresult

[-?, +1, m] 

void luaL_pushresult (luaL_Buffer *B);

Finishes the use of buffer B leaving the final string on the top of the stack. 

结束对缓冲器B的使用，把最终字符串留在栈顶。 
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


