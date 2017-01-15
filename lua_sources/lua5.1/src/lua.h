/*
** $Id: lua.h,v 1.218.1.5 2008/08/06 13:30:12 roberto Exp $
** Lua - An Extensible Extension Language
** Lua.org, PUC-Rio, Brazil (http://www.lua.org)
** See Copyright Notice at the end of this file
*/

//lua_rawequal

#ifndef lua_h
#define lua_h

#include <stdarg.h>
#include <stddef.h>


#include "luaconf.h"

/** @mainpage

  @section intro Introduction
  
  @section api API Reference
 */

#define LUA_VERSION	"Lua 5.1"
#define LUA_RELEASE	"Lua 5.1.4"
#define LUA_VERSION_NUM	501
#define LUA_COPYRIGHT	"Copyright (C) 1994-2008 Lua.org, PUC-Rio"
#define LUA_AUTHORS 	"R. Ierusalimschy, L. H. de Figueiredo & W. Celes"


/* mark for precompiled code (`<esc>Lua') */
#define	LUA_SIGNATURE	"\033Lua"

/* option for multiple returns in `lua_pcall' and `lua_call' */
#define LUA_MULTRET	(-1)


/*
** pseudo-indices
*/
#define LUA_REGISTRYINDEX	(-10000)
#define LUA_ENVIRONINDEX	(-10001)
#define LUA_GLOBALSINDEX	(-10002)
#define lua_upvalueindex(i)	(LUA_GLOBALSINDEX-(i))


/* thread status; 0 is OK */
#define LUA_YIELD	1
#define LUA_ERRRUN	2
#define LUA_ERRSYNTAX	3
#define LUA_ERRMEM	4
#define LUA_ERRERR	5


typedef struct lua_State lua_State;

/**
lua_CFunction

typedef int (*lua_CFunction) (lua_State *L);

Type for C functions. 

In order to communicate properly with Lua, a C function must use the following protocol, which defines the way parameters and results are passed: a C function receives its arguments from Lua in its stack in direct order (the first argument is pushed first). So, when the function starts, lua_gettop(L) returns the number of arguments received by the function. The first argument (if any) is at index 1 and its last argument is at index lua_gettop(L). To return values to Lua, a C function just pushes them onto the stack, in direct order (the first result is pushed first), and returns the number of results. Any other value in the stack below the results will be properly discarded by Lua. Like a Lua function, a C function called by Lua can also return many results. 

As an example, the following function receives a variable number of numerical arguments and returns their average and sum: 

static int foo (lua_State *L) {

int n = lua_gettop(L);     number of arguments 

lua_Number sum = 0;

int i;

for (i = 1; i <= n; i++) {

if (!lua_isnumber(L, i)) {

lua_pushstring(L, "incorrect argument");

lua_error(L);

}

sum += lua_tonumber(L, i);

}

lua_pushnumber(L, sum/n);         first result 

lua_pushnumber(L, sum);          second result 

return 2;                    number of results 

}

����C���������͡� 

Ϊ����Luaǡ����ͨѶ��C��������ʹ�������Э�飬�������˲����ͽ���Ĵ��ݷ�ʽ��C��������ջ����˳��ķ�ʽ����һ����������ѹջ����������Lua�Ĳ��������ԣ���������ʼʱ��lua_gettop(L)���غ����յ��Ĳ�����������һ������������ڣ�������1�������Ĳ���������lua_gettop(L)����Ҫ��Lua����ֵ��C����ֻ��Ҫ������˳��ѹջ����һ�����������ѹջ���������ؽ���ĸ�����ջ�н��������κ�����ֵ����Luaǡ���ض�����ͬLua����һ������Lua���õ�C����Ҳ�ܷ��ض������� 

��Ϊ���ӣ�����ĺ������տɱ����������ֲ��������������ǵ�ƽ�������ܺͣ� 

static int foo (lua_State *L) {

int n = lua_gettop(L);     �����ĸ��� 

lua_Number sum = 0;

int i;

for (i = 1; i <= n; i++) {

if (!lua_isnumber(L, i)) {

lua_pushstring(L, "incorrect argument");

lua_error(L);

}

sum += lua_tonumber(L, i);

}

lua_pushnumber(L, sum/n);    ��1������ 

lua_pushnumber(L, sum);      ��2������ 

return 2;                    ����ĸ��� 

}
*/
typedef int (*lua_CFunction) (lua_State *L);

/*
** functions that read/write blocks when loading/dumping Lua chunks
*/
/**
lua_Reader

typedef const char * (*lua_Reader) (lua_State *L,
                                    void *data,
                                    size_t *size);

The reader function used by lua_load. Every time it needs another piece of the chunk, lua_load calls the reader, passing along its data parameter. The reader must return a pointer to a block of memory with a new piece of the chunk and set size to the block size. The block must exist until the reader function is called again. To signal the end of the chunk, the reader must return NULL or set size to zero. The reader function may return pieces of any size greater than zero. 

��lua_loadʹ�õĶ�ȡ��������ÿ����Ҫ��Ԫ����һ��ʱ��lua_load��������data�������ö�ȡ������ȡ�����뷵��һ�����ڴ���ָ�룬���к��е�Ԫ���´���Σ���������sizeΪ��ߴ硣�����һֱ����ֱ���ٴε��ö�ȡ����������ȡ�����뷵��NULL������sizeΪ0��ָʾ��Ԫ��������ȡ���������ܷ��ش���0���κγߴ�Ĵ���Ρ� 
*/
typedef const char * (*lua_Reader) (lua_State *L, void *ud, size_t *sz);

/**
lua_Writer
	
typedef int (*lua_Writer) (lua_State *L,
							   const void* p,
							   size_t sz,
							   void* ud);

The type of the writer function used by lua_dump. Every time it produces another piece of chunk, lua_dump calls the writer, passing along the buffer to be written (p), its size (sz), and the data parameter supplied to lua_dump. 

The writer returns an error code: 0 means no errors; any other value means an error and stops lua_dump from calling the writer again. 

��lua_dumpʹ�õļ�¼�����������͡�lua_dump����Ҫ��д��Ļ�������p�����ñ�������ͬʱ���뻺�����ߴ磨sz�����ṩ��lua_dump��data������ÿ�ε��û������Ԫ����һ�δ��롣 

��¼�����ش�����룺0��ʾû�����κ�����ֵ��ʾ������ֹlua_dump�ٴε��ü�¼���� 
*/
typedef int (*lua_Writer) (lua_State *L, const void* p, size_t sz, void* ud);

/**
 ** prototype for memory-allocation functions 
 */
/**
lua_Alloc
typedef void * (*lua_Alloc) (void *ud,
                             void *ptr,
                             size_t osize,
                             size_t nsize);

The type of the memory-allocation function used by Lua states. The allocator function must provide a functionality similar to realloc, but not exactly the same. Its arguments are ud, an opaque pointer passed to lua_newstate; ptr, a pointer to the block being allocated/reallocated/freed; osize, the original size of the block; nsize, the new size of the block. ptr is NULL if and only if osize is zero. When nsize is zero, the allocator must return NULL; if osize is not zero, it should free the block pointed to by ptr. When nsize is not zero, the allocator returns NULL if and only if it cannot fill the request. When nsize is not zero and osize is zero, the allocator should behave like malloc. When nsize and osize are not zero, the allocator behaves like realloc. Lua assumes that the allocator never fails when osize >= nsize. 

Here is a simple implementation for the allocator function. It is used in the auxiliary library by luaL_newstate. 

     static void *l_alloc (void *ud, void *ptr, size_t osize,
                                                size_t nsize) {

(void)ud;  (void)osize; 

if (nsize == 0) {

free(ptr);

return NULL;

}

else

return realloc(ptr, nsize);

}

This code assumes that free(NULL) has no effect and that realloc(NULL, size) is equivalent to malloc(size). ANSI C ensures both behaviors. 

�������͵��ڴ���亯����Lua״̬��ʹ�á����������������ṩ����realloc�Ĺ��ܣ����ǲ�����ȫһ�������Ĳ�����ud��һ����lua_newstate����Ĳ�͸��ָ�룻ptr��һ��ָ�򼴽�������/�ط���/�ͷŵ��ڴ���ָ�룻osize���ڴ��ԭ���ĳߴ磻nsize���ڴ����³ߴ硣���ҽ���osize��0ʱptrΪNULL����nsize��0ʱ�����������뷵��NULL�����osize��0���������ͷ�ptrָ����ڴ�顣��nsize��0ʱ�����ҽ���������������������ʱ����NULL����nsize��0��osize��0ʱ��������Ӧ�ñ��ֵ�����malloc����nsize��osize��0ʱ�����������ֵ�����realloc��Lua�ٶ���osize >= nsizeʱ����������ʧ�ܡ� 

����и������������ļ�ʵ�֡������������еı�luaL_newstateʹ�á� 

static void *l_alloc (void *ud, void *ptr, size_t osize,
                                                size_t nsize) {

(void)ud;  (void)osize;

if (nsize == 0) {

free(ptr);

return NULL;

}

else

return realloc(ptr, nsize);

}

�ô���ٶ�free(NULL)�������ö���realloc(NULL, size)�ȼ���malloc(size)��ANSI Cȷ����������Ϊ��
*/
typedef void * (*lua_Alloc) (void *ud, void *ptr, size_t osize, size_t nsize);


/**
 ** basic types
 */
#define LUA_TNONE		(-1)

#define LUA_TNIL		0
#define LUA_TBOOLEAN		1
#define LUA_TLIGHTUSERDATA	2
#define LUA_TNUMBER		3
#define LUA_TSTRING		4
#define LUA_TTABLE		5
#define LUA_TFUNCTION		6
#define LUA_TUSERDATA		7
#define LUA_TTHREAD		8



/* minimum Lua stack available to a C function */
#define LUA_MINSTACK	20


/*
** generic extra include file
*/
#if defined(LUA_USER_H)
#include LUA_USER_H
#endif

/**
lua_Number

typedef double lua_Number;

The type of numbers in Lua. By default, it is double, but that can be changed in luaconf.h. 

Through the configuration file you can change Lua to operate with another type for numbers (e.g., float or long). 

Lua�е��������͡�ȱʡ��˫���ȸ���������������luaconf.h�иı䡣 

ͨ�������ļ��ܸı�Luaȥ���������������������֣����絥���ȸ����������ͣ��� 
*/
/* type of numbers in Lua */
typedef LUA_NUMBER lua_Number;

/* type for integer functions */
/**
lua_Integer

typedef ptrdiff_t lua_Integer;

The type used by the Lua API to represent integral values. 

By default it is a ptrdiff_t, which is usually the largest signed integral type the machine handles "comfortably". 

��Lua API������ʾ����ֵ�����͡� 

ȱʡ��ptrdiff_t����ͨ���ǻ����ܴ�������Ĵ��������͡� 
*/
typedef LUA_INTEGER lua_Integer;



/*
** state manipulation
*/

/**
lua_newstate

[-0, +0, -] 

lua_State *lua_newstate (lua_Alloc f, void *ud);

Creates a new, independent state. Returns NULL if cannot create the state (due to lack of memory). The argument f is the allocator function; Lua does all memory allocation for this state through this function. The second argument, ud, is an opaque pointer that Lua simply passes to the allocator in every call. 

����һ���µģ�������״̬��

����޷�����״̬�򷵻�NULL����Ϊȱ���ڴ棩��

����f�Ƿ�����������Luaͨ���������Ϊ���״ִ̬�������ڴ���䡣

�ڶ�������ud��һ����͸����ָ�롣Lua�򵥵���ÿ�ε���ʱ�������ݸ�������������
*/
LUA_API lua_State *(lua_newstate) (lua_Alloc f, void *ud);

