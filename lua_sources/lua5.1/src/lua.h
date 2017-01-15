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

用于C函数的类型。 

为了与Lua恰当地通讯，C函数必须使用下面的协议，它定义了参数和结果的传递方式：C函数在其栈中以顺序的方式（第一参数被首先压栈）接收来自Lua的参数。所以，当函数开始时，lua_gettop(L)返回函数收到的参数个数。第一参数（如果存在）在索引1处，最后的参数在索引lua_gettop(L)处。要向Lua返回值，C函数只需要将它们顺序压栈（第一个结果被首先压栈），并返回结果的个数。栈中结果下面的任何其他值将被Lua恰当地丢弃。同Lua函数一样，被Lua调用的C函数也能返回多个结果。 

作为例子，下面的函数接收可变数量的数字参数，并返回它们的平均数与总和： 

static int foo (lua_State *L) {

int n = lua_gettop(L);     参数的个数 

lua_Number sum = 0;

int i;

for (i = 1; i <= n; i++) {

if (!lua_isnumber(L, i)) {

lua_pushstring(L, "incorrect argument");

lua_error(L);

}

sum += lua_tonumber(L, i);

}

lua_pushnumber(L, sum/n);    第1个参数 

lua_pushnumber(L, sum);      第2个参数 

return 2;                    结果的个数 

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

被lua_load使用的读取器函数。每次需要单元的另一段时，lua_load沿着它的data参数调用读取器。读取器必须返回一个的内存块的指针，其中含有单元的新代码段，并且设置size为块尺寸。块必须一直存在直到再次调用读取器函数。读取器必须返回NULL或设置size为0来指示单元结束。读取器函数可能返回大于0的任何尺寸的代码段。 
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

由lua_dump使用的记录器函数的类型。lua_dump沿着要被写入的缓冲区（p）调用本函数，同时传入缓冲区尺寸（sz）和提供给lua_dump的data参数。每次调用会产生单元的另一段代码。 

记录器返回错误代码：0表示没错误；任何其他值表示错误并阻止lua_dump再次调用记录器。 
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

这种类型的内存分配函数由Lua状态机使用。分配器函数必须提供类似realloc的功能，但是不必完全一样。它的参数是ud，一个由lua_newstate传入的不透明指针；ptr，一个指向即将被分配/重分配/释放的内存块的指针；osize，内存块原来的尺寸；nsize，内存块的新尺寸。当且仅当osize是0时ptr为NULL。当nsize是0时，分配器必须返回NULL；如果osize非0，它将会释放ptr指向的内存块。当nsize非0时，当且仅当分配器不能满足请求时返回NULL。当nsize非0且osize是0时，分配器应该表现的类似malloc。当nsize和osize非0时，分配器表现的类似realloc。Lua假定当osize >= nsize时分配器决不失败。 

这儿有个分配器函数的简单实现。它被辅助库中的被luaL_newstate使用。 

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

该代码假定free(NULL)不起作用而且realloc(NULL, size)等价于malloc(size)。ANSI C确保这两种行为。
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

Lua中的数字类型。缺省是双精度浮点数，但是能在luaconf.h中改变。 

通过配置文件能改变Lua去操作其他的类型用作数字（例如单精度浮点数或长整型）。 
*/
/* type of numbers in Lua */
typedef LUA_NUMBER lua_Number;

/* type for integer functions */
/**
lua_Integer

typedef ptrdiff_t lua_Integer;

The type used by the Lua API to represent integral values. 

By default it is a ptrdiff_t, which is usually the largest signed integral type the machine handles "comfortably". 

被Lua API用来表示整型值的类型。 

缺省是ptrdiff_t，它通常是机器能处理的最大的带符号整型。 
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

创建一个新的，独立的状态。

如果无法创建状态则返回NULL（因为缺少内存）。

参数f是分配器函数；Lua通过这个函数为这个状态执行所有内存分配。

第二个参数ud是一个不透明的指针。Lua简单地在每次调用时把它传递给分配器函数。
*/
LUA_API lua_State *(lua_newstate) (lua_Alloc f, void *ud);

/**
lua_close

[-0, +0, -] 

void lua_close (lua_State *L);

Destroys all objects in the given Lua state (calling the corresponding garbage-collection metamethods, if any) and frees all dynamic memory used by this state. On several platforms, you may not need to call this function, because all resources are naturally released when the host program ends. On the other hand, long-running programs, such as a daemon or a web server, might need to release states as soon as they are not needed, to avoid growing too large. 

销毁给定Lua状态机中的全部对象（如果存在对应的垃圾收集元方法则调用它们），并释放该状态机占用的所有动态内存。在一些平台上，你可能不需要调用本函数，因为当宿主程序结束时，所有资源自然地被释放。另一方面，长期运行的程序，比如后台程序（daemon）或web服务器，可能需要在状态机不再需要时立刻释放它们，以避免增长过大。 
*/
LUA_API void       (lua_close) (lua_State *L);

/**
lua_newthread

[-0, +1, m] 

lua_State *lua_newthread (lua_State *L);

Creates a new thread, pushes it on the stack, and returns a pointer to a lua_State that represents this new thread. The new state returned by this function shares with the original state all global objects (such as tables), but has an independent execution stack. 

There is no explicit function to close or to destroy a thread. Threads are subject to garbage collection, like any Lua object. 

创建新线程，将其压栈，并返回指向lua_State的指针，它表示该新线程。本函数返回的新状态机与初始状态机共享所有全局对象（例如表），但具有独立的执行栈。 

没有关闭或销毁线程的显式函数。像任何Lua对象一样，线程受垃圾收集的支配。 
*/
LUA_API lua_State *(lua_newthread) (lua_State *L);

/**
lua_atpanic

[-0, +0, -]

lua_CFunction lua_atpanic (lua_State *L, lua_CFunction panicf);

Sets a new panic function and returns the old one. 

If an error happens outside any protected environment, Lua calls a panic function and then calls exit(EXIT_FAILURE), thus exiting the host application. Your panic function can avoid this exit by never returning (e.g., doing a long jump). 

The panic function can access the error message at the top of the stack. 

设置新的应急（panic）函数并返回前一个。 

如果在任何受保护的环境外面发生了错误，Lua调用应急函数接着调用exit(EXIT_FAILURE)，从而退出宿主程序。你的应急函数可通过永不返回（例如执行一次长跳转）以避免这次退出。 

应急函数可访问栈顶的错误消息。 
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

返回栈顶元素的索引。因为索引从1开始，该结果等于栈中元素的数量（所以0表示空栈）。 
*/
LUA_API int   (lua_gettop) (lua_State *L);

/**
lua_settop

[-?, +?, -] 

void lua_settop (lua_State *L, int index);

Accepts any acceptable index, or 0, and sets the stack top to this index. If the new top is larger than the old one, then the new elements are filled with nil. If index is 0, then all stack elements are removed. 

接受任意可接受的索引，或者0，把栈顶设置在此索引上。

如果新的栈顶大于原有的，那么新创建的元素被填充为空。

如果索引为0，那么所有堆栈元素都会被删除。 
*/
LUA_API void  (lua_settop) (lua_State *L, int idx);

/**
lua_pushvalue

[-0, +1, -] 

void lua_pushvalue (lua_State *L, int index);

Pushes a copy of the element at the given valid index onto the stack. 

把所给合法index处的元素的拷贝压入栈内。
*/
LUA_API void  (lua_pushvalue) (lua_State *L, int idx);

/**
lua_remove

[-1, +0, -] 

void lua_remove (lua_State *L, int index);

Removes the element at the given valid index, shifting down the elements above this index to fill the gap. Cannot be called with a pseudo-index, because a pseudo-index is not an actual stack position. 

移除给定的有效索引处的元素，并将该索引上面的元素下移来填充空隙。不能用伪索引调用，因为伪索引不是真实的栈位置。 
*/
LUA_API void  (lua_remove) (lua_State *L, int idx);

/**
lua_insert

[-1, +1, -] 

void lua_insert (lua_State *L, int index);

Moves the top element into the given valid index, shifting up the elements above this index to open space. Cannot be called with a pseudo-index, because a pseudo-index is not an actual stack position. 

将栈顶元素移入给定的有效索引，并将该索引上面的元素上移至开放空间。不能以伪索引调用，因为伪索引不是真实的栈位置。 
*/
LUA_API void  (lua_insert) (lua_State *L, int idx);

/**
lua_replace

[-1, +0, -] 

void lua_replace (lua_State *L, int index);

Moves the top element into the given position (and pops it), without shifting any element (therefore replacing the value at the given position). 

将栈顶元素移动到给定的索引中（并弹出它），不会移动任何元素（因此替换给定位置的值）。 
*/
LUA_API void  (lua_replace) (lua_State *L, int idx);

/**
lua_checkstack

[-0, +0, m] 

int lua_checkstack (lua_State *L, int extra);

Ensures that there are at least extra free stack slots in the stack. It returns false if it cannot grow the stack to that size. This function never shrinks the stack; if the stack is already larger than the new size, it is left unchanged. 

确保栈中存在至少extra个空闲栈槽位。如果栈不能增长到那个尺寸则返回假。本函数从不缩小栈；如果栈已经比新尺寸大则无变化。 
*/
LUA_API int   (lua_checkstack) (lua_State *L, int sz);

/**
lua_xmove

[-?, +?, -] 

void lua_xmove (lua_State *from, lua_State *to, int n);

Exchange values between different threads of the same global state. 

This function pops n values from the stack from, and pushes them onto the stack to. 

交换相同全局状态中不同线程的值。

这个函数从堆栈from中弹出n个值，然后把它们压入堆栈to。
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

如果所给可接受索引处的值是数或者可转换为数的字符串则返回1，否则返回0。
*/
LUA_API int             (lua_isnumber) (lua_State *L, int idx);

/**
lua_isstring

[-0, +0, -] 

int lua_isstring (lua_State *L, int index);

Returns 1 if the value at the given acceptable index is a string or a number (which is always convertible to a string), and 0 otherwise. 

如果所给可接受索引处的值是字符串或者数（总是可以转换为字符串）则返回1，否则返回0。
*/
LUA_API int             (lua_isstring) (lua_State *L, int idx);

/**
lua_iscfunction

[-0, +0, -] 

int lua_iscfunction (lua_State *L, int index);

Returns 1 if the value at the given acceptable index is a C function, and 0 otherwise. 

如果所给可接受索引处的值为C函数则返回1，否则返回0。
*/
LUA_API int             (lua_iscfunction) (lua_State *L, int idx);

/*
lua_isuserdata

[-0, +0, -] 

int lua_isuserdata (lua_State *L, int index);

Returns 1 if the value at the given acceptable index is a userdata (either full or light), and 0 otherwise. 

如果所给可接受索引处的值是一个userdata（完全或者轻量级）则返回1，否则返回0。
*/
LUA_API int             (lua_isuserdata) (lua_State *L, int idx);

/**
lua_type

[-0, +0, -] 

int lua_type (lua_State *L, int index);

Returns the type of the value in the given acceptable index, or LUA_TNONE for a non-valid index (that is, an index to an "empty" stack position). The types returned by lua_type are coded by the following constants defined in lua.h: LUA_TNIL, LUA_TNUMBER, LUA_TBOOLEAN, LUA_TSTRING, LUA_TTABLE, LUA_TFUNCTION, LUA_TUSERDATA, LUA_TTHREAD, and LUA_TLIGHTUSERDATA. 

返回给定的认可的索引处的值的类型，或者对不合法的索引返回LUA_TNONE（即指向“空”栈位置的索引）。lua_type返回的类型在lua.h中定义，被编码为下面的常量： LUA_TNIL、LUA_TNUMBER、LUA_TBOOLEAN、LUA_TSTRING、LUA_TTABLE、LUA_TFUNCTION、LUA_TUSERDATA、LUA_TTHREAD和LUA_TLIGHTUSERDATA。 
*/
LUA_API int             (lua_type) (lua_State *L, int idx);

/**
lua_typename

[-0, +0, -] 

const char *lua_typename  (lua_State *L, int tp);

Returns the name of the type encoded by the value tp, which must be one the values returned by lua_type. 

返回由值tp编码的类型名，tp必须是lua_type的返回值的其中一个。 
*/
LUA_API const char     *(lua_typename) (lua_State *L, int tp);

/**
lua_equal

[-0, +0, e]

int lua_equal (lua_State *L, int index1, int index2);

Returns 1 if the two values in acceptable indices index1 and index2 are equal, following the semantics of the Lua == operator (that is, may call metamethods). Otherwise returns 0. Also returns 0 if any of the indices is non valid. 

沿用Lua的==操作符的语义（即可能调用元方法），比较在可接受索引index1和index2中的两个值，如果相等则返回1。否则返回0。如果任何索引无效也返回0。 
*/
LUA_API int            (lua_equal) (lua_State *L, int idx1, int idx2);

/**
lua_rawequal

[-0, +0, -] 

int lua_rawequal (lua_State *L, int index1, int index2);

Returns 1 if the two values in acceptable indices index1 and index2 are primitively equal (that is, without calling metamethods). Otherwise returns 0. Also returns 0 if any of the indices are non valid. 

如果两个所接收的索引index1和index2处的值原生相等（即不调用元方法），则返回1。

否则，返回0。

如果任意索引不合法，也返回0。
*/
LUA_API int            (lua_rawequal) (lua_State *L, int idx1, int idx2);

/**
lua_lessthan

[-0, +0, e] 

int lua_lessthan (lua_State *L, int index1, int index2);

Returns 1 if the value at acceptable index index1 is smaller than the value at acceptable index index2, following the semantics of the Lua < operator (that is, may call metamethods). Otherwise returns 0. Also returns 0 if any of the indices is non valid. 

如果所给可接受索引index1处的值小于索引index2处的值则返回1，遵循Lua的<运算符的语义（即，可能调用元方法）。

否则返回0。

如果任意索引不合法也会返回0。
*/
LUA_API int            (lua_lessthan) (lua_State *L, int idx1, int idx2);

/**
lua_tonumber

[-0, +0, -] 

lua_Number lua_tonumber (lua_State *L, int index);

Converts the Lua value at the given acceptable index to the C type lua_Number (see lua_Number). The Lua value must be a number or a string convertible to a number (see §2.2.1); otherwise, lua_tonumber returns 0. 

lua_Number lua_tonumber (lua_State *L, int index);

把所给可接受索引处的值转换为C类型lua_Number（参考lua_Number）。

Lua值必须为一个数或可以转换为数的字符串（参考§2.2.1）；否则，lua_tonumber返回0。
*/
LUA_API lua_Number      (lua_tonumber) (lua_State *L, int idx);

/**
lua_tointeger

[-0, +0, -] 

lua_Integer lua_tointeger (lua_State *L, int index);

Converts the Lua value at the given acceptable index to the signed integral type lua_Integer. The Lua value must be a number or a string convertible to a number (see §2.2.1); otherwise, lua_tointeger returns 0. 

If the number is not an integer, it is truncated in some non-specified way. 

把所给可接受索引处的值转换为带符号的整型lua_Integer。

Lua值必须是一个数或者可转为数的字符串（参考§2.2.1），否则，lua_tointeger返回0。

如果数不是整数，会以不确定的方式被剪切。
*/
LUA_API lua_Integer     (lua_tointeger) (lua_State *L, int idx);

/**
lua_toboolean

[-0, +0, -] 

int lua_toboolean (lua_State *L, int index);

Converts the Lua value at the given acceptable index to a C boolean value (0 or 1). Like all tests in Lua, lua_toboolean returns 1 for any Lua value different from false and nil; otherwise it returns 0. It also returns 0 when called with a non-valid index. (If you want to accept only actual boolean values, use lua_isboolean to test the value's type.) 

把所给可接受的索引处的Lua值转换为C的布尔值(0或1)。

好像Lua的所有测试那样，对于任何不是false和nil的值lua_toboolean返回1；否则返回0。

它在用非法索引调用时也返回0。

（如果你只想接收实际的布尔值，使用lua_isboolean来检查值类型）
*/
LUA_API int             (lua_toboolean) (lua_State *L, int idx);

/**
lua_tolstring

[-0, +0, m] 

const char *lua_tolstring (lua_State *L, int index, size_t *len);

Converts the Lua value at the given acceptable index to a C string. If len is not NULL, it also sets *len with the string length. The Lua value must be a string or a number; otherwise, the function returns NULL. If the value is a number, then lua_tolstring also changes the actual value in the stack to a string. (This change confuses lua_next when lua_tolstring is applied to keys during a table traversal.) 

lua_tolstring returns a fully aligned pointer to a string inside the Lua state. This string always has a zero ('\0') after its last character (as in C), but can contain other zeros in its body. Because Lua has garbage collection, there is no guarantee that the pointer returned by lua_tolstring will be valid after the corresponding value is removed from the stack. 

把所给可接受索引处的值转换为C的字符串。

如果len不是NULL，也可以设置*len为字符串长度。

Lua值不许是一个字符串或数。否则，这个函数返回NULL。

如果值是数，那么lua_tolstring还会改变堆栈的实际值为字符串。

（当lua_tolstring应用到表遍历的键中，这种改变会混淆lua_next。）

lua_tolstring返回一个完全对齐的指向Lua状态内部的字符串的指针。

因为Lua有垃圾回收，所以不保证lua_tolstring所返回的指针在相应值从堆栈中删除后仍合法。
*/
LUA_API const char     *(lua_tolstring) (lua_State *L, int idx, size_t *len);

/**
lua_objlen

[-0, +0, -] 

size_t lua_objlen (lua_State *L, int index);

Returns the "length" of the value at the given acceptable index: for strings, this is the string length; for tables, this is the result of the length operator ('#'); for userdata, this is the size of the block of memory allocated for the userdata; for other values, it is 0. 

返回给定的认可的索引处的值的“长度”：对于字符串，这是其长度；对于表，这是取长操作符（‘#’）的结果；对于用户数据，这是为其分配的内存块的尺寸；对于其他类型是0。 
*/
LUA_API size_t          (lua_objlen) (lua_State *L, int idx);

/**
lua_tocfunction

[-0, +0, -] 

lua_CFunction lua_tocfunction (lua_State *L, int index);

Converts a value at the given acceptable index to a C function. That value must be a C function; otherwise, returns NULL. 

把所给可接受索引处的值转换为C函数。

那个值必须是C函数，否则返回NULL。
*/
LUA_API lua_CFunction   (lua_tocfunction) (lua_State *L, int idx);

/**
lua_touserdata

[-0, +0, -] 

void *lua_touserdata (lua_State *L, int index);

If the value at the given acceptable index is a full userdata, returns its block address. If the value is a light userdata, returns its pointer. Otherwise, returns NULL. 

如果给定的认可的索引处的值是完整的用户数据，则返回其块地址。

如果是轻型用户数据，返回其指针。否则返回NULL。 
*/
LUA_API void	       *(lua_touserdata) (lua_State *L, int idx);

/**
lua_tothread

[-0, +0, -] 

lua_State *lua_tothread (lua_State *L, int index);

Converts the value at the given acceptable index to a Lua thread (represented as lua_State*). This value must be a thread; otherwise, the function returns NULL. 

把给定的认可的索引处的值转换为Lua线程（用lua_State*表示）。该值必须为线程；否则，本函数返回NULL。 
*/
LUA_API lua_State      *(lua_tothread) (lua_State *L, int idx);

/**
lua_topointer

[-0, +0, -] 

const void *lua_topointer (lua_State *L, int index);

Converts the value at the given acceptable index to a generic C pointer (void*). The value can be a userdata, a table, a thread, or a function; otherwise, lua_topointer returns NULL. Different objects will give different pointers. There is no way to convert the pointer back to its original value. 

Typically this function is used only for debug information. 

把所给可接受索引处的值转换为泛型的C指针（void*）。

这个值可以是用户定义数据，表，线程，或者是函数；否则，lua_topointer返回NULL。

不同的对象会给出不同的指针。

没有方法可以把指针转换回它原有值。

特别地这个函数仅用于调试信息。
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

压入一个空值进堆栈。 
*/
LUA_API void  (lua_pushnil) (lua_State *L);

/**
lua_pushnumber

[-0, +1, -] 

void lua_pushnumber (lua_State *L, lua_Number n);

Pushes a number with value n onto the stack. 

压入一个拥有值n的数字进堆栈。
*/
LUA_API void  (lua_pushnumber) (lua_State *L, lua_Number n);

/**
lua_pushinteger

[-0, +1, -] 

void lua_pushinteger (lua_State *L, lua_Integer n);

Pushes a number with value n onto the stack. 

将一个值为n的数字压栈。 
*/
LUA_API void  (lua_pushinteger) (lua_State *L, lua_Integer n);

/**
lua_pushlstring

[-0, +1, m] 

void lua_pushlstring (lua_State *L, const char *s, size_t len);

Pushes the string pointed to by s with size len onto the stack. Lua makes (or reuses) an internal copy of the given string, so the memory at s can be freed or reused immediately after the function returns. The string can contain embedded zeros. 

将s指向的尺寸为len的字符串压栈。Lua制造（或重用）给定字符串的内部拷贝，所以函数返回后s的内存立刻可被释放或重用。字符串可含有内嵌的0。 
*/
LUA_API void  (lua_pushlstring) (lua_State *L, const char *s, size_t l);

/**
lua_pushstring

[-0, +1, m] 

void lua_pushstring (lua_State *L, const char *s);

Pushes the zero-terminated string pointed to by s onto the stack. Lua makes (or reuses) an internal copy of the given string, so the memory at s can be freed or reused immediately after the function returns. The string cannot contain embedded zeros; it is assumed to end at the first zero. 

将s指向的以0结尾的字符串压栈。Lua制造（或重用）给定字符串的内部拷贝，

所以函数返回后s的内存立刻可被释放或重用。字符串不可含有内嵌的0；

假定它在首个0处结束。 
*/
LUA_API void  (lua_pushstring) (lua_State *L, const char *s);

/**
lua_pushvfstring

[-0, +1, m] 

const char *lua_pushvfstring (lua_State *L,
								  const char *fmt,
								  va_list argp);

Equivalent to lua_pushfstring, except that it receives a va_list instead of a variable number of arguments. 

等效于lua_pushfstring，除了接收一个va_list以代替参数个数。
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

将格式化的字符串压栈并返回指向它的指针。它与C函数sprintf类似，但也有一些重要的区别： 

你不需要为结果分配空间：结果是Lua字符串且Lua会照看内存分配（以及通过垃圾收集解除分配）。 

转换说明符非常的有限。没有标记、宽度或精度。转换说明符只能是'%%' （在字符串中插入一个'%'），'%s' （插入一个以0结尾的字符串，没有尺寸限制），'%f' （插入一个lua_Number），'%p' （插入一个指针作为十六进制数），'%d' （插入一个int），以及'%c' （插入一个int作为字符）。 
*/
LUA_API const char *(lua_pushfstring) (lua_State *L, const char *fmt, ...);

/**
lua_pushcclosure

[-n, +1, m] 

void lua_pushcclosure (lua_State *L, lua_CFunction fn, int n);

Pushes a new C closure onto the stack. 

When a C function is created, it is possible to associate some values with it, thus creating a C closure (see §3.4); these values are then accessible to the function whenever it is called. To associate values with a C function, first these values should be pushed onto the stack (when there are multiple values, the first value is pushed first). Then lua_pushcclosure is called to create and push the C function onto the stack, with the argument n telling how many values should be associated with the function. lua_pushcclosure also pops these values from the stack. 

The maximum value for n is 255. 

把新的C闭包压栈。 

当C函数被创建时，它可以把一些值与自己关联，这样就创建了C闭包（见§3.4）；接下来无论何时它被调用，这些值对该函数都是可访问的。要将值与C函数关联，首先这些值应当被压栈（当有多个值时第一个值首先压栈）。然后用参数n调用lua_pushcclosure来创建C函数并将其压栈，n表明应当把多少值关联到该函数。lua_pushcclosure也会将这些值从栈中弹出。 

n的最大值是255。 
*/
LUA_API void  (lua_pushcclosure) (lua_State *L, lua_CFunction fn, int n);

/**
lua_pushboolean

[-0, +1, -] 

void lua_pushboolean (lua_State *L, int b);

Pushes a boolean value with value b onto the stack. 

把值b作为布尔值压栈。 
*/
LUA_API void  (lua_pushboolean) (lua_State *L, int b);

/**
lua_pushlightuserdata

[-0, +1, -] 

void lua_pushlightuserdata (lua_State *L, void *p);

Pushes a light userdata onto the stack. 

Userdata represent C values in Lua. A light userdata represents a pointer. It is a value (like a number): you do not create it, it has no individual metatable, and it is not collected (as it was never created). A light userdata is equal to "any" light userdata with the same C address. 

将轻型用户数据压栈。 

在Lua中用户数据表示C值。轻型用户数据表示一个指针。它是个值（就像数字）：你不用创建它，它没有单独的元表，而且它不会被回收（如同从不被创建）。带有相同的C地址的轻型用户数据相等。 
*/
LUA_API void  (lua_pushlightuserdata) (lua_State *L, void *p);

/**
lua_pushthread

[-0, +1, -] 

int lua_pushthread (lua_State *L);

Pushes the thread represented by L onto the stack. Returns 1 if this thread is the main thread of its state. 

把L代表的线程压入栈中。

如果这个线程是状态的主线程则返回1。
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

This function pops the key from the stack (putting the resulting value in its place). As in Lua, this function may trigger a metamethod for the "index" event (see §2.8). 

将值t[k]压栈，其中t是指定的有效索引处的值，并且k是栈顶的值。 

本函数将键出栈（将结果值放在它的位置）。同Lua中一样，本函数可能触发用于“index”事件的元方法（见§2.8）。 
*/
LUA_API void  (lua_gettable) (lua_State *L, int idx);

/**
lua_getfield

[-0, +1, e] 

void lua_getfield (lua_State *L, int index, const char *k);

Pushes onto the stack the value t[k], where t is the value at the given valid index. As in Lua, this function may trigger a metamethod for the "index" event (see §2.8). 

将t[k]的值压栈，其中t是给定的有效索引处的值。同Lua中一样，本函数可能触发用于“index”事件的元方法（见§2.8）。 
*/
LUA_API void  (lua_getfield) (lua_State *L, int idx, const char *k);

/*
lua_rawget

[-1, +1, -] 

void lua_rawget (lua_State *L, int index);

Similar to lua_gettable, but does a raw access (i.e., without metamethods). 

类似于lua_gettable，但执行原生访问(例如，没有元方法） 
*/
LUA_API void  (lua_rawget) (lua_State *L, int idx);

/**
lua_rawgeti

[-0, +1, -] 

void lua_rawgeti (lua_State *L, int index, int n);

Pushes onto the stack the value t[n], where t is the value at the given valid index. The access is raw; that is, it does not invoke metamethods. 

把t[n]的值压入栈中，其中t是所给合法Index处的值。

访问是原生的，不执行元方法
*/
LUA_API void  (lua_rawgeti) (lua_State *L, int idx, int n);

/**
lua_createtable

[-0, +1, m] 

void lua_createtable (lua_State *L, int narr, int nrec);

Creates a new empty table and pushes it onto the stack. The new table has space pre-allocated for narr array elements and nrec non-array elements. This pre-allocation is useful when you know exactly how many elements the table will have. Otherwise you can use the function lua_newtable. 

创建新的空表并将其压栈。新表预分配narr个数组元素和nrec个非数组元素的空闲空间。当你确切地知道表将由多少个元素时，预分配是非常有用的。否则，你可用函数lua_newtable。 
*/
LUA_API void  (lua_createtable) (lua_State *L, int narr, int nrec);

/**
lua_newuserdata

[-0, +1, m] 

void *lua_newuserdata (lua_State *L, size_t size);

This function allocates a new block of memory with the given size, pushes onto the stack a new full userdata with the block address, and returns this address. 

Userdata represent C values in Lua. A full userdata represents a block of memory. It is an object (like a table): you must create it, it can have its own metatable, and you can detect when it is being collected. A full userdata is only equal to itself (under raw equality). 

When Lua collects a full userdata with a gc metamethod, Lua calls the metamethod and marks the userdata as finalized. When this userdata is collected again then Lua frees its corresponding memory. 

本函数分配新的给定尺寸的内存块，以块地址的方式将完整的用户数据压栈，并返回该地址。 

Lua中的用户数据表示C值。完整的用户数据表示一块内存。它是个对象（如同表）：你必须创建它，它可以有自己的元表，而且当被收集时能被检测到。完整的用户数据只等于自己（依照原始的相等比较）。 

当Lua用gc元方法收集完整的用户数据时，Lua调用该元方法并把用户数据标记为完成的。当该用户数据再次被收集时，Lua释放其对应的内存。 
*/
LUA_API void *(lua_newuserdata) (lua_State *L, size_t sz);

/**
lua_getmetatable

[-0, +(0|1), -] 

int lua_getmetatable (lua_State *L, int index);

Pushes onto the stack the metatable of the value at the given acceptable index. If the index is not valid, or if the value does not have a metatable, the function returns 0 and pushes nothing on the stack. 

将给定的认可的索引处的值的元表压栈。如果索引无效，或者如果该值没有元表，本函数返回0且不会压栈任何东西。 
*/
LUA_API int   (lua_getmetatable) (lua_State *L, int objindex);

/**
lua_getfenv

[-0, +1, -] 

void lua_getfenv (lua_State *L, int index);

Pushes onto the stack the environment table of the value at the given index. 

将给定索引处的值的环境表压栈。 
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

This function pops both the key and the value from the stack. As in Lua, this function may trigger a metamethod for the "newindex" event (see §2.8). 

执行t[k] = v的等价操作，其中t是给定的有效索引处的值，v是栈顶的值，k正好是栈顶下面的值。 

本函数将键和值都弹出栈。同Lua中一样，本函数可能触发“newindex”事件的元方法（见§2.8）。 
*/
LUA_API void  (lua_settable) (lua_State *L, int idx);

/**
lua_setfield

[-1, +0, e] 

void lua_setfield (lua_State *L, int index, const char *k);

Does the equivalent to t[k] = v, where t is the value at the given valid index and v is the value at the top of the stack. 

This function pops the value from the stack. As in Lua, this function may trigger a metamethod for the "newindex" event (see §2.8). 

执行t[k] = v的等价操作，其中t是给定的有效索引处的值，v是栈顶的值。 

本函数从栈中弹出值。同Lua中一样，本函数可能触发“newindex”事件的元方法（见§2.8）。 
*/
LUA_API void  (lua_setfield) (lua_State *L, int idx, const char *k);

/**
lua_rawset

[-2, +0, m] 

void lua_rawset (lua_State *L, int index);

Similar to lua_settable, but does a raw assignment (i.e., without metamethods). 

类似lua_settable，但是执行一次原生赋值（也就是不用元方法）。 
*/
LUA_API void  (lua_rawset) (lua_State *L, int idx);

/**
lua_rawseti

[-1, +0, m] 

void lua_rawseti (lua_State *L, int index, int n);

Does the equivalent of t[n] = v, where t is the value at the given valid index and v is the value at the top of the stack. 

This function pops the value from the stack. The assignment is raw; that is, it does not invoke metamethods. 

执行t[n] = v的等价操作，其中t是给定的有效索引处的值，v是栈顶的值。 

本函数将值出栈。赋值是原生的；即不调用元方法。 
*/
LUA_API void  (lua_rawseti) (lua_State *L, int idx, int n);

/**
lua_setmetatable

[-1, +0, -] 

int lua_setmetatable (lua_State *L, int index);

Pops a table from the stack and sets it as the new metatable for the value at the given acceptable index. 

从栈中弹出一个表并将其设为给定的认可的索引处的值的新元表。 
*/
LUA_API int   (lua_setmetatable) (lua_State *L, int objindex);

/**
lua_setfenv

[-1, +0, -] 

int lua_setfenv (lua_State *L, int index);

Pops a table from the stack and sets it as the new environment for the value at the given index. If the value at the given index is neither a function nor a thread nor a userdata, lua_setfenv returns 0. Otherwise it returns 1. 

从栈中弹出一个表并把它设为给定索引处的值的新环境。如果给定索引处的值既不是函数又不是线程也不是用户数据，lua_setfenv返回0。否则返回1。 
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

调用一个函数。 

要调用函数必须使用下面的协议：首先，将要被调用的函数压栈；然后，将函数参数顺序压栈；即第一参数首先压栈。最后，调用lua_call；nargs是你压栈的参数数量。当函数被调用时所有参数以及函数值被弹出栈。当函数返回时其结果被压栈。结果被调整为nresults个，除非nresults是LUA_MULTRET。在该情况下，来自函数的所有结果被压栈。Lua会确保返回值适合栈空间。函数结果被顺序压栈（第一个结果首先压栈），因而调用后最后一个结果在栈顶。 

被调用函数内的任何错误会被向上传播（使用longjmp）。 

下面的例子显示宿主程序可如何执行与该Lua代码等价的操作： 

a = f("how", t.x, 14)

这是C中的代码： 

lua_getfield(L, LUA_GLOBALSINDEX, "f");           要被调用的函数 

lua_pushstring(L, "how");                              第1个参数 

lua_getfield(L, LUA_GLOBALSINDEX, "t");             要被索引的表 

lua_getfield(L, -1, "x");            将t.x的结果压栈（第2个参数） 

lua_remove(L, -2);                   从栈中删除‘t’ 

lua_pushinteger(L, 14);                                第3个参数 

lua_call(L, 3, 1);                以3个参数调用‘f’，返回1个结果 

lua_setfield(L, LUA_GLOBALSINDEX, "a");         设置全局变量‘a’ 

注意上面的代码是“对称的”：在其结尾，栈回到其初始配置。这是一种良好的编程习惯。 
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

在保护模式中调用函数。 

nargs和nresults同lua_call中的有相同的含义。如果调用期间没有错误，

lua_pcall的行为完全类似lua_call。然而，如果存在任何错误，lua_pcall捕捉它，

将单个值压栈（错误消息），并返回错误代码。同lua_call一样，

lua_pcall总是从栈中移除函数及其参数。 

如果errfunc是0，则在栈上返回的错误消息就是原始的错误消息。

否则,errfunc就是错误处理器函数（error handler function）的栈索引。

（当前实现中，该索引不能是伪索引。）在发生运行时错误时，该函数将用错误消息调用，

而且其返回值将变成栈上被lua_pcall返回的消息。 

典型地，错误处理器函数用于向错误消息中增加更多调试信息，例如栈回溯。

这种信息不能在lua_pcall返回后搜集，因为那时候栈已释放。 

成功时lua_pcall函数返回0，或者下面的错误代码之一（在lua.h中定义）： 

LUA_ERRRUN: 运行时错误。 

LUA_ERRMEM: 内存分配错误。对于这类错误，Lua不会调用错误处理器函数。 

LUA_ERRERR: 当运行错误处理器函数时的错误。 
*/
LUA_API int   (lua_pcall) (lua_State *L, int nargs, int nresults, int errfunc);

/**
lua_cpcall

[-0, +(0|1), -] 

int lua_cpcall (lua_State *L, lua_CFunction func, void *ud);

Calls the C function func in protected mode. func starts with only one element in its stack, a light userdata containing ud. In case of errors, lua_cpcall returns the same error codes as lua_pcall, plus the error object on the top of the stack; otherwise, it returns zero, and does not change the stack. All values returned by func are discarded. 

以保护模式调用C函数func。

func的栈中只有一个元素，是个包含ud的轻量级用户数据。

发生错误时，lua_cpcall返回同lua_pcall一样的错误代码，以及栈顶的错误对象；

否则返回0且不改变栈。func返回的所有值被丢弃。

注：第三个参数ud是userdata，可以在C函数中用lua_touserdata()取出。
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

The chunkname argument gives a name to the chunk, which is used for error messages and in debug information (see §3.8). 

加载一个Lua块。

如果没有错误，lua_load把编译好的块作为一个Lua函数压入栈顶。

否则，它把错误信息压入栈中。

lua_load的返回值有：

0: 无错误; 

LUA_ERRSYNTAX: 在预编译时出现语法错误； 

LUA_ERRMEM: 内存分配错误。

这个函数值能加载一个块；它不会运行这个函数。

lua_load自动检测块是文本格式还是二进制格式，并且相应进行加载（见程序luac）。

lua_load函数使用一个用户提供的读入器函数读入块（见lua_Reader）。

data参数是一个传递给读入器函数的不透明值。

chunkname参数给块起名，用于错误信息以及调试信息（见§3.8）。 
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

将函数转储为二进制代码单元。接收栈顶的Lua函数并产生二进制单元，如果后者被再次加载，得到与被转储的等价的函数。当产生单元的各部分时，lua_dump用给定的data调用函数writer（见lua_Writer）来写出它们。 

返回值是最后一次调用记录器（writer）返回的错误代码；0表示没有错误。 

本函数不会将Lua函数从栈中弹出。 
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

挂起一个协程 

这个函数应该仅在C函数return表达式中使用，如下：

return lua_yield (L, nresults);

当一个C函数像那样调用lua_yield，运行的协程暂停它的执行，并且在使此协程重新开始的lua_resume调用处返回。

参数nresults是堆栈中传递给lua_resume结果的值个数。
*/
LUA_API int  (lua_yield) (lua_State *L, int nresults);

/**
lua_resume

[-?, +?, -] 

int lua_resume (lua_State *L, int narg);

Starts and resumes a coroutine in a given thread. 

To start a coroutine, you first create a new thread (see lua_newthread); then you push onto its stack the main function plus any arguments; then you call lua_resume, with narg being the number of arguments. This call returns when the coroutine suspends or finishes its execution. When it returns, the stack contains all values passed to lua_yield, or all values returned by the body function. lua_resume returns LUA_YIELD if the coroutine yields, 0 if the coroutine finishes its execution without errors, or an error code in case of errors (see lua_pcall). In case of errors, the stack is not unwound, so you can use the debug API over it. The error message is on the top of the stack. To restart a coroutine, you put on its stack only the values to be passed as results from yield, and then call lua_resume. 

启动和恢复给定线程中的一个协程。 

要启动一个协程，首先创建线程（见lua_newthread）；接着将主函数以及任何参数压到它的栈中；然后用narg指定参数个数来调用lua_resume。当协程挂起或运行完成时该调用返回。当它返回时，栈中含有lua_yield传入的所有值，或主体函数返回的所有值。如果协程中断则lua_resume返回LUA_YIELD，如果运行无误结束则返回0，或者在发生错误时返回错误代码（见lua_pcall）。发生错误时，栈未被释放，所以可对它使用调试API。错误消息位于栈顶。要重新启动协程，你只需把要从yield传出的结果放到它的栈上，然后调用lua_resume。 
*/
LUA_API int  (lua_resume) (lua_State *L, int narg);

/**
lua_status

[-0, +0, -] 

int lua_status (lua_State *L);

Returns the status of the thread L. 

The status can be 0 for a normal thread, an error code if the thread finished its execution with an error, or LUA_YIELD if the thread is suspended. 

返回线程L的状态值。

线程正常时状态值为0，线程遇到错误而结束它的执行则返回一个错误码，线程挂起（暂停）则返回LUA_YIELD。
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

LUA_GCSETPAUSE: sets data as the new value for the pause of the collector (see §2.10). The function returns the previous value of the pause. 

LUA_GCSETSTEPMUL: sets data as the new value for the step multiplier of the collector (see §2.10). The function returns the previous value of the step multiplier. 

控制垃圾收集器。 

本函数根据参数what的值执行若干任务： 

LUA_GCSTOP: 停止垃圾收集器。 

LUA_GCRESTART: 重新启动垃圾收集器。 

LUA_GCCOLLECT: 执行一次完整的垃圾收集器周期。 

LUA_GCCOUNT: 返回Lua当前占用的内存总量（以Kbyte为单位）。 

LUA_GCCOUNTB: 返回Lua当前的内存用量的字节数除1024的余数。 

LUA_GCSTEP: 执行一步增量垃圾收集。步“长”由data以未指定的方式控制（值越大意味着步骤越多）。要控制步长，你必须实验性地调整data的值。如果该步完成了一个垃圾收集周期则函数返回1。 

LUA_GCSETPAUSE: 设置data作为收集器的pause（见§2.10）的新值。返回pause的前一个值。 

LUA_GCSETSTEPMUL: 设置data作为收集器的step multiplier（见§2.10）的新值。返回step multiplier的前一个值。 
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

产生一个Lua错误。错误消息（实际上可为任何类型的Lua值）必须在栈顶。本函数执行长跳转，因此从不返回。（见luaL_error）。 
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

从栈中弹出一个键，并从给定的索引处的表中弹出一个键-值对（给定键的“下一”对）。如果表中没有更多的元素，则lua_next返回0（且不会压栈任何东西）。 

典型的遍历看起来像这样： 

表在栈中的索引‘t’处 

lua_pushnil(L);   第一个键 

while (lua_next(L, t) != 0) {

使用‘键’（索引-2）和‘值’(索引-1） 

printf("%s - %s\n",

lua_typename(L, lua_type(L, -2)),

lua_typename(L, lua_type(L, -1)));

移除‘值’；为下次迭代保留‘键’ 

lua_pop(L, 1);

}

当遍历表时，不要直接对键调用lua_tolstring，除非你知道键确实是字符串。回忆一下，lua_tolstring会改变给定所引处的值；这会干扰下一次调用lua_next。 
*/
LUA_API int   (lua_next) (lua_State *L, int idx);

/**
lua_concat

[-n, +1, e] 

void lua_concat (lua_State *L, int n);

Concatenates the n values at the top of the stack, pops them, and leaves the result at the top. If n is 1, the result is the single value on the stack (that is, the function does nothing); if n is 0, the result is the empty string. Concatenation is performed following the usual semantics of Lua (see §2.5.4). 

连接栈顶的n个值，弹出它们并将结果留在栈顶。如果n是1，

结果就是栈上的单个值（即函数什么也不做）；如果n是0，结果是空字符串。

连接操作依照Lua的常规语义执行（见§2.5.4）。 
*/
LUA_API void  (lua_concat) (lua_State *L, int n);

/**
lua_getallocf

[-0, +0, -] 

lua_Alloc lua_getallocf (lua_State *L, void **ud);

Returns the memory-allocation function of a given state. If ud is not NULL, Lua stores in *ud the opaque pointer passed to lua_newstate. 

返回给定状态机的内存分配函数。如果ud不为NULL，Lua将lua_newstate传入的不透明指针存入*ud。 
*/
LUA_API lua_Alloc (lua_getallocf) (lua_State *L, void **ud);

/**
lua_setallocf

[-0, +0, -] 

void lua_setallocf (lua_State *L, lua_Alloc f, void *ud);

Changes the allocator function of a given state to f with user data ud. 

void lua_setallocf (lua_State *L, lua_Alloc f, void *ud);

把给定状态机的分配器函数换成带用户数据ud的f。 
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

从栈中弹出n个元素。 
*/
#define lua_pop(L,n)		lua_settop(L, -(n)-1)

/**
lua_newtable

[-0, +1, m] 

void lua_newtable (lua_State *L);

Creates a new empty table and pushes it onto the stack. It is equivalent to lua_createtable(L, 0, 0). 

创建新的空表并将其压栈。它等价于lua_createtable(L, 0, 0)。 
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

设置C函数f为全局变量name的新值。它被定义为宏： 

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

将C函数压栈。本函数接收一个指向C函数的指针并把一个function类型的Lua值压栈，当这种类型的函数被调用时，会调用相应的C函数。 

任何要在Lua中注册的函数必须遵循正确的协议，以接收其参数并返回其结果（见lua_CFunction）。 

lua_pushcfunction被定义为宏： 

#define lua_pushcfunction(L,f)  lua_pushcclosure(L,f,0)
*/
#define lua_pushcfunction(L,f)	lua_pushcclosure(L, (f), 0)

#define lua_strlen(L,i)		lua_objlen(L, (i))

/**
lua_isfunction

[-0, +0, -] 

int lua_isfunction (lua_State *L, int index);

Returns 1 if the value at the given acceptable index is a function (either C or Lua), and 0 otherwise. 

如果所给可接受索引处的值为函数（C或者是Lua）则返回1，否则返回0。
*/
#define lua_isfunction(L,n)	(lua_type(L, (n)) == LUA_TFUNCTION)

/**
lua_istable

[-0, +0, -] 

int lua_istable (lua_State *L, int index);

Returns 1 if the value at the given acceptable index is a table, and 0 otherwise. 

如果所给可接受索引处的值是表则返回1，否则返回0。
*/
#define lua_istable(L,n)	(lua_type(L, (n)) == LUA_TTABLE)

/**
lua_islightuserdata

[-0, +0, -] 

int lua_islightuserdata (lua_State *L, int index);

Returns 1 if the value at the given acceptable index is a light userdata, and 0 otherwise. 

如果所给可接受索引处的值为轻量级userdata则返回1，否则返回0。
*/
#define lua_islightuserdata(L,n)	(lua_type(L, (n)) == LUA_TLIGHTUSERDATA)

/**
lua_isnil

[-0, +0, -] 

int lua_isnil (lua_State *L, int index);

Returns 1 if the value at the given acceptable index is nil, and 0 otherwise. 

如果所给可接受索引处的值为nil则返回1，否则返回0。
*/
#define lua_isnil(L,n)		(lua_type(L, (n)) == LUA_TNIL)

/**
lua_isboolean

[-0, +0, -] 

int lua_isboolean (lua_State *L, int index);

Returns 1 if the value at the given acceptable index has type boolean, and 0 otherwise. 

如果给定的认可的索引处的值具有布尔类型则返回1，否则返回0。 
*/
#define lua_isboolean(L,n)	(lua_type(L, (n)) == LUA_TBOOLEAN)

/**
lua_isthread

[-0, +0, -] 

int lua_isthread (lua_State *L, int index);

Returns 1 if the value at the given acceptable index is a thread, and 0 otherwise. 

如果所给可接受索引处的值是一个线程则返回1，否则返回0。
*/
#define lua_isthread(L,n)	(lua_type(L, (n)) == LUA_TTHREAD)

/**
lua_isnone

[-0, +0, -] 

int lua_isnone (lua_State *L, int index);

Returns 1 if the given acceptable index is not valid (that is, it refers to an element outside the current stack), and 0 otherwise. 

如果所给可接受索引处的值不可用（即引用的元素在当前堆栈以外）则返回1，否则返回0。
*/
#define lua_isnone(L,n)		(lua_type(L, (n)) == LUA_TNONE)

/**
lua_isnoneornil

[-0, +0, -] 

int lua_isnoneornil (lua_State *L, int index);

Returns 1 if the given acceptable index is not valid (that is, it refers to an element outside the current stack) or if the value at this index is nil, and 0 otherwise. 

如果所给可接受索引处的值不可用（即引用的元素在当前堆栈以外）或者索引处的值为nil则返回1，否则返回0。
*/
#define lua_isnoneornil(L, n)	(lua_type(L, (n)) <= 0)

/**
lua_pushliteral

[-0, +1, m] 

void lua_pushliteral (lua_State *L, const char *s);

This macro is equivalent to lua_pushlstring, but can be used only when s is a literal string. In these cases, it automatically provides the string length. 

本宏等价于lua_pushlstring，但是只能当s是字面字符串时使用。在这些情况下，它自动地提供字符串长度。 
*/
#define lua_pushliteral(L, s)	\
	lua_pushlstring(L, "" s, (sizeof(s)/sizeof(char))-1)

/**
lua_setglobal

[-1, +0, e] 

void lua_setglobal (lua_State *L, const char *name);

Pops a value from the stack and sets it as the new value of global name. It is defined as a macro: 

#define lua_setglobal(L,s)   lua_setfield(L, LUA_GLOBALSINDEX, s)

从栈中弹出一个值并将其设为全局变量name的新值。它被定义为宏： 

#define lua_setglobal(L,s)   lua_setfield(L, LUA_GLOBALSINDEX, s)
*/
#define lua_setglobal(L,s)	lua_setfield(L, LUA_GLOBALSINDEX, (s))

/**
lua_getglobal

[-0, +1, e] 

void lua_getglobal (lua_State *L, const char *name);

Pushes onto the stack the value of the global name. It is defined as a macro: 

#define lua_getglobal(L,s)  lua_getfield(L, LUA_GLOBALSINDEX, s)

将全局变量name的值压栈。它被定义为宏： 

#define lua_getglobal(L,s)  lua_getfield(L, LUA_GLOBALSINDEX, s)
*/
#define lua_getglobal(L,s)	lua_getfield(L, LUA_GLOBALSINDEX, (s))

/**
lua_tostring

[-0, +0, m] 

const char *lua_tostring (lua_State *L, int index);

Equivalent to lua_tolstring with len equal to NULL. 

等价于len等于NULL时的lua_tolstring。 
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

调试的hook函数类型。 

当hook被调用时，其ar参数将其字段event设为触发该hook的特定事件。Lua用下面的常量标识这些事件：LUA_HOOKCALL、LUA_HOOKRET、LUA_HOOKTAILRET、LUA_HOOKLINE和LUA_HOOKCOUNT。此外，对于行事件，也会设置字段currentline。要得到ar中其他字段的值，hook必须调用lua_getinfo。对于返回事件，event可以是普通值LUA_HOOKRET或LUA_HOOKTAILRET。对于后者，Lua将模拟从执行了尾调用的函数中的返回；这种情况下，调用lua_getinfo是无用的。 

当Lua运行hook时，它禁止其他的hook调用。因此，如果hook回调Lua来执行函数或单元，该次执行不带任何hook调用。 
*/
typedef void (*lua_Hook) (lua_State *L, lua_Debug *ar);

/**
lua_getstack

[-0, +0, -] 

int lua_getstack (lua_State *L, int level, lua_Debug *ar);

Get information about the interpreter runtime stack. 

This function fills parts of a lua_Debug structure with an identification of the activation record of the function executing at a given level. Level 0 is the current running function, whereas level n+1 is the function that has called level n. When there are no errors, lua_getstack returns 1; when called with a level greater than the stack depth, it returns 0. 

得到关于解释器运行时的栈信息。 

本函数用执行于给定级别的函数的活动记录的标识符填充部分lua_Debug结构。0级是当前运行的函数，而n+1级是调用n级的函数。无错误时lua_getstack返回1；当以超过栈深度的级别调用时返回0。 
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

返回关于特定函数或函数调用的信息。

要获得函数调用的信息，参数ar必须是被前一个调用lua_getstack填充的合法的激活记录，或者是作为参数传给hook（参考lua_Hook）。

为了获得函数信息，你可以把它压入堆栈然后启动带字符'>'开头的字符串what。

（这种情况下，lua_getinfo会弹出栈顶的函数。）

例如，为了指导哪一行定义了函数f，你可以写如下代码：

lua_Debug ar;

lua_getfield(L, LUA_GLOBALSINDEX, "f");   获取全局'f' 

lua_getinfo(L, ">S", &ar);

printf("%d\n", ar.linedefined);

字符串what的每个字符选择了结构体ar要填充的域或者是压入堆栈的值：

'n': 填充域name和namewhat； 

'S': 填充域source, short_src, linedefined, lastlinedefined, 和what； 

'l': 填充域currentline； 

'u': 填充域nups；

'f': 把运行于所给层的函数压入栈顶； 

'L': 把一个索引是函数合法行号的表压入堆栈（一个合法行是带有关联代码的行，所以一行中可以放置一个断点。不合法的行包括空行和注释。）

这个函数在错误时返回0（例如，what的一个不合法选项）
*/
LUA_API int lua_getinfo (lua_State *L, const char *what, lua_Debug *ar);

/**
lua_getlocal

[-0, +(0|1), -] 

const char *lua_getlocal (lua_State *L, lua_Debug *ar, int n);

Gets information about a local variable of a given activation record. The parameter ar must be a valid activation record that was filled by a previous call to lua_getstack or given as argument to a hook (see lua_Hook). The index n selects which local variable to inspect (1 is the first parameter or active local variable, and so on, until the last active local variable). lua_getlocal pushes the variable's value onto the stack and returns its name. 

Variable names starting with '(' (open parentheses) represent internal variables (loop control variables, temporaries, and C function locals). 

Returns NULL (and pushes nothing) when the index is greater than the number of active local variables. 

获得给定的活动记录的一个局部变量的信息。参数ar必须是有效的活动记录，它被之前的lua_getstack调用填充，或者作为hook的参数（见lua_Hook）。索引n选取要检阅哪个局部变量（1是第一个参数或活动的局部变量，以此类推，直到最后一个活动的局部变量）。lua_getlocal将变量的值压栈并返回其名字。 

‘(’（开圆括号）开头的变量表示内部变量（循环控制变量、临时变量和C函数局部变量）。 

当索引比活动的局部变量的数量大时返回NULL（且不压栈任何东西）。 
*/
LUA_API const char *lua_getlocal (lua_State *L, const lua_Debug *ar, int n);

/**
lua_setlocal

[-(0|1), +0, -] 

const char *lua_setlocal (lua_State *L, lua_Debug *ar, int n);

Sets the value of a local variable of a given activation record. Parameters ar and n are as in lua_getlocal (see lua_getlocal). lua_setlocal assigns the value at the top of the stack to the variable and returns its name. It also pops the value from the stack. 

Returns NULL (and pops nothing) when the index is greater than the number of active local variables. 

设置给定的活动记录的一个局部变量的值。参数ar和n与lua_getlocal中一样（见lua_getlocal）。lua_setlocal把栈顶的值赋给变量并返回其名字。它也将值从栈中弹出。 

当索引超过活动的局部变量的数量时返回NULL（且不出栈任何东西）。 
*/
LUA_API const char *lua_setlocal (lua_State *L, const lua_Debug *ar, int n);

/**
lua_getupvalue

[-0, +(0|1), -] 

const char *lua_getupvalue (lua_State *L, int funcindex, int n);

Gets information about a closure's upvalue. (For Lua functions, upvalues are the external local variables that the function uses, and that are consequently included in its closure.) lua_getupvalue gets the index n of an upvalue, pushes the upvalue's value onto the stack, and returns its name. funcindex points to the closure in the stack. (Upvalues have no particular order, as they are active through the whole function. So, they are numbered in an arbitrary order.) 

Returns NULL (and pushes nothing) when the index is greater than the number of upvalues. For C functions, this function uses the empty string "" as a name for all upvalues. 

得到闭包的一个upvalue的信息。（对于Lua函数，upvalue是函数用到因而被包含在其闭包内的外部局部变量。）lua_getupvalue得到索引n处的upvalue，将其值压栈并返回其名。funcindex指向栈中的闭包。（upvalue没有特定的顺序，因为它们在整个函数中从头至尾都是活动的，所以它们以任意顺序被编号。） 

当索引比upvalue的数量大时返回NULL（且不压栈任何东西）。对于C函数，本函数用空串""作为所有upvalue的名字。 
*/
LUA_API const char *lua_getupvalue (lua_State *L, int funcindex, int n);

/**
lua_setupvalue

[-(0|1), +0, -] 

const char *lua_setupvalue (lua_State *L, int funcindex, int n);

Sets the value of a closure's upvalue. It assigns the value at the top of the stack to the upvalue and returns its name. It also pops the value from the stack. Parameters funcindex and n are as in the lua_getupvalue (see lua_getupvalue). 

Returns NULL (and pops nothing) when the index is greater than the number of upvalues. 

设置闭包的upvalue的值。它把栈顶的值赋给upvalue并返回其名字。它也把值从栈中弹出。参数funcindex和n同lua_getupvalue中一样（见lua_getupvalue）。 

当索引超过upvalue的数量时返回NULL（且不出栈任何东西）。 
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

设置调试的hook函数。 

参数f是hook函数。mask规定发生哪个事件时hook将被调用：

它由常量LUA_MASKCALL、LUA_MASKRET、LUA_MASKLINE和LUA_MASKCOUNT按位或组成。

只当掩码含有LUA_MASKCOUNT时，count参数才有意义。

对于每个事件，hook如下面解释的那样被调用： 

call hook: 当解释器调用函数时被调用。hook在Lua刚进入新函数后、
		   在函数得到其参数前被调用。 

return hook: 当解释器从函数返回时被调用。hook在Lua正要离开函数前被调用。
			 你不能访问要被函数返回的值。 

line hook: 当解释器将要开始新行代码的执行或跳回到代码中（甚至是同一行）时被调用。
		  （该事件只在Lua正执行Lua函数时发生。） 

count hook: 在解释器执行每count条指令后被调用。（该事件只在Lua正执行Lua函数时发生。） 
			通过设定mask为0禁用hook。
*/
LUA_API int lua_sethook (lua_State *L, lua_Hook func, int mask, int count);

/**
lua_gethook

[-0, +0, -] 

lua_Hook lua_gethook (lua_State *L);

Returns the current hook function. 

返回当前钩子函数。 
*/
LUA_API lua_Hook lua_gethook (lua_State *L);

/**
lua_gethookmask

[-0, +0, -] 

int lua_gethookmask (lua_State *L);

Returns the current hook mask. 

返回当前钩子掩码。 
*/
LUA_API int lua_gethookmask (lua_State *L);

/**
lua_gethookcount

[-0, +0, -] 

int lua_gethookcount (lua_State *L);

Returns the current hook count. 

返回当前钩子个数
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

用于持有关于活动函数的信息的不同事项的结构。lua_getstack只填充本结构的专用部分，留作后用。要用有用的信息填充lua_Debug的其他字段，调用lua_getinfo。 

lua_Debug的字段具有下面的含义： 

source: 如果函数在字符串中定义，则source就是那个字符串。如果函数在文件中定义，则source以‘@’开头后跟文件名。 

short_src: 一个source的“可打印”版本，将用于错误消息中。 

linedefined: 函数定义开始的行号。 

lastlinedefined: 函数定义结束的行号。 

what: 如果函数是Lua函数则为字符串"Lua"，如果是C函数则为"C"，如果是单元的主体部分则为"main"，并且如果是执行了尾调用的函数则为 "tail"。最后一种情况，Lua没有关于函数的其他信息。 

currentline: 给定函数正执行的当前行。当没有行信息可用时，currentline被设为-1。 

name: 给定函数的合适的名字。因为Lua函数是第一类值，所以它们没有固定的名字：一些函数可为多个全局变量的值，然而其他的可只存储于一个表字段中。函数lua_getinfo检查函数是如何被调用的以找到合适的名字。如果找不到，则name被设为NULL。 

namewhat: 解释name字段。依据函数被如何调用，namewhat的值可为"global"、"local"、"method"、"field"、"upvalue"或""（空字符串）。（当看似没有其他选项可用时Lua使用空串。） 

nups: 函数的upvalue的数量。 
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