/**
lua_close

[-0, +0, -] 

void lua_close (lua_State *L);

Destroys all objects in the given Lua state (calling the corresponding garbage-collection metamethods, if any) and frees all dynamic memory used by this state. On several platforms, you may not need to call this function, because all resources are naturally released when the host program ends. On the other hand, long-running programs, such as a daemon or a web server, might need to release states as soon as they are not needed, to avoid growing too large. 

���ٸ���Lua״̬���е�ȫ������������ڶ�Ӧ�������ռ�Ԫ������������ǣ������ͷŸ�״̬��ռ�õ����ж�̬�ڴ档��һЩƽ̨�ϣ�����ܲ���Ҫ���ñ���������Ϊ�������������ʱ��������Դ��Ȼ�ر��ͷš���һ���棬�������еĳ��򣬱����̨����daemon����web��������������Ҫ��״̬��������Ҫʱ�����ͷ����ǣ��Ա����������� 
*/
LUA_API void       (lua_close) (lua_State *L);

/**
lua_newthread

[-0, +1, m] 

lua_State *lua_newthread (lua_State *L);

Creates a new thread, pushes it on the stack, and returns a pointer to a lua_State that represents this new thread. The new state returned by this function shares with the original state all global objects (such as tables), but has an independent execution stack. 

There is no explicit function to close or to destroy a thread. Threads are subject to garbage collection, like any Lua object. 

�������̣߳�����ѹջ��������ָ��lua_State��ָ�룬����ʾ�����̡߳����������ص���״̬�����ʼ״̬����������ȫ�ֶ���������������ж�����ִ��ջ�� 

û�йرջ������̵߳���ʽ���������κ�Lua����һ�����߳��������ռ���֧�䡣 
*/
LUA_API lua_State *(lua_newthread) (lua_State *L);

/**
lua_atpanic

[-0, +0, -]

lua_CFunction lua_atpanic (lua_State *L, lua_CFunction panicf);

Sets a new panic function and returns the old one. 

If an error happens outside any protected environment, Lua calls a panic function and then calls exit(EXIT_FAILURE), thus exiting the host application. Your panic function can avoid this exit by never returning (e.g., doing a long jump). 

The panic function can access the error message at the top of the stack. 

�����µ�Ӧ����panic������������ǰһ���� 

������κ��ܱ����Ļ������淢���˴���Lua����Ӧ���������ŵ���exit(EXIT_FAILURE)���Ӷ��˳������������Ӧ��������ͨ���������أ�����ִ��һ�γ���ת���Ա�������˳��� 

Ӧ�������ɷ���ջ���Ĵ�����Ϣ�� 
*/
LUA_API lua_CFunction (lua_atpanic) (lua_State *L, lua_CFunction panicf);


/*
** basic stack manipulation
*/

/**
lua_gettop

[-0, +0, -] 

int lua_gettop (lua_State *L);

Returns the index of the top element in the stack. Because indices start at 1, this result is equal to the number of elements in the stack (and so 0 means an empty stack). 

int lua_gettop (lua_State *L);

����ջ��Ԫ�ص���������Ϊ������1��ʼ���ý������ջ��Ԫ�ص�����������0��ʾ��ջ���� 
*/
LUA_API int   (lua_gettop) (lua_State *L);

/**
lua_settop

[-?, +?, -] 

void lua_settop (lua_State *L, int index);

Accepts any acceptable index, or 0, and sets the stack top to this index. If the new top is larger than the old one, then the new elements are filled with nil. If index is 0, then all stack elements are removed. 

��������ɽ��ܵ�����������0����ջ�������ڴ������ϡ�

����µ�ջ������ԭ�еģ���ô�´�����Ԫ�ر����Ϊ�ա�

�������Ϊ0����ô���ж�ջԪ�ض��ᱻɾ���� 
*/
LUA_API void  (lua_settop) (lua_State *L, int idx);

/**
lua_pushvalue

[-0, +1, -] 

void lua_pushvalue (lua_State *L, int index);

Pushes a copy of the element at the given valid index onto the stack. 

�������Ϸ�index����Ԫ�صĿ���ѹ��ջ�ڡ�
*/
LUA_API void  (lua_pushvalue) (lua_State *L, int idx);

/**
lua_remove

[-1, +0, -] 

void lua_remove (lua_State *L, int index);

Removes the element at the given valid index, shifting down the elements above this index to fill the gap. Cannot be called with a pseudo-index, because a pseudo-index is not an actual stack position. 

�Ƴ���������Ч��������Ԫ�أ����������������Ԫ������������϶��������α�������ã���Ϊα����������ʵ��ջλ�á� 
*/
LUA_API void  (lua_remove) (lua_State *L, int idx);

/**
lua_insert

[-1, +1, -] 

void lua_insert (lua_State *L, int index);

Moves the top element into the given valid index, shifting up the elements above this index to open space. Cannot be called with a pseudo-index, because a pseudo-index is not an actual stack position. 

��ջ��Ԫ�������������Ч���������������������Ԫ�����������ſռ䡣������α�������ã���Ϊα����������ʵ��ջλ�á� 
*/
LUA_API void  (lua_insert) (lua_State *L, int idx);

/**
lua_replace

[-1, +0, -] 

void lua_replace (lua_State *L, int index);

Moves the top element into the given position (and pops it), without shifting any element (therefore replacing the value at the given position). 

��ջ��Ԫ���ƶ��������������У������������������ƶ��κ�Ԫ�أ�����滻����λ�õ�ֵ���� 
*/
LUA_API void  (lua_replace) (lua_State *L, int idx);

/**
lua_checkstack

[-0, +0, m] 

int lua_checkstack (lua_State *L, int extra);

Ensures that there are at least extra free stack slots in the stack. It returns false if it cannot grow the stack to that size. This function never shrinks the stack; if the stack is already larger than the new size, it is left unchanged. 

ȷ��ջ�д�������extra������ջ��λ�����ջ�����������Ǹ��ߴ��򷵻ؼ١��������Ӳ���Сջ�����ջ�Ѿ����³ߴ�����ޱ仯�� 
*/
LUA_API int   (lua_checkstack) (lua_State *L, int sz);

/**
lua_xmove

[-?, +?, -] 

void lua_xmove (lua_State *from, lua_State *to, int n);

Exchange values between different threads of the same global state. 

This function pops n values from the stack from, and pushes them onto the stack to. 

������ͬȫ��״̬�в�ͬ�̵߳�ֵ��

��������Ӷ�ջfrom�е���n��ֵ��Ȼ�������ѹ���ջto��
*/
LUA_API void  (lua_xmove) (lua_State *from, lua_State *to, int n);


/*
** access functions (stack -> C)
*/

/**
lua_isnumber

[-0, +0, -] 

int lua_isnumber (lua_State *L, int index);

Returns 1 if the value at the given acceptable index is a number or a string convertible to a number, and 0 otherwise. 

��������ɽ�����������ֵ�������߿�ת��Ϊ�����ַ����򷵻�1�����򷵻�0��
*/
LUA_API int             (lua_isnumber) (lua_State *L, int idx);

/**
lua_isstring

[-0, +0, -] 

int lua_isstring (lua_State *L, int index);

Returns 1 if the value at the given acceptable index is a string or a number (which is always convertible to a string), and 0 otherwise. 

��������ɽ�����������ֵ���ַ��������������ǿ���ת��Ϊ�ַ������򷵻�1�����򷵻�0��
*/
LUA_API int             (lua_isstring) (lua_State *L, int idx);

/**
lua_iscfunction

[-0, +0, -] 

int lua_iscfunction (lua_State *L, int index);

Returns 1 if the value at the given acceptable index is a C function, and 0 otherwise. 

��������ɽ�����������ֵΪC�����򷵻�1�����򷵻�0��
*/
LUA_API int             (lua_iscfunction) (lua_State *L, int idx);

/*
lua_isuserdata

[-0, +0, -] 

int lua_isuserdata (lua_State *L, int index);

Returns 1 if the value at the given acceptable index is a userdata (either full or light), and 0 otherwise. 

��������ɽ�����������ֵ��һ��userdata����ȫ�������������򷵻�1�����򷵻�0��
*/
LUA_API int             (lua_isuserdata) (lua_State *L, int idx);

/**
lua_type

[-0, +0, -] 

int lua_type (lua_State *L, int index);

Returns the type of the value in the given acceptable index, or LUA_TNONE for a non-valid index (that is, an index to an "empty" stack position). The types returned by lua_type are coded by the following constants defined in lua.h: LUA_TNIL, LUA_TNUMBER, LUA_TBOOLEAN, LUA_TSTRING, LUA_TTABLE, LUA_TFUNCTION, LUA_TUSERDATA, LUA_TTHREAD, and LUA_TLIGHTUSERDATA. 

���ظ������Ͽɵ���������ֵ�����ͣ����߶Բ��Ϸ�����������LUA_TNONE����ָ�򡰿ա�ջλ�õ���������lua_type���ص�������lua.h�ж��壬������Ϊ����ĳ����� LUA_TNIL��LUA_TNUMBER��LUA_TBOOLEAN��LUA_TSTRING��LUA_TTABLE��LUA_TFUNCTION��LUA_TUSERDATA��LUA_TTHREAD��LUA_TLIGHTUSERDATA�� 
*/
LUA_API int             (lua_type) (lua_State *L, int idx);

/**
lua_typename

[-0, +0, -] 

const char *lua_typename  (lua_State *L, int tp);

Returns the name of the type encoded by the value tp, which must be one the values returned by lua_type. 

������ֵtp�������������tp������lua_type�ķ���ֵ������һ���� 
*/
LUA_API const char     *(lua_typename) (lua_State *L, int tp);

/**
lua_equal

[-0, +0, e]

int lua_equal (lua_State *L, int index1, int index2);

Returns 1 if the two values in acceptable indices index1 and index2 are equal, following the semantics of the Lua == operator (that is, may call metamethods). Otherwise returns 0. Also returns 0 if any of the indices is non valid. 

����Lua��==�����������壨�����ܵ���Ԫ���������Ƚ��ڿɽ�������index1��index2�е�����ֵ���������򷵻�1�����򷵻�0������κ�������ЧҲ����0�� 
*/
LUA_API int            (lua_equal) (lua_State *L, int idx1, int idx2);

/**
lua_rawequal

[-0, +0, -] 

int lua_rawequal (lua_State *L, int index1, int index2);

Returns 1 if the two values in acceptable indices index1 and index2 are primitively equal (that is, without calling metamethods). Otherwise returns 0. Also returns 0 if any of the indices are non valid. 

������������յ�����index1��index2����ֵԭ����ȣ���������Ԫ���������򷵻�1��

���򣬷���0��

��������������Ϸ���Ҳ����0��
*/
LUA_API int            (lua_rawequal) (lua_State *L, int idx1, int idx2);

/**
lua_lessthan

[-0, +0, e] 

int lua_lessthan (lua_State *L, int index1, int index2);

Returns 1 if the value at acceptable index index1 is smaller than the value at acceptable index index2, following the semantics of the Lua < operator (that is, may call metamethods). Otherwise returns 0. Also returns 0 if any of the indices is non valid. 

��������ɽ�������index1����ֵС������index2����ֵ�򷵻�1����ѭLua��<����������壨�������ܵ���Ԫ��������

���򷵻�0��

��������������Ϸ�Ҳ�᷵��0��
*/
LUA_API int            (lua_lessthan) (lua_State *L, int idx1, int idx2);

/**
lua_tonumber

[-0, +0, -] 

lua_Number lua_tonumber (lua_State *L, int index);

Converts the Lua value at the given acceptable index to the C type lua_Number (see lua_Number). The Lua value must be a number or a string convertible to a number (see ��2.2.1); otherwise, lua_tonumber returns 0. 

lua_Number lua_tonumber (lua_State *L, int index);

�������ɽ�����������ֵת��ΪC����lua_Number���ο�lua_Number����

Luaֵ����Ϊһ���������ת��Ϊ�����ַ������ο���2.2.1��������lua_tonumber����0��
*/
LUA_API lua_Number      (lua_tonumber) (lua_State *L, int idx);

/**
lua_tointeger

[-0, +0, -] 

lua_Integer lua_tointeger (lua_State *L, int index);

Converts the Lua value at the given acceptable index to the signed integral type lua_Integer. The Lua value must be a number or a string convertible to a number (see ��2.2.1); otherwise, lua_tointeger returns 0. 

If the number is not an integer, it is truncated in some non-specified way. 

�������ɽ�����������ֵת��Ϊ�����ŵ�����lua_Integer��

Luaֵ������һ�������߿�תΪ�����ַ������ο���2.2.1��������lua_tointeger����0��

������������������Բ�ȷ���ķ�ʽ�����С�
*/
LUA_API lua_Integer     (lua_tointeger) (lua_State *L, int idx);

/**
lua_toboolean

[-0, +0, -] 

int lua_toboolean (lua_State *L, int index);

Converts the Lua value at the given acceptable index to a C boolean value (0 or 1). Like all tests in Lua, lua_toboolean returns 1 for any Lua value different from false and nil; otherwise it returns 0. It also returns 0 when called with a non-valid index. (If you want to accept only actual boolean values, use lua_isboolean to test the value's type.) 

�������ɽ��ܵ���������Luaֵת��ΪC�Ĳ���ֵ(0��1)��

����Lua�����в��������������κβ���false��nil��ֵlua_toboolean����1�����򷵻�0��

�����÷Ƿ���������ʱҲ����0��

�������ֻ�����ʵ�ʵĲ���ֵ��ʹ��lua_isboolean�����ֵ���ͣ�
*/
LUA_API int             (lua_toboolean) (lua_State *L, int idx);

/**
lua_tolstring

[-0, +0, m] 

const char *lua_tolstring (lua_State *L, int index, size_t *len);

Converts the Lua value at the given acceptable index to a C string. If len is not NULL, it also sets *len with the string length. The Lua value must be a string or a number; otherwise, the function returns NULL. If the value is a number, then lua_tolstring also changes the actual value in the stack to a string. (This change confuses lua_next when lua_tolstring is applied to keys during a table traversal.) 

lua_tolstring returns a fully aligned pointer to a string inside the Lua state. This string always has a zero ('\0') after its last character (as in C), but can contain other zeros in its body. Because Lua has garbage collection, there is no guarantee that the pointer returned by lua_tolstring will be valid after the corresponding value is removed from the stack. 

�������ɽ�����������ֵת��ΪC���ַ�����

���len����NULL��Ҳ��������*lenΪ�ַ������ȡ�

Luaֵ������һ���ַ������������������������NULL��

���ֵ��������ôlua_tolstring����ı��ջ��ʵ��ֵΪ�ַ�����

����lua_tolstringӦ�õ�������ļ��У����ָı�����lua_next����

lua_tolstring����һ����ȫ�����ָ��Lua״̬�ڲ����ַ�����ָ�롣

��ΪLua���������գ����Բ���֤lua_tolstring�����ص�ָ������Ӧֵ�Ӷ�ջ��ɾ�����ԺϷ���
*/
LUA_API const char     *(lua_tolstring) (lua_State *L, int idx, size_t *len);

/**
lua_objlen

[-0, +0, -] 

size_t lua_objlen (lua_State *L, int index);

Returns the "length" of the value at the given acceptable index: for strings, this is the string length; for tables, this is the result of the length operator ('#'); for userdata, this is the size of the block of memory allocated for the userdata; for other values, it is 0. 

���ظ������Ͽɵ���������ֵ�ġ����ȡ��������ַ����������䳤�ȣ����ڱ�����ȡ������������#�����Ľ���������û����ݣ�����Ϊ�������ڴ��ĳߴ磻��������������0�� 
*/
LUA_API size_t          (lua_objlen) (lua_State *L, int idx);

/**
lua_tocfunction

[-0, +0, -] 

lua_CFunction lua_tocfunction (lua_State *L, int index);

Converts a value at the given acceptable index to a C function. That value must be a C function; otherwise, returns NULL. 

�������ɽ�����������ֵת��ΪC������

�Ǹ�ֵ������C���������򷵻�NULL��
*/
LUA_API lua_CFunction   (lua_tocfunction) (lua_State *L, int idx);

/**
lua_touserdata

[-0, +0, -] 

void *lua_touserdata (lua_State *L, int index);

If the value at the given acceptable index is a full userdata, returns its block address. If the value is a light userdata, returns its pointer. Otherwise, returns NULL. 

����������Ͽɵ���������ֵ���������û����ݣ��򷵻�����ַ��

����������û����ݣ�������ָ�롣���򷵻�NULL�� 
*/
LUA_API void	       *(lua_touserdata) (lua_State *L, int idx);

/**
lua_tothread

[-0, +0, -] 

lua_State *lua_tothread (lua_State *L, int index);

Converts the value at the given acceptable index to a Lua thread (represented as lua_State*). This value must be a thread; otherwise, the function returns NULL. 

�Ѹ������Ͽɵ���������ֵת��ΪLua�̣߳���lua_State*��ʾ������ֵ����Ϊ�̣߳����򣬱���������NULL�� 
*/
LUA_API lua_State      *(lua_tothread) (lua_State *L, int idx);

/**
lua_topointer

[-0, +0, -] 

const void *lua_topointer (lua_State *L, int index);

Converts the value at the given acceptable index to a generic C pointer (void*). The value can be a userdata, a table, a thread, or a function; otherwise, lua_topointer returns NULL. Different objects will give different pointers. There is no way to convert the pointer back to its original value. 

Typically this function is used only for debug information. 

�������ɽ�����������ֵת��Ϊ���͵�Cָ�루void*����

���ֵ�������û��������ݣ����̣߳������Ǻ���������lua_topointer����NULL��

��ͬ�Ķ���������ͬ��ָ�롣

û�з������԰�ָ��ת������ԭ��ֵ��

�ر��������������ڵ�����Ϣ��
*/
LUA_API const void     *(lua_topointer) (lua_State *L, int idx);


/*
** push functions (C -> stack)
*/

/**
lua_pushnil

[-0, +1, -] 

void lua_pushnil (lua_State *L);

Pushes a nil value onto the stack. 

ѹ��һ����ֵ����ջ�� 
*/
LUA_API void  (lua_pushnil) (lua_State *L);

/**
lua_pushnumber

[-0, +1, -] 

void lua_pushnumber (lua_State *L, lua_Number n);

Pushes a number with value n onto the stack. 

ѹ��һ��ӵ��ֵn�����ֽ���ջ��
*/
LUA_API void  (lua_pushnumber) (lua_State *L, lua_Number n);

/**
lua_pushinteger

[-0, +1, -] 

void lua_pushinteger (lua_State *L, lua_Integer n);

Pushes a number with value n onto the stack. 

��һ��ֵΪn������ѹջ�� 
*/
LUA_API void  (lua_pushinteger) (lua_State *L, lua_Integer n);

/**
lua_pushlstring

[-0, +1, m] 

void lua_pushlstring (lua_State *L, const char *s, size_t len);

Pushes the string pointed to by s with size len onto the stack. Lua makes (or reuses) an internal copy of the given string, so the memory at s can be freed or reused immediately after the function returns. The string can contain embedded zeros. 

��sָ��ĳߴ�Ϊlen���ַ���ѹջ��Lua���죨�����ã������ַ������ڲ����������Ժ������غ�s���ڴ����̿ɱ��ͷŻ����á��ַ����ɺ�����Ƕ��0�� 
*/
LUA_API void  (lua_pushlstring) (lua_State *L, const char *s, size_t l);

/**
lua_pushstring

[-0, +1, m] 

void lua_pushstring (lua_State *L, const char *s);

Pushes the zero-terminated string pointed to by s onto the stack. Lua makes (or reuses) an internal copy of the given string, so the memory at s can be freed or reused immediately after the function returns. The string cannot contain embedded zeros; it is assumed to end at the first zero. 

��sָ�����0��β���ַ���ѹջ��Lua���죨�����ã������ַ������ڲ�������

���Ժ������غ�s���ڴ����̿ɱ��ͷŻ����á��ַ������ɺ�����Ƕ��0��

�ٶ������׸�0�������� 
*/
LUA_API void  (lua_pushstring) (lua_State *L, const char *s);

/**
lua_pushvfstring

[-0, +1, m] 

const char *lua_pushvfstring (lua_State *L,
								  const char *fmt,
								  va_list argp);

Equivalent to lua_pushfstring, except that it receives a va_list instead of a variable number of arguments. 

��Ч��lua_pushfstring�����˽���һ��va_list�Դ������������
*/
LUA_API const char *(lua_pushvfstring) (lua_State *L, const char *fmt,
                                                      va_list argp);

/**
lua_pushfstring

[-0, +1, m] 

const char *lua_pushfstring (lua_State *L, const char *fmt, ...);

Pushes onto the stack a formatted string and returns a pointer to this string. It is similar to the C function sprintf, but has some important differences: 

You do not have to allocate space for the result: the result is a Lua string and Lua takes care of memory allocation (and deallocation, through garbage collection). 

The conversion specifiers are quite restricted. There are no flags, widths, or precisions. The conversion specifiers can only be '%%' (inserts a '%' in the string), '%s' (inserts a zero-terminated string, with no size restrictions), '%f' (inserts a lua_Number), '%p' (inserts a pointer as a hexadecimal numeral), '%d' (inserts an int), and '%c' (inserts an int as a character). 

����ʽ�����ַ���ѹջ������ָ������ָ�롣����C����sprintf���ƣ���Ҳ��һЩ��Ҫ������ 

�㲻��ҪΪ�������ռ䣺�����Lua�ַ�����Lua���տ��ڴ���䣨�Լ�ͨ�������ռ�������䣩�� 

ת��˵�����ǳ������ޡ�û�б�ǡ���Ȼ򾫶ȡ�ת��˵����ֻ����'%%' �����ַ����в���һ��'%'����'%s' ������һ����0��β���ַ�����û�гߴ����ƣ���'%f' ������һ��lua_Number����'%p' ������һ��ָ����Ϊʮ������������'%d' ������һ��int�����Լ�'%c' ������һ��int��Ϊ�ַ����� 
*/
LUA_API const char *(lua_pushfstring) (lua_State *L, const char *fmt, ...);

/**
lua_pushcclosure

[-n, +1, m] 

void lua_pushcclosure (lua_State *L, lua_CFunction fn, int n);

Pushes a new C closure onto the stack. 

When a C function is created, it is possible to associate some values with it, thus creating a C closure (see ��3.4); these values are then accessible to the function whenever it is called. To associate values with a C function, first these values should be pushed onto the stack (when there are multiple values, the first value is pushed first). Then lua_pushcclosure is called to create and push the C function onto the stack, with the argument n telling how many values should be associated with the function. lua_pushcclosure also pops these values from the stack. 

The maximum value for n is 255. 

���µ�C�հ�ѹջ�� 

��C����������ʱ�������԰�һЩֵ���Լ������������ʹ�����C�հ�������3.4�������������ۺ�ʱ�������ã���Щֵ�Ըú������ǿɷ��ʵġ�Ҫ��ֵ��C����������������ЩֵӦ����ѹջ�����ж��ֵʱ��һ��ֵ����ѹջ����Ȼ���ò���n����lua_pushcclosure������C����������ѹջ��n����Ӧ���Ѷ���ֵ�������ú�����lua_pushcclosureҲ�Ὣ��Щֵ��ջ�е����� 

n�����ֵ��255�� 
*/
LUA_API void  (lua_pushcclosure) (lua_State *L, lua_CFunction fn, int n);

/**
lua_pushboolean

[-0, +1, -] 

void lua_pushboolean (lua_State *L, int b);

Pushes a boolean value with value b onto the stack. 

��ֵb��Ϊ����ֵѹջ�� 
*/
LUA_API void  (lua_pushboolean) (lua_State *L, int b);

/**
lua_pushlightuserdata

[-0, +1, -] 

void lua_pushlightuserdata (lua_State *L, void *p);

Pushes a light userdata onto the stack. 

Userdata represent C values in Lua. A light userdata represents a pointer. It is a value (like a number): you do not create it, it has no individual metatable, and it is not collected (as it was never created). A light userdata is equal to "any" light userdata with the same C address. 

�������û�����ѹջ�� 

��Lua���û����ݱ�ʾCֵ�������û����ݱ�ʾһ��ָ�롣���Ǹ�ֵ���������֣����㲻�ô���������û�е�����Ԫ�����������ᱻ���գ���ͬ�Ӳ�����������������ͬ��C��ַ�������û�������ȡ� 
*/
LUA_API void  (lua_pushlightuserdata) (lua_State *L, void *p);

/**
lua_pushthread

[-0, +1, -] 

int lua_pushthread (lua_State *L);

Pushes the thread represented by L onto the stack. Returns 1 if this thread is the main thread of its state. 

��L������߳�ѹ��ջ�С�

�������߳���״̬�����߳��򷵻�1��
*/
LUA_API int   (lua_pushthread) (lua_State *L);


/*
** get functions (Lua -> stack)
*/

/**
lua_gettable

[-1, +1, e] 

void lua_gettable (lua_State *L, int index);

Pushes onto the stack the value t[k], where t is the value at the given valid index and k is the value at the top of the stack. 

This function pops the key from the stack (putting the resulting value in its place). As in Lua, this function may trigger a metamethod for the "index" event (see ��2.8). 

��ֵt[k]ѹջ������t��ָ������Ч��������ֵ������k��ջ����ֵ�� 

������������ջ�������ֵ��������λ�ã���ͬLua��һ�������������ܴ������ڡ�index���¼���Ԫ����������2.8���� 
*/
LUA_API void  (lua_gettable) (lua_State *L, int idx);

/**
lua_getfield

[-0, +1, e] 

void lua_getfield (lua_State *L, int index, const char *k);

Pushes onto the stack the value t[k], where t is the value at the given valid index. As in Lua, this function may trigger a metamethod for the "index" event (see ��2.8). 

��t[k]��ֵѹջ������t�Ǹ�������Ч��������ֵ��ͬLua��һ�������������ܴ������ڡ�index���¼���Ԫ����������2.8���� 
*/
LUA_API void  (lua_getfield) (lua_State *L, int idx, const char *k);

/*
lua_rawget

[-1, +1, -] 

void lua_rawget (lua_State *L, int index);

Similar to lua_gettable, but does a raw access (i.e., without metamethods). 

������lua_gettable����ִ��ԭ������(���磬û��Ԫ������ 
*/
LUA_API void  (lua_rawget) (lua_State *L, int idx);

/**
lua_rawgeti

[-0, +1, -] 

void lua_rawgeti (lua_State *L, int index, int n);

Pushes onto the stack the value t[n], where t is the value at the given valid index. The access is raw; that is, it does not invoke metamethods. 

��t[n]��ֵѹ��ջ�У�����t�������Ϸ�Index����ֵ��

������ԭ���ģ���ִ��Ԫ����
*/
LUA_API void  (lua_rawgeti) (lua_State *L, int idx, int n);

/**
lua_createtable

[-0, +1, m] 

void lua_createtable (lua_State *L, int narr, int nrec);

Creates a new empty table and pushes it onto the stack. The new table has space pre-allocated for narr array elements and nrec non-array elements. This pre-allocation is useful when you know exactly how many elements the table will have. Otherwise you can use the function lua_newtable. 

�����µĿձ�����ѹջ���±�Ԥ����narr������Ԫ�غ�nrec��������Ԫ�صĿ��пռ䡣����ȷ�е�֪�����ɶ��ٸ�Ԫ��ʱ��Ԥ�����Ƿǳ����õġ���������ú���lua_newtable�� 
*/
LUA_API void  (lua_createtable) (lua_State *L, int narr, int nrec);

/**
lua_newuserdata

[-0, +1, m] 

void *lua_newuserdata (lua_State *L, size_t size);

This function allocates a new block of memory with the given size, pushes onto the stack a new full userdata with the block address, and returns this address. 

Userdata represent C values in Lua. A full userdata represents a block of memory. It is an object (like a table): you must create it, it can have its own metatable, and you can detect when it is being collected. A full userdata is only equal to itself (under raw equality). 

When Lua collects a full userdata with a gc metamethod, Lua calls the metamethod and marks the userdata as finalized. When this userdata is collected again then Lua frees its corresponding memory. 

�����������µĸ����ߴ���ڴ�飬�Կ��ַ�ķ�ʽ���������û�����ѹջ�������ظõ�ַ�� 

Lua�е��û����ݱ�ʾCֵ���������û����ݱ�ʾһ���ڴ档���Ǹ�������ͬ��������봴���������������Լ���Ԫ�����ҵ����ռ�ʱ�ܱ���⵽���������û�����ֻ�����Լ�������ԭʼ����ȱȽϣ��� 

��Lua��gcԪ�����ռ��������û�����ʱ��Lua���ø�Ԫ���������û����ݱ��Ϊ��ɵġ������û������ٴα��ռ�ʱ��Lua�ͷ����Ӧ���ڴ档 
*/
LUA_API void *(lua_newuserdata) (lua_State *L, size_t sz);

/**
lua_getmetatable

[-0, +(0|1), -] 

int lua_getmetatable (lua_State *L, int index);

Pushes onto the stack the metatable of the value at the given acceptable index. If the index is not valid, or if the value does not have a metatable, the function returns 0 and pushes nothing on the stack. 

���������Ͽɵ���������ֵ��Ԫ��ѹջ�����������Ч�����������ֵû��Ԫ������������0�Ҳ���ѹջ�κζ����� 
*/
LUA_API int   (lua_getmetatable) (lua_State *L, int objindex);

/**
lua_getfenv

[-0, +1, -] 

void lua_getfenv (lua_State *L, int index);

Pushes onto the stack the environment table of the value at the given index. 

��������������ֵ�Ļ�����ѹջ�� 
*/
LUA_API void  (lua_getfenv) (lua_State *L, int idx);


/*
** set functions (stack -> Lua)
*/

/**
lua_settable

[-2, +0, e] 

void lua_settable (lua_State *L, int index);

Does the equivalent to t[k] = v, where t is the value at the given valid index, v is the value at the top of the stack, and k is the value just below the top. 

This function pops both the key and the value from the stack. As in Lua, this function may trigger a metamethod for the "newindex" event (see ��2.8). 

ִ��t[k] = v�ĵȼ۲���������t�Ǹ�������Ч��������ֵ��v��ջ����ֵ��k������ջ�������ֵ�� 

������������ֵ������ջ��ͬLua��һ�������������ܴ�����newindex���¼���Ԫ����������2.8���� 
*/
LUA_API void  (lua_settable) (lua_State *L, int idx);

/**
lua_setfield

[-1, +0, e] 

void lua_setfield (lua_State *L, int index, const char *k);

Does the equivalent to t[k] = v, where t is the value at the given valid index and v is the value at the top of the stack. 

This function pops the value from the stack. As in Lua, this function may trigger a metamethod for the "newindex" event (see ��2.8). 

ִ��t[k] = v�ĵȼ۲���������t�Ǹ�������Ч��������ֵ��v��ջ����ֵ�� 

��������ջ�е���ֵ��ͬLua��һ�������������ܴ�����newindex���¼���Ԫ����������2.8���� 
*/
LUA_API void  (lua_setfield) (lua_State *L, int idx, const char *k);

/**
lua_rawset

[-2, +0, m] 

void lua_rawset (lua_State *L, int index);

Similar to lua_settable, but does a raw assignment (i.e., without metamethods). 

����lua_settable������ִ��һ��ԭ����ֵ��Ҳ���ǲ���Ԫ�������� 
*/
LUA_API void  (lua_rawset) (lua_State *L, int idx);

/**
lua_rawseti

[-1, +0, m] 

void lua_rawseti (lua_State *L, int index, int n);

Does the equivalent of t[n] = v, where t is the value at the given valid index and v is the value at the top of the stack. 

This function pops the value from the stack. The assignment is raw; that is, it does not invoke metamethods. 

ִ��t[n] = v�ĵȼ۲���������t�Ǹ�������Ч��������ֵ��v��ջ����ֵ�� 

��������ֵ��ջ����ֵ��ԭ���ģ���������Ԫ������ 
*/
LUA_API void  (lua_rawseti) (lua_State *L, int idx, int n);

/**
lua_setmetatable

[-1, +0, -] 

int lua_setmetatable (lua_State *L, int index);

Pops a table from the stack and sets it as the new metatable for the value at the given acceptable index. 

��ջ�е���һ����������Ϊ�������Ͽɵ���������ֵ����Ԫ�� 
*/
LUA_API int   (lua_setmetatable) (lua_State *L, int objindex);

/**
lua_setfenv

[-1, +0, -] 

int lua_setfenv (lua_State *L, int index);

Pops a table from the stack and sets it as the new environment for the value at the given index. If the value at the given index is neither a function nor a thread nor a userdata, lua_setfenv returns 0. Otherwise it returns 1. 

��ջ�е���һ����������Ϊ������������ֵ���»��������������������ֵ�Ȳ��Ǻ����ֲ����߳�Ҳ�����û����ݣ�lua_setfenv����0�����򷵻�1�� 
*/
LUA_API int   (lua_setfenv) (lua_State *L, int idx);


/*
** `load' and `call' functions (load and run Lua code)
*/

/**
lua_call

[-(nargs + 1), +nresults, e] 

void lua_call (lua_State *L, int nargs, int nresults);

Calls a function. 

To call a function you must use the following protocol: first, the function to be called is pushed onto the stack; then, the arguments to the function are pushed in direct order; that is, the first argument is pushed first. Finally you call lua_call; nargs is the number of arguments that you pushed onto the stack. All arguments and the function value are popped from the stack when the function is called. The function results are pushed onto the stack when the function returns. The number of results is adjusted to nresults, unless nresults is LUA_MULTRET. In this case, all results from the function are pushed. Lua takes care that the returned values fit into the stack space. The function results are pushed onto the stack in direct order (the first result is pushed first), so that after the call the last result is on the top of the stack. 

Any error inside the called function is propagated upwards (with a longjmp). 

The following example shows how the host program can do the equivalent to this Lua code: 

a = f("how", t.x, 14)

Here it is in C: 

lua_getfield(L, LUA_GLOBALSINDEX, "f");  function to be called 

lua_pushstring(L, "how");                         1st argument 

lua_getfield(L, LUA_GLOBALSINDEX, "t");    table to be indexed 

lua_getfield(L, -1, "x");         push result of t.x (2nd arg) 

lua_remove(L, -2);                   remove 't' from the stack 

lua_pushinteger(L, 14);                           3rd argument 

lua_call(L, 3, 1);      call 'f' with 3 arguments and 1 result 

lua_setfield(L, LUA_GLOBALSINDEX, "a");         set global 'a' 

Note that the code above is "balanced": at its end, the stack is back to its original configuration. This is considered good programming practice. 

����һ�������� 

Ҫ���ú�������ʹ�������Э�飺���ȣ���Ҫ�����õĺ���ѹջ��Ȼ�󣬽���������˳��ѹջ������һ��������ѹջ����󣬵���lua_call��nargs����ѹջ�Ĳ���������������������ʱ���в����Լ�����ֵ������ջ������������ʱ������ѹջ�����������Ϊnresults��������nresults��LUA_MULTRET���ڸ�����£����Ժ��������н����ѹջ��Lua��ȷ������ֵ�ʺ�ջ�ռ䡣���������˳��ѹջ����һ���������ѹջ����������ú����һ�������ջ���� 

�����ú����ڵ��κδ���ᱻ���ϴ�����ʹ��longjmp���� 

�����������ʾ������������ִ�����Lua����ȼ۵Ĳ����� 

a = f("how", t.x, 14)

����C�еĴ��룺 

lua_getfield(L, LUA_GLOBALSINDEX, "f");           Ҫ�����õĺ��� 

lua_pushstring(L, "how");                              ��1������ 

lua_getfield(L, LUA_GLOBALSINDEX, "t");             Ҫ�������ı� 

lua_getfield(L, -1, "x");            ��t.x�Ľ��ѹջ����2�������� 

lua_remove(L, -2);                   ��ջ��ɾ����t�� 

lua_pushinteger(L, 14);                                ��3������ 

lua_call(L, 3, 1);                ��3���������á�f��������1����� 

lua_setfield(L, LUA_GLOBALSINDEX, "a");         ����ȫ�ֱ�����a�� 

ע������Ĵ����ǡ��ԳƵġ��������β��ջ�ص����ʼ���á�����һ�����õı��ϰ�ߡ� 
*/
LUA_API void  (lua_call) (lua_State *L, int nargs, int nresults);

/**
lua_pcall

[-(nargs + 1), +(nresults|1), -] 

int lua_pcall (lua_State *L, int nargs, int nresults, int errfunc);

Calls a function in protected mode. 

Both nargs and nresults have the same meaning as in lua_call. If there are no errors during the call, lua_pcall behaves exactly like lua_call. However, if there is any error, lua_pcall catches it, pushes a single value on the stack (the error message), and returns an error code. Like lua_call, lua_pcall always removes the function and its arguments from the stack. 

If errfunc is 0, then the error message returned on the stack is exactly the original error message. Otherwise, errfunc is the stack index of an error handler function. (In the current implementation, this index cannot be a pseudo-index.) In case of runtime errors, this function will be called with the error message and its return value will be the message returned on the stack by lua_pcall. 

Typically, the error handler function is used to add more debug information to the error message, such as a stack traceback. Such information cannot be gathered after the return of lua_pcall, since by then the stack has unwound. 

The lua_pcall function returns 0 in case of success or one of the following error codes (defined in lua.h): 

LUA_ERRRUN: a runtime error. 

LUA_ERRMEM: memory allocation error. For such errors, Lua does not call the error handler function. 

LUA_ERRERR: error while running the error handler function. 

�ڱ���ģʽ�е��ú����� 

nargs��nresultsͬlua_call�е�����ͬ�ĺ��塣��������ڼ�û�д���

lua_pcall����Ϊ��ȫ����lua_call��Ȼ������������κδ���lua_pcall��׽����

������ֵѹջ��������Ϣ���������ش�����롣ͬlua_callһ����

lua_pcall���Ǵ�ջ���Ƴ�������������� 

���errfunc��0������ջ�Ϸ��صĴ�����Ϣ����ԭʼ�Ĵ�����Ϣ��

����,errfunc���Ǵ�������������error handler function����ջ������

����ǰʵ���У�������������α���������ڷ�������ʱ����ʱ���ú������ô�����Ϣ���ã�

�����䷵��ֵ�����ջ�ϱ�lua_pcall���ص���Ϣ�� 

���͵أ������������������������Ϣ�����Ӹ��������Ϣ������ջ���ݡ�

������Ϣ������lua_pcall���غ��Ѽ�����Ϊ��ʱ��ջ���ͷš� 

�ɹ�ʱlua_pcall��������0����������Ĵ������֮һ����lua.h�ж��壩�� 

LUA_ERRRUN: ����ʱ���� 

LUA_ERRMEM: �ڴ������󡣶����������Lua������ô������������� 

LUA_ERRERR: �����д�����������ʱ�Ĵ��� 
*/
LUA_API int   (lua_pcall) (lua_State *L, int nargs, int nresults, int errfunc);

/**
lua_cpcall

[-0, +(0|1), -] 

int lua_cpcall (lua_State *L, lua_CFunction func, void *ud);

Calls the C function func in protected mode. func starts with only one element in its stack, a light userdata containing ud. In case of errors, lua_cpcall returns the same error codes as lua_pcall, plus the error object on the top of the stack; otherwise, it returns zero, and does not change the stack. All values returned by func are discarded. 

�Ա���ģʽ����C����func��

func��ջ��ֻ��һ��Ԫ�أ��Ǹ�����ud���������û����ݡ�

��������ʱ��lua_cpcall����ͬlua_pcallһ���Ĵ�����룬�Լ�ջ���Ĵ������

���򷵻�0�Ҳ��ı�ջ��func���ص�����ֵ��������

ע������������ud��userdata��������C��������lua_touserdata()ȡ����
*/
LUA_API int   (lua_cpcall) (lua_State *L, lua_CFunction func, void *ud);

/**
lua_load

[-0, +1, -] 

int lua_load (lua_State *L,
              lua_Reader reader,
              void *data,
              const char *chunkname);

Loads a Lua chunk. If there are no errors, lua_load pushes the compiled chunk as a Lua function on top of the stack. Otherwise, it pushes an error message. The return values of lua_load are: 

0: no errors; 

LUA_ERRSYNTAX: syntax error during pre-compilation; 

LUA_ERRMEM: memory allocation error. 

This function only loads a chunk; it does not run it. 

lua_load automatically detects whether the chunk is text or binary, and loads it accordingly (see program luac). 

The lua_load function uses a user-supplied reader function to read the chunk (see lua_Reader). The data argument is an opaque value passed to the reader function. 

The chunkname argument gives a name to the chunk, which is used for error messages and in debug information (see ��3.8). 

����һ��Lua�顣

���û�д���lua_load�ѱ���õĿ���Ϊһ��Lua����ѹ��ջ����

�������Ѵ�����Ϣѹ��ջ�С�

lua_load�ķ���ֵ�У�

0: �޴���; 

LUA_ERRSYNTAX: ��Ԥ����ʱ�����﷨���� 

LUA_ERRMEM: �ڴ�������

�������ֵ�ܼ���һ���飻�������������������

lua_load�Զ��������ı���ʽ���Ƕ����Ƹ�ʽ��������Ӧ���м��أ�������luac����

lua_load����ʹ��һ���û��ṩ�Ķ�������������飨��lua_Reader����

data������һ�����ݸ������������Ĳ�͸��ֵ��

chunkname�����������������ڴ�����Ϣ�Լ�������Ϣ������3.8���� 
*/
LUA_API int   (lua_load) (lua_State *L, lua_Reader reader, void *dt,
                                        const char *chunkname);

/**
lua_dump

[-0, +0, m] 

int lua_dump (lua_State *L, lua_Writer writer, void *data);

Dumps a function as a binary chunk. Receives a Lua function on the top of the stack and produces a binary chunk that, if loaded again, results in a function equivalent to the one dumped. As it produces parts of the chunk, lua_dump calls function writer (see lua_Writer) with the given data to write them. 

The value returned is the error code returned by the last call to the writer; 0 means no errors. 

This function does not pop the Lua function from the stack. 

������ת��Ϊ�����ƴ��뵥Ԫ������ջ����Lua���������������Ƶ�Ԫ��������߱��ٴμ��أ��õ��뱻ת���ĵȼ۵ĺ�������������Ԫ�ĸ�����ʱ��lua_dump�ø�����data���ú���writer����lua_Writer����д�����ǡ� 

����ֵ�����һ�ε��ü�¼����writer�����صĴ�����룻0��ʾû�д��� 

���������ὫLua������ջ�е����� 
*/
LUA_API int (lua_dump) (lua_State *L, lua_Writer writer, void *data);


/*
** coroutine functions
*/

/**
lua_yield

[-?, +?, -] 

int lua_yield  (lua_State *L, int nresults);

Yields a coroutine. 

This function should only be called as the return expression of a C function, as follows: 

return lua_yield (L, nresults);

When a C function calls lua_yield in that way, the running coroutine suspends its execution, and the call to lua_resume that started this coroutine returns. The parameter nresults is the number of values from the stack that are passed as results to lua_resume. 

����һ��Э�� 

�������Ӧ�ý���C����return���ʽ��ʹ�ã����£�

return lua_yield (L, nresults);

��һ��C��������������lua_yield�����е�Э����ͣ����ִ�У�������ʹ��Э�����¿�ʼ��lua_resume���ô����ء�

����nresults�Ƕ�ջ�д��ݸ�lua_resume�����ֵ������
*/
LUA_API int  (lua_yield) (lua_State *L, int nresults);

/**
lua_resume

[-?, +?, -] 

int lua_resume (lua_State *L, int narg);

Starts and resumes a coroutine in a given thread. 

To start a coroutine, you first create a new thread (see lua_newthread); then you push onto its stack the main function plus any arguments; then you call lua_resume, with narg being the number of arguments. This call returns when the coroutine suspends or finishes its execution. When it returns, the stack contains all values passed to lua_yield, or all values returned by the body function. lua_resume returns LUA_YIELD if the coroutine yields, 0 if the coroutine finishes its execution without errors, or an error code in case of errors (see lua_pcall). In case of errors, the stack is not unwound, so you can use the debug API over it. The error message is on the top of the stack. To restart a coroutine, you put on its stack only the values to be passed as results from yield, and then call lua_resume. 

�����ͻָ������߳��е�һ��Э�̡� 

Ҫ����һ��Э�̣����ȴ����̣߳���lua_newthread�������Ž��������Լ��κβ���ѹ������ջ�У�Ȼ����nargָ����������������lua_resume����Э�̹�����������ʱ�õ��÷��ء���������ʱ��ջ�к���lua_yield���������ֵ�������庯�����ص�����ֵ�����Э���ж���lua_resume����LUA_YIELD�����������������򷵻�0�������ڷ�������ʱ���ش�����루��lua_pcall������������ʱ��ջδ���ͷţ����Կɶ���ʹ�õ���API��������Ϣλ��ջ����Ҫ��������Э�̣���ֻ���Ҫ��yield�����Ľ���ŵ�����ջ�ϣ�Ȼ�����lua_resume�� 
*/
LUA_API int  (lua_resume) (lua_State *L, int narg);

/**
lua_status

[-0, +0, -] 

int lua_status (lua_State *L);

Returns the status of the thread L. 

The status can be 0 for a normal thread, an error code if the thread finished its execution with an error, or LUA_YIELD if the thread is suspended. 

�����߳�L��״ֵ̬��

�߳�����ʱ״ֵ̬Ϊ0���߳������������������ִ���򷵻�һ�������룬�̹߳�����ͣ���򷵻�LUA_YIELD��
*/
LUA_API int  (lua_status) (lua_State *L);

/*
** garbage-collection function and options
*/

#define LUA_GCSTOP		0
#define LUA_GCRESTART		1
#define LUA_GCCOLLECT		2
#define LUA_GCCOUNT		3
#define LUA_GCCOUNTB		4
#define LUA_GCSTEP		5
#define LUA_GCSETPAUSE		6
#define LUA_GCSETSTEPMUL	7

/**
lua_gc

[-0, +0, e] 

int lua_gc (lua_State *L, int what, int data);

Controls the garbage collector. 

This function performs several tasks, according to the value of the parameter what: 

LUA_GCSTOP: stops the garbage collector. 

LUA_GCRESTART: restarts the garbage collector. 

LUA_GCCOLLECT: performs a full garbage-collection cycle. 

LUA_GCCOUNT: returns the current amount of memory (in Kbytes) in use by Lua. 

LUA_GCCOUNTB: returns the remainder of dividing the current amount of bytes of memory in use by Lua by 1024. 

LUA_GCSTEP: performs an incremental step of garbage collection. The step "size" is controlled by data (larger values mean more steps) in a non-specified way. If you want to control the step size you must experimentally tune the value of data. The function returns 1 if the step finished a garbage-collection cycle. 

LUA_GCSETPAUSE: sets data as the new value for the pause of the collector (see ��2.10). The function returns the previous value of the pause. 

LUA_GCSETSTEPMUL: sets data as the new value for the step multiplier of the collector (see ��2.10). The function returns the previous value of the step multiplier. 

���������ռ����� 

���������ݲ���what��ִֵ���������� 

LUA_GCSTOP: ֹͣ�����ռ����� 

LUA_GCRESTART: �������������ռ����� 

LUA_GCCOLLECT: ִ��һ�������������ռ������ڡ� 

LUA_GCCOUNT: ����Lua��ǰռ�õ��ڴ���������KbyteΪ��λ���� 

LUA_GCCOUNTB: ����Lua��ǰ���ڴ��������ֽ�����1024�������� 

LUA_GCSTEP: ִ��һ�����������ռ�������������data��δָ���ķ�ʽ���ƣ�ֵԽ����ζ�Ų���Խ�ࣩ��Ҫ���Ʋ����������ʵ���Եص���data��ֵ������ò������һ�������ռ�������������1�� 

LUA_GCSETPAUSE: ����data��Ϊ�ռ�����pause������2.10������ֵ������pause��ǰһ��ֵ�� 

LUA_GCSETSTEPMUL: ����data��Ϊ�ռ�����step multiplier������2.10������ֵ������step multiplier��ǰһ��ֵ�� 
*/
LUA_API int (lua_gc) (lua_State *L, int what, int data);


/*
** miscellaneous functions
*/

/**
lua_error

[-1, +0, v] 

int lua_error (lua_State *L);

Generates a Lua error. The error message (which can actually be a Lua value of any type) must be on the stack top. This function does a long jump, and therefore never returns. (see luaL_error). 

����һ��Lua���󡣴�����Ϣ��ʵ���Ͽ�Ϊ�κ����͵�Luaֵ��������ջ����������ִ�г���ת����˴Ӳ����ء�����luaL_error���� 
*/
LUA_API int   (lua_error) (lua_State *L);

/**
lua_next

[-1, +(2|0), e] 

int lua_next (lua_State *L, int index);

Pops a key from the stack, and pushes a key-value pair from the table at the given index (the "next" pair after the given key). If there are no more elements in the table, then lua_next returns 0 (and pushes nothing). 

A typical traversal looks like this: 

table is in the stack at index 't' 

lua_pushnil(L);   first key 

while (lua_next(L, t) != 0) {

uses 'key' (at index -2) and 'value' (at index -1) 

printf("%s - %s\n",

lua_typename(L, lua_type(L, -2)),

lua_typename(L, lua_type(L, -1)));

removes 'value'; keeps 'key' for next iteration 

lua_pop(L, 1);

}

While traversing a table, do not call lua_tolstring directly on a key, unless you know that the key is actually a string. Recall that lua_tolstring changes the value at the given index; this confuses the next call to lua_next. 

��ջ�е���һ���������Ӹ������������ı��е���һ����-ֵ�ԣ��������ġ���һ���ԣ����������û�и����Ԫ�أ���lua_next����0���Ҳ���ѹջ�κζ������� 

���͵ı����������������� 

����ջ�е�������t���� 

lua_pushnil(L);   ��һ���� 

while (lua_next(L, t) != 0) {

ʹ�á�����������-2���͡�ֵ��(����-1�� 

printf("%s - %s\n",

lua_typename(L, lua_type(L, -2)),

lua_typename(L, lua_type(L, -1)));

�Ƴ���ֵ����Ϊ�´ε������������� 

lua_pop(L, 1);

}

��������ʱ����Ҫֱ�ӶԼ�����lua_tolstring��������֪����ȷʵ���ַ���������һ�£�lua_tolstring��ı������������ֵ����������һ�ε���lua_next�� 
*/
LUA_API int   (lua_next) (lua_State *L, int idx);

/**
lua_concat

[-n, +1, e] 

void lua_concat (lua_State *L, int n);

Concatenates the n values at the top of the stack, pops them, and leaves the result at the top. If n is 1, the result is the single value on the stack (that is, the function does nothing); if n is 0, the result is the empty string. Concatenation is performed following the usual semantics of Lua (see ��2.5.4). 

����ջ����n��ֵ���������ǲ����������ջ�������n��1��

�������ջ�ϵĵ���ֵ��������ʲôҲ�����������n��0������ǿ��ַ�����

���Ӳ�������Lua�ĳ�������ִ�У�����2.5.4���� 
*/
LUA_API void  (lua_concat) (lua_State *L, int n);

/**
lua_getallocf

[-0, +0, -] 

lua_Alloc lua_getallocf (lua_State *L, void **ud);

Returns the memory-allocation function of a given state. If ud is not NULL, Lua stores in *ud the opaque pointer passed to lua_newstate. 

���ظ���״̬�����ڴ���亯�������ud��ΪNULL��Lua��lua_newstate����Ĳ�͸��ָ�����*ud�� 
*/
LUA_API lua_Alloc (lua_getallocf) (lua_State *L, void **ud);

/**
lua_setallocf

[-0, +0, -] 

void lua_setallocf (lua_State *L, lua_Alloc f, void *ud);

Changes the allocator function of a given state to f with user data ud. 

void lua_setallocf (lua_State *L, lua_Alloc f, void *ud);

�Ѹ���״̬���ķ������������ɴ��û�����ud��f�� 
*/
LUA_API void lua_setallocf (lua_State *L, lua_Alloc f, void *ud);



/* 
** ===============================================================
** some useful macros
** ===============================================================
*/

/**
lua_pop

[-n, +0, -] 

void lua_pop (lua_State *L, int n);

Pops n elements from the stack. 

��ջ�е���n��Ԫ�ء� 
*/
#define lua_pop(L,n)		lua_settop(L, -(n)-1)

/**
lua_newtable

[-0, +1, m] 

void lua_newtable (lua_State *L);

Creates a new empty table and pushes it onto the stack. It is equivalent to lua_createtable(L, 0, 0). 

�����µĿձ�����ѹջ�����ȼ���lua_createtable(L, 0, 0)�� 
*/
#define lua_newtable(L)		lua_createtable(L, 0, 0)

/**
lua_register

[-0, +0, e] 

void lua_register (lua_State *L,
					   const char *name,
					   lua_CFunction f);

Sets the C function f as the new value of global name. It is defined as a macro: 

#define lua_register(L,n,f) \

(lua_pushcfunction(L, f), lua_setglobal(L, n))

����C����fΪȫ�ֱ���name����ֵ����������Ϊ�꣺ 

#define lua_register(L,n,f) \

(lua_pushcfunction(L, f), lua_setglobal(L, n))
*/
#define lua_register(L,n,f) (lua_pushcfunction(L, (f)), lua_setglobal(L, (n)))

/**
lua_pushcfunction

[-0, +1, m] 

void lua_pushcfunction (lua_State *L, lua_CFunction f);

Pushes a C function onto the stack. This function receives a pointer to a C function and pushes onto the stack a Lua value of type function that, when called, invokes the corresponding C function. 

Any function to be registered in Lua must follow the correct protocol to receive its parameters and return its results (see lua_CFunction). 

lua_pushcfunction is defined as a macro: 

#define lua_pushcfunction(L,f)  lua_pushcclosure(L,f,0)

��C����ѹջ������������һ��ָ��C������ָ�벢��һ��function���͵�Luaֵѹջ�����������͵ĺ���������ʱ���������Ӧ��C������ 

�κ�Ҫ��Lua��ע��ĺ���������ѭ��ȷ��Э�飬�Խ����������������������lua_CFunction���� 

lua_pushcfunction������Ϊ�꣺ 

#define lua_pushcfunction(L,f)  lua_pushcclosure(L,f,0)
*/
#define lua_pushcfunction(L,f)	lua_pushcclosure(L, (f), 0)

#define lua_strlen(L,i)		lua_objlen(L, (i))

/**
lua_isfunction

[-0, +0, -] 

int lua_isfunction (lua_State *L, int index);

Returns 1 if the value at the given acceptable index is a function (either C or Lua), and 0 otherwise. 

��������ɽ�����������ֵΪ������C������Lua���򷵻�1�����򷵻�0��
*/
#define lua_isfunction(L,n)	(lua_type(L, (n)) == LUA_TFUNCTION)

/**
lua_istable

[-0, +0, -] 

int lua_istable (lua_State *L, int index);

Returns 1 if the value at the given acceptable index is a table, and 0 otherwise. 

��������ɽ�����������ֵ�Ǳ��򷵻�1�����򷵻�0��
*/
#define lua_istable(L,n)	(lua_type(L, (n)) == LUA_TTABLE)

/**
lua_islightuserdata

[-0, +0, -] 

int lua_islightuserdata (lua_State *L, int index);

Returns 1 if the value at the given acceptable index is a light userdata, and 0 otherwise. 

��������ɽ�����������ֵΪ������userdata�򷵻�1�����򷵻�0��
*/
#define lua_islightuserdata(L,n)	(lua_type(L, (n)) == LUA_TLIGHTUSERDATA)

/**
lua_isnil

[-0, +0, -] 

int lua_isnil (lua_State *L, int index);

Returns 1 if the value at the given acceptable index is nil, and 0 otherwise. 

��������ɽ�����������ֵΪnil�򷵻�1�����򷵻�0��
*/
#define lua_isnil(L,n)		(lua_type(L, (n)) == LUA_TNIL)

/**
lua_isboolean

[-0, +0, -] 

int lua_isboolean (lua_State *L, int index);

Returns 1 if the value at the given acceptable index has type boolean, and 0 otherwise. 

����������Ͽɵ���������ֵ���в��������򷵻�1�����򷵻�0�� 
*/
#define lua_isboolean(L,n)	(lua_type(L, (n)) == LUA_TBOOLEAN)

/**
lua_isthread

[-0, +0, -] 

int lua_isthread (lua_State *L, int index);

Returns 1 if the value at the given acceptable index is a thread, and 0 otherwise. 

��������ɽ�����������ֵ��һ���߳��򷵻�1�����򷵻�0��
*/
#define lua_isthread(L,n)	(lua_type(L, (n)) == LUA_TTHREAD)

/**
lua_isnone

[-0, +0, -] 

int lua_isnone (lua_State *L, int index);

Returns 1 if the given acceptable index is not valid (that is, it refers to an element outside the current stack), and 0 otherwise. 

��������ɽ�����������ֵ�����ã������õ�Ԫ���ڵ�ǰ��ջ���⣩�򷵻�1�����򷵻�0��
*/
#define lua_isnone(L,n)		(lua_type(L, (n)) == LUA_TNONE)

/**
lua_isnoneornil

[-0, +0, -] 

int lua_isnoneornil (lua_State *L, int index);

Returns 1 if the given acceptable index is not valid (that is, it refers to an element outside the current stack) or if the value at this index is nil, and 0 otherwise. 

��������ɽ�����������ֵ�����ã������õ�Ԫ���ڵ�ǰ��ջ���⣩������������ֵΪnil�򷵻�1�����򷵻�0��
*/
#define lua_isnoneornil(L, n)	(lua_type(L, (n)) <= 0)

/**
lua_pushliteral

[-0, +1, m] 

void lua_pushliteral (lua_State *L, const char *s);

This macro is equivalent to lua_pushlstring, but can be used only when s is a literal string. In these cases, it automatically provides the string length. 

����ȼ���lua_pushlstring������ֻ�ܵ�s�������ַ���ʱʹ�á�����Щ����£����Զ����ṩ�ַ������ȡ� 
*/
#define lua_pushliteral(L, s)	\
	lua_pushlstring(L, "" s, (sizeof(s)/sizeof(char))-1)

/**
lua_setglobal

[-1, +0, e] 

void lua_setglobal (lua_State *L, const char *name);

Pops a value from the stack and sets it as the new value of global name. It is defined as a macro: 

#define lua_setglobal(L,s)   lua_setfield(L, LUA_GLOBALSINDEX, s)

��ջ�е���һ��ֵ��������Ϊȫ�ֱ���name����ֵ����������Ϊ�꣺ 

#define lua_setglobal(L,s)   lua_setfield(L, LUA_GLOBALSINDEX, s)
*/
#define lua_setglobal(L,s)	lua_setfield(L, LUA_GLOBALSINDEX, (s))

/**
lua_getglobal

[-0, +1, e] 

void lua_getglobal (lua_State *L, const char *name);

Pushes onto the stack the value of the global name. It is defined as a macro: 

#define lua_getglobal(L,s)  lua_getfield(L, LUA_GLOBALSINDEX, s)

��ȫ�ֱ���name��ֵѹջ����������Ϊ�꣺ 

#define lua_getglobal(L,s)  lua_getfield(L, LUA_GLOBALSINDEX, s)
*/
#define lua_getglobal(L,s)	lua_getfield(L, LUA_GLOBALSINDEX, (s))

/**
lua_tostring

[-0, +0, m] 

const char *lua_tostring (lua_State *L, int index);

Equivalent to lua_tolstring with len equal to NULL. 

�ȼ���len����NULLʱ��lua_tolstring�� 
*/
#define lua_tostring(L,i)	lua_tolstring(L, (i), NULL)



/*
** compatibility macros and functions
*/

#define lua_open()	luaL_newstate()

#define lua_getregistry(L)	lua_pushvalue(L, LUA_REGISTRYINDEX)

#define lua_getgccount(L)	lua_gc(L, LUA_GCCOUNT, 0)

#define lua_Chunkreader		lua_Reader
#define lua_Chunkwriter		lua_Writer


/* hack */
LUA_API void lua_setlevel	(lua_State *from, lua_State *to);


/*
** {======================================================================
** Debug API
** =======================================================================
*/


/*
** Event codes
*/
#define LUA_HOOKCALL	0
#define LUA_HOOKRET	1
#define LUA_HOOKLINE	2
#define LUA_HOOKCOUNT	3
#define LUA_HOOKTAILRET 4


/*
** Event masks
*/
#define LUA_MASKCALL	(1 << LUA_HOOKCALL)
#define LUA_MASKRET	(1 << LUA_HOOKRET)
#define LUA_MASKLINE	(1 << LUA_HOOKLINE)
#define LUA_MASKCOUNT	(1 << LUA_HOOKCOUNT)

typedef struct lua_Debug lua_Debug;  /* activation record */


/* Functions to be called by the debuger in specific events */
/**
lua_Hook

typedef void (*lua_Hook) (lua_State *L, lua_Debug *ar);

Type for debugging hook functions. 

Whenever a hook is called, its ar argument has its field event set to the specific event that triggered the hook. Lua identifies these events with the following constants: LUA_HOOKCALL, LUA_HOOKRET, LUA_HOOKTAILRET, LUA_HOOKLINE, and LUA_HOOKCOUNT. Moreover, for line events, the field currentline is also set. To get the value of any other field in ar, the hook must call lua_getinfo. For return events, event can be LUA_HOOKRET, the normal value, or LUA_HOOKTAILRET. In the latter case, Lua is simulating a return from a function that did a tail call; in this case, it is useless to call lua_getinfo. 

While Lua is running a hook, it disables other calls to hooks. Therefore, if a hook calls back Lua to execute a function or a chunk, this execution occurs without any calls to hooks. 

���Ե�hook�������͡� 

��hook������ʱ����ar���������ֶ�event��Ϊ������hook���ض��¼���Lua������ĳ�����ʶ��Щ�¼���LUA_HOOKCALL��LUA_HOOKRET��LUA_HOOKTAILRET��LUA_HOOKLINE��LUA_HOOKCOUNT�����⣬�������¼���Ҳ�������ֶ�currentline��Ҫ�õ�ar�������ֶε�ֵ��hook�������lua_getinfo�����ڷ����¼���event��������ֵͨLUA_HOOKRET��LUA_HOOKTAILRET�����ں��ߣ�Lua��ģ���ִ����β���õĺ����еķ��أ���������£�����lua_getinfo�����õġ� 

��Lua����hookʱ������ֹ������hook���á���ˣ����hook�ص�Lua��ִ�к�����Ԫ���ô�ִ�в����κ�hook���á� 
*/
typedef void (*lua_Hook) (lua_State *L, lua_Debug *ar);

/**
lua_getstack

[-0, +0, -] 

int lua_getstack (lua_State *L, int level, lua_Debug *ar);

Get information about the interpreter runtime stack. 

This function fills parts of a lua_Debug structure with an identification of the activation record of the function executing at a given level. Level 0 is the current running function, whereas level n+1 is the function that has called level n. When there are no errors, lua_getstack returns 1; when called with a level greater than the stack depth, it returns 0. 

�õ����ڽ���������ʱ��ջ��Ϣ�� 

��������ִ���ڸ�������ĺ����Ļ��¼�ı�ʶ����䲿��lua_Debug�ṹ��0���ǵ�ǰ���еĺ�������n+1���ǵ���n���ĺ������޴���ʱlua_getstack����1�����Գ���ջ��ȵļ������ʱ����0�� 
*/
LUA_API int lua_getstack (lua_State *L, int level, lua_Debug *ar);

/*
lua_getinfo

[-(0|1), +(0|1|2), m] 

int lua_getinfo (lua_State *L, const char *what, lua_Debug *ar);

Returns information about a specific function or function invocation. 

To get information about a function invocation, the parameter ar must be a valid activation record that was filled by a previous call to lua_getstack or given as argument to a hook (see lua_Hook). 

To get information about a function you push it onto the stack and start the what string with the character '>'. (In that case, lua_getinfo pops the function in the top of the stack.) For instance, to know in which line a function f was defined, you can write the following code: 

lua_Debug ar;

lua_getfield(L, LUA_GLOBALSINDEX, "f");   get global 'f' 

lua_getinfo(L, ">S", &ar);

printf("%d\n", ar.linedefined);

Each character in the string what selects some fields of the structure ar to be filled or a value to be pushed on the stack: 

'n': fills in the field name and namewhat; 

'S': fills in the fields source, short_src, linedefined, lastlinedefined, and what; 

'l': fills in the field currentline; 

'u': fills in the field nups; 

'f': pushes onto the stack the function that is running at the given level; 

'L': pushes onto the stack a table whose indices are the numbers of the lines that are valid on the function. (A valid line is a line with some associated code, that is, a line where you can put a break point. Non-valid lines include empty lines and comments.) 

This function returns 0 on error (for instance, an invalid option in what). 

���ع����ض������������õ���Ϣ��

Ҫ��ú������õ���Ϣ������ar�����Ǳ�ǰһ������lua_getstack���ĺϷ��ļ����¼����������Ϊ��������hook���ο�lua_Hook����

Ϊ�˻�ú�����Ϣ������԰���ѹ���ջȻ���������ַ�'>'��ͷ���ַ���what��

����������£�lua_getinfo�ᵯ��ջ���ĺ�������

���磬Ϊ��ָ����һ�ж����˺���f�������д���´��룺

lua_Debug ar;

lua_getfield(L, LUA_GLOBALSINDEX, "f");   ��ȡȫ��'f' 

lua_getinfo(L, ">S", &ar);

printf("%d\n", ar.linedefined);

�ַ���what��ÿ���ַ�ѡ���˽ṹ��arҪ�����������ѹ���ջ��ֵ��

'n': �����name��namewhat�� 

'S': �����source, short_src, linedefined, lastlinedefined, ��what�� 

'l': �����currentline�� 

'u': �����nups��

'f': ��������������ĺ���ѹ��ջ���� 

'L': ��һ�������Ǻ����Ϸ��кŵı�ѹ���ջ��һ���Ϸ����Ǵ��й���������У�����һ���п��Է���һ���ϵ㡣���Ϸ����а������к�ע�͡���

��������ڴ���ʱ����0�����磬what��һ�����Ϸ�ѡ�
*/
LUA_API int lua_getinfo (lua_State *L, const char *what, lua_Debug *ar);

/**
lua_getlocal

[-0, +(0|1), -] 

const char *lua_getlocal (lua_State *L, lua_Debug *ar, int n);

Gets information about a local variable of a given activation record. The parameter ar must be a valid activation record that was filled by a previous call to lua_getstack or given as argument to a hook (see lua_Hook). The index n selects which local variable to inspect (1 is the first parameter or active local variable, and so on, until the last active local variable). lua_getlocal pushes the variable's value onto the stack and returns its name. 

Variable names starting with '(' (open parentheses) represent internal variables (loop control variables, temporaries, and C function locals). 

Returns NULL (and pushes nothing) when the index is greater than the number of active local variables. 

��ø����Ļ��¼��һ���ֲ���������Ϣ������ar��������Ч�Ļ��¼������֮ǰ��lua_getstack������䣬������Ϊhook�Ĳ�������lua_Hook��������nѡȡҪ�����ĸ��ֲ�������1�ǵ�һ���������ľֲ��������Դ����ƣ�ֱ�����һ����ľֲ���������lua_getlocal��������ֵѹջ�����������֡� 

��(������Բ���ţ���ͷ�ı�����ʾ�ڲ�������ѭ�����Ʊ�������ʱ������C�����ֲ��������� 

�������Ȼ�ľֲ�������������ʱ����NULL���Ҳ�ѹջ�κζ������� 
*/
LUA_API const char *lua_getlocal (lua_State *L, const lua_Debug *ar, int n);

/**
lua_setlocal

[-(0|1), +0, -] 

const char *lua_setlocal (lua_State *L, lua_Debug *ar, int n);

Sets the value of a local variable of a given activation record. Parameters ar and n are as in lua_getlocal (see lua_getlocal). lua_setlocal assigns the value at the top of the stack to the variable and returns its name. It also pops the value from the stack. 

Returns NULL (and pops nothing) when the index is greater than the number of active local variables. 

���ø����Ļ��¼��һ���ֲ�������ֵ������ar��n��lua_getlocal��һ������lua_getlocal����lua_setlocal��ջ����ֵ�������������������֡���Ҳ��ֵ��ջ�е����� 

������������ľֲ�����������ʱ����NULL���Ҳ���ջ�κζ������� 
*/
LUA_API const char *lua_setlocal (lua_State *L, const lua_Debug *ar, int n);

/**
lua_getupvalue

[-0, +(0|1), -] 

const char *lua_getupvalue (lua_State *L, int funcindex, int n);

Gets information about a closure's upvalue. (For Lua functions, upvalues are the external local variables that the function uses, and that are consequently included in its closure.) lua_getupvalue gets the index n of an upvalue, pushes the upvalue's value onto the stack, and returns its name. funcindex points to the closure in the stack. (Upvalues have no particular order, as they are active through the whole function. So, they are numbered in an arbitrary order.) 

Returns NULL (and pushes nothing) when the index is greater than the number of upvalues. For C functions, this function uses the empty string "" as a name for all upvalues. 

�õ��հ���һ��upvalue����Ϣ��������Lua������upvalue�Ǻ����õ��������������հ��ڵ��ⲿ�ֲ���������lua_getupvalue�õ�����n����upvalue������ֵѹջ������������funcindexָ��ջ�еıհ�����upvalueû���ض���˳����Ϊ���������������д�ͷ��β���ǻ�ģ���������������˳�򱻱�š��� 

��������upvalue��������ʱ����NULL���Ҳ�ѹջ�κζ�����������C�������������ÿմ�""��Ϊ����upvalue�����֡� 
*/
LUA_API const char *lua_getupvalue (lua_State *L, int funcindex, int n);

/**
lua_setupvalue

[-(0|1), +0, -] 

const char *lua_setupvalue (lua_State *L, int funcindex, int n);

Sets the value of a closure's upvalue. It assigns the value at the top of the stack to the upvalue and returns its name. It also pops the value from the stack. Parameters funcindex and n are as in the lua_getupvalue (see lua_getupvalue). 

Returns NULL (and pops nothing) when the index is greater than the number of upvalues. 

���ñհ���upvalue��ֵ������ջ����ֵ����upvalue�����������֡���Ҳ��ֵ��ջ�е���������funcindex��nͬlua_getupvalue��һ������lua_getupvalue���� 

����������upvalue������ʱ����NULL���Ҳ���ջ�κζ������� 
*/
LUA_API const char *lua_setupvalue (lua_State *L, int funcindex, int n);

/**
lua_sethook

[-0, +0, -] 

int lua_sethook (lua_State *L, lua_Hook f, int mask, int count);

Sets the debugging hook function. 

Argument f is the hook function. mask specifies on which events the hook will be called: it is formed by a bitwise or of the constants LUA_MASKCALL, LUA_MASKRET, LUA_MASKLINE, and LUA_MASKCOUNT. The count argument is only meaningful when the mask includes LUA_MASKCOUNT. For each event, the hook is called as explained below: 

The call hook: is called when the interpreter calls a function. The hook is called just after Lua enters the new function, before the function gets its arguments. 

The return hook: is called when the interpreter returns from a function. The hook is called just before Lua leaves the function. You have no access to the values to be returned by the function. 

The line hook: is called when the interpreter is about to start the execution of a new line of code, or when it jumps back in the code (even to the same line). (This event only happens while Lua is executing a Lua function.) 

The count hook: is called after the interpreter executes every count instructions. (This event only happens while Lua is executing a Lua function.) 

A hook is disabled by setting mask to zero. 

���õ��Ե�hook������ 

����f��hook������mask�涨�����ĸ��¼�ʱhook�������ã�

���ɳ���LUA_MASKCALL��LUA_MASKRET��LUA_MASKLINE��LUA_MASKCOUNT��λ����ɡ�

ֻ�����뺬��LUA_MASKCOUNTʱ��count�����������塣

����ÿ���¼���hook��������͵����������ã� 

call hook: �����������ú���ʱ�����á�hook��Lua�ս����º�����
		   �ں����õ������ǰ�����á� 

return hook: ���������Ӻ�������ʱ�����á�hook��Lua��Ҫ�뿪����ǰ�����á�
			 �㲻�ܷ���Ҫ���������ص�ֵ�� 

line hook: ����������Ҫ��ʼ���д����ִ�л����ص������У�������ͬһ�У�ʱ�����á�
		  �����¼�ֻ��Lua��ִ��Lua����ʱ�������� 

count hook: �ڽ�����ִ��ÿcount��ָ��󱻵��á������¼�ֻ��Lua��ִ��Lua����ʱ�������� 
			ͨ���趨maskΪ0����hook��
*/
LUA_API int lua_sethook (lua_State *L, lua_Hook func, int mask, int count);

/**
lua_gethook

[-0, +0, -] 

lua_Hook lua_gethook (lua_State *L);

Returns the current hook function. 

���ص�ǰ���Ӻ����� 
*/
LUA_API lua_Hook lua_gethook (lua_State *L);

/**
lua_gethookmask

[-0, +0, -] 

int lua_gethookmask (lua_State *L);

Returns the current hook mask. 

���ص�ǰ�������롣 
*/
LUA_API int lua_gethookmask (lua_State *L);

/**
lua_gethookcount

[-0, +0, -] 

int lua_gethookcount (lua_State *L);

Returns the current hook count. 

���ص�ǰ���Ӹ���
*/
LUA_API int lua_gethookcount (lua_State *L);

/**
lua_Debug

typedef struct lua_Debug {

int event;

const char *name;            (n) 

const char *namewhat;        (n) 

const char *what;            (S) 

const char *source;          (S) 

int currentline;             (l) 

int nups;                    (u) number of upvalues 

int linedefined;             (S) 

int lastlinedefined;         (S) 

char short_src[LUA_IDSIZE];  (S) 

private part 

other fields

} lua_Debug;

A structure used to carry different pieces of information about an active function. lua_getstack fills only the private part of this structure, for later use. To fill the other fields of lua_Debug with useful information, call lua_getinfo. 

The fields of lua_Debug have the following meaning: 

source: If the function was defined in a string, then source is that string. If the function was defined in a file, then source starts with a '@' followed by the file name. 

short_src: a "printable" version of source, to be used in error messages. 

linedefined: the line number where the definition of the function starts. 

lastlinedefined: the line number where the definition of the function ends. 

what: the string "Lua" if the function is a Lua function, "C" if it is a C function, "main" if it is the main part of a chunk, and "tail" if it was a function that did a tail call. In the latter case, Lua has no other information about the function. 

currentline: the current line where the given function is executing. When no line information is available, currentline is set to -1. 

name: a reasonable name for the given function. Because functions in Lua are first-class values, they do not have a fixed name: some functions can be the value of multiple global variables, while others can be stored only in a table field. The lua_getinfo function checks how the function was called to find a suitable name. If it cannot find a name, then name is set to NULL. 

namewhat: explains the name field. The value of namewhat can be "global", "local", "method", "field", "upvalue", or "" (the empty string), according to how the function was called. (Lua uses the empty string when no other option seems to apply.) 

nups: the number of upvalues of the function. 

���ڳ��й��ڻ��������Ϣ�Ĳ�ͬ����Ľṹ��lua_getstackֻ��䱾�ṹ��ר�ò��֣��������á�Ҫ�����õ���Ϣ���lua_Debug�������ֶΣ�����lua_getinfo�� 

lua_Debug���ֶξ�������ĺ��壺 

source: ����������ַ����ж��壬��source�����Ǹ��ַ���������������ļ��ж��壬��source�ԡ�@����ͷ����ļ����� 

short_src: һ��source�ġ��ɴ�ӡ���汾�������ڴ�����Ϣ�С� 

linedefined: �������忪ʼ���кš� 

lastlinedefined: ��������������кš� 

what: ���������Lua������Ϊ�ַ���"Lua"�������C������Ϊ"C"������ǵ�Ԫ�����岿����Ϊ"main"�����������ִ����β���õĺ�����Ϊ "tail"�����һ�������Luaû�й��ں�����������Ϣ�� 

currentline: ����������ִ�еĵ�ǰ�С���û������Ϣ����ʱ��currentline����Ϊ-1�� 

name: ���������ĺ��ʵ����֡���ΪLua�����ǵ�һ��ֵ����������û�й̶������֣�һЩ������Ϊ���ȫ�ֱ�����ֵ��Ȼ�������Ŀ�ֻ�洢��һ�����ֶ��С�����lua_getinfo��麯������α����õ����ҵ����ʵ����֡�����Ҳ�������name����ΪNULL�� 

namewhat: ����name�ֶΡ����ݺ�������ε��ã�namewhat��ֵ��Ϊ"global"��"local"��"method"��"field"��"upvalue"��""�����ַ���������������û������ѡ�����ʱLuaʹ�ÿմ����� 

nups: ������upvalue�������� 
*/
struct lua_Debug {
  int event;
  const char *name;	/* (n) */
  const char *namewhat;	/* (n) `global', `local', `field', `method' */
  const char *what;	/* (S) `Lua', `C', `main', `tail' */
  const char *source;	/* (S) */
  int currentline;	/* (l) */
  int nups;		/* (u) number of upvalues */
  int linedefined;	/* (S) */
  int lastlinedefined;	/* (S) */
  char short_src[LUA_IDSIZE]; /* (S) */
  /* private part */
  int i_ci;  /* active function */
};

/* }====================================================================== */


/******************************************************************************
* Copyright (C) 1994-2008 Lua.org, PUC-Rio.  All rights reserved.
*
* Permission is hereby granted, free of charge, to any person obtaining
* a copy of this software and associated documentation files (the
* "Software"), to deal in the Software without restriction, including
* without limitation the rights to use, copy, modify, merge, publish,
* distribute, sublicense, and/or sell copies of the Software, and to
* permit persons to whom the Software is furnished to do so, subject to
* the following conditions:
*
* The above copyright notice and this permission notice shall be
* included in all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
******************************************************************************/


#endif
