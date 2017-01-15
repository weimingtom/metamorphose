/*
** $Id: lapi.c,v 2.55.1.5 2008/07/04 18:41:18 roberto Exp $
** Lua API
** See Copyright Notice in lua.h
*/


#include <assert.h>
#include <math.h>
#include <stdarg.h>
#include <string.h>

#define lapi_c
#define LUA_CORE

#include "lua.h"

#include "lapi.h"
#include "ldebug.h"
#include "ldo.h"
#include "lfunc.h"
#include "lgc.h"
#include "lmem.h"
#include "lobject.h"
#include "lstate.h"
#include "lstring.h"
#include "ltable.h"
#include "ltm.h"
#include "lundump.h"
#include "lvm.h"



const char lua_ident[] =
  "$Lua: " LUA_RELEASE " " LUA_COPYRIGHT " $\n"
  "$Authors: " LUA_AUTHORS " $\n"
  "$URL: www.lua.org $\n";



#define api_checknelems(L, n)	api_check(L, (n) <= (L->top - L->base))

#define api_checkvalidindex(L, i)	api_check(L, (i) != luaO_nilobject)

#define api_incr_top(L)   {api_check(L, L->top < L->ci->top); L->top++;}



static TValue *index2adr (lua_State *L, int idx) {
  if (idx > 0) {
    TValue *o = L->base + (idx - 1);
    api_check(L, idx <= L->ci->top - L->base);
    if (o >= L->top) return cast(TValue *, luaO_nilobject);
    else return o;
  }
  else if (idx > LUA_REGISTRYINDEX) {
    api_check(L, idx != 0 && -idx <= L->top - L->base);
    return L->top + idx;
  }
  else switch (idx) {  /* pseudo-indices */
    case LUA_REGISTRYINDEX: return registry(L);
    case LUA_ENVIRONINDEX: {
      Closure *func = curr_func(L);
      sethvalue(L, &L->env, func->c.env);
      return &L->env;
    }
    case LUA_GLOBALSINDEX: return gt(L);
    default: {
      Closure *func = curr_func(L);
      idx = LUA_GLOBALSINDEX - idx;
      return (idx <= func->c.nupvalues)
                ? &func->c.upvalue[idx-1]
                : cast(TValue *, luaO_nilobject);
    }
  }
}


static Table *getcurrenv (lua_State *L) {
  if (L->ci == L->base_ci)  /* no enclosing function? */
    return hvalue(gt(L));  /* use global table as environment */
  else {
    Closure *func = curr_func(L);
    return func->c.env;
  }
}


void luaA_pushobject (lua_State *L, const TValue *o) {
  setobj2s(L, L->top, o);
  api_incr_top(L);
}

/*
lua_checkstack
[-0, +0, m] 
int lua_checkstack (lua_State *L, int extra);
Ensures that there are at least extra free stack slots in the stack. It returns false if it cannot grow the stack to that size. This function never shrinks the stack; if the stack is already larger than the new size, it is left unchanged. 

lua_checkstack
[-0, +0, m] 
int lua_checkstack (lua_State *L, int extra);
确保栈中存在至少extra个空闲栈槽位。如果栈不能增长到那个尺寸则返回假。本函数从不缩小栈；如果栈已经比新尺寸大则无变化。 
*/
LUA_API int lua_checkstack (lua_State *L, int size) {
  int res = 1;
  lua_lock(L);
  if (size > LUAI_MAXCSTACK || (L->top - L->base + size) > LUAI_MAXCSTACK)
    res = 0;  /* stack overflow */
  else if (size > 0) {
    luaD_checkstack(L, size);
    if (L->ci->top < L->top + size)
      L->ci->top = L->top + size;
  }
  lua_unlock(L);
  return res;
}

/*
lua_xmove
[-?, +?, -] 
	void lua_xmove (lua_State *from, lua_State *to, int n);
	Exchange values between different threads of the same global state. 
	This function pops n values from the stack from, and pushes them onto the stack to. 

lua_xmove
[-?, +?, -] 
	void lua_xmove (lua_State *from, lua_State *to, int n);
	交换相同全局状态中不同线程的值。
	这个函数从堆栈from中弹出n个值，然后把它们压入堆栈to。
*/
LUA_API void lua_xmove (lua_State *from, lua_State *to, int n) {
  int i;
  if (from == to) return;
  lua_lock(to);
  api_checknelems(from, n);
  api_check(from, G(from) == G(to));
  api_check(from, to->ci->top - to->top >= n);
  from->top -= n;
  for (i = 0; i < n; i++) {
    setobj2s(to, to->top++, from->top + i);
  }
  lua_unlock(to);
}


LUA_API void lua_setlevel (lua_State *from, lua_State *to) {
  to->nCcalls = from->nCcalls;
}

/*
lua_atpanic
[-0, +0, -]
	lua_CFunction lua_atpanic (lua_State *L, lua_CFunction panicf);
	Sets a new panic function and returns the old one. 
	If an error happens outside any protected environment, Lua calls a panic function and then calls exit(EXIT_FAILURE), thus exiting the host application. Your panic function can avoid this exit by never returning (e.g., doing a long jump). 
	The panic function can access the error message at the top of the stack. 

lua_atpanic
[-0, +0, -] 
	lua_CFunction lua_atpanic (lua_State *L, lua_CFunction panicf);
	设置新的应急（panic）函数并返回前一个。 
	如果在任何受保护的环境外面发生了错误，Lua调用应急函数接着调用exit(EXIT_FAILURE)，从而退出宿主程序。你的应急函数可通过永不返回（例如执行一次长跳转）以避免这次退出。 
	应急函数可访问栈顶的错误消息。 
*/
LUA_API lua_CFunction lua_atpanic (lua_State *L, lua_CFunction panicf) {
  lua_CFunction old;
  lua_lock(L);
  old = G(L)->panic;
  G(L)->panic = panicf;
  lua_unlock(L);
  return old;
}

/*
lua_newthread
[-0, +1, m] 
	lua_State *lua_newthread (lua_State *L);
	Creates a new thread, pushes it on the stack, and returns a pointer to a lua_State that represents this new thread. The new state returned by this function shares with the original state all global objects (such as tables), but has an independent execution stack. 
	There is no explicit function to close or to destroy a thread. Threads are subject to garbage collection, like any Lua object. 

lua_newthread
[-0, +1, m] 
	lua_State *lua_newthread (lua_State *L);
	创建新线程，将其压栈，并返回指向lua_State的指针，它表示该新线程。本函数返回的新状态机与初始状态机共享所有全局对象（例如表），但具有独立的执行栈。 
	没有关闭或销毁线程的显式函数。像任何Lua对象一样，线程受垃圾收集的支配。 
*/
LUA_API lua_State *lua_newthread (lua_State *L) {
  lua_State *L1;
  lua_lock(L);
  luaC_checkGC(L);
  L1 = luaE_newthread(L);
  setthvalue(L, L->top, L1);
  api_incr_top(L);
  lua_unlock(L);
  luai_userstatethread(L, L1);
  return L1;
}



/*
** basic stack manipulation
*/

/*
lua_gettop
[-0, +0, -] 
	int lua_gettop (lua_State *L);
	Returns the index of the top element in the stack. Because indices start at 1, this result is equal to the number of elements in the stack (and so 0 means an empty stack). 

lua_gettop
[-0, +0, -] 
	int lua_gettop (lua_State *L);
	返回栈顶元素的索引。因为索引从1开始，该结果等于栈中元素的数量（所以0表示空栈）。 
*/
LUA_API int lua_gettop (lua_State *L) {
  return cast_int(L->top - L->base);
}

/*
lua_settop
[-?, +?, -] 
	void lua_settop (lua_State *L, int index);
	Accepts any acceptable index, or 0, and sets the stack top to this index. If the new top is larger than the old one, then the new elements are filled with nil. If index is 0, then all stack elements are removed. 

lua_settop
[-?, +?, -] 
	void lua_settop (lua_State *L, int index);
	接受任意可接受的索引，或者0，把栈顶设置在此索引上。
	如果新的栈顶大于原有的，那么新创建的元素被填充为空。
	如果索引为0，那么所有堆栈元素都会被删除。 
*/
LUA_API void lua_settop (lua_State *L, int idx) {
  lua_lock(L);
  if (idx >= 0) {
    api_check(L, idx <= L->stack_last - L->base);
    while (L->top < L->base + idx)
      setnilvalue(L->top++);
    L->top = L->base + idx;
  }
  else {
    api_check(L, -(idx+1) <= (L->top - L->base));
    L->top += idx+1;  /* `subtract' index (index is negative) */
  }
  lua_unlock(L);
}

/*
lua_remove
[-1, +0, -] 
	void lua_remove (lua_State *L, int index);
	Removes the element at the given valid index, shifting down the elements above this index to fill the gap. Cannot be called with a pseudo-index, because a pseudo-index is not an actual stack position. 

lua_remove
[-1, +0, -] 
	void lua_remove (lua_State *L, int index);
	移除给定的有效索引处的元素，并将该索引上面的元素下移来填充空隙。不能用伪索引调用，因为伪索引不是真实的栈位置。 
*/
LUA_API void lua_remove (lua_State *L, int idx) {
  StkId p;
  lua_lock(L);
  p = index2adr(L, idx);
  api_checkvalidindex(L, p);
  while (++p < L->top) setobjs2s(L, p-1, p);
  L->top--;
  lua_unlock(L);
}

/*
lua_insert
[-1, +1, -] 
	void lua_insert (lua_State *L, int index);
	Moves the top element into the given valid index, shifting up the elements above this index to open space. Cannot be called with a pseudo-index, because a pseudo-index is not an actual stack position. 

lua_insert
[-1, +1, -] 
	void lua_insert (lua_State *L, int index);
	将栈顶元素移入给定的有效索引，并将该索引上面的元素上移至开放空间。不能以伪索引调用，因为伪索引不是真实的栈位置。 
*/
LUA_API void lua_insert (lua_State *L, int idx) {
  StkId p;
  StkId q;
  lua_lock(L);
  p = index2adr(L, idx);
  api_checkvalidindex(L, p);
  for (q = L->top; q>p; q--) setobjs2s(L, q, q-1);
  setobjs2s(L, p, L->top);
  lua_unlock(L);
}

/*
lua_replace
[-1, +0, -] 
	void lua_replace (lua_State *L, int index);
	Moves the top element into the given position (and pops it), without shifting any element (therefore replacing the value at the given position). 

lua_replace
[-1, +0, -] 
	void lua_replace (lua_State *L, int index);
	将栈顶元素移动到给定的索引中（并弹出它），不会移动任何元素（因此替换给定位置的值）。 
*/
LUA_API void lua_replace (lua_State *L, int idx) {
  StkId o;
  lua_lock(L);
  /* explicit test for incompatible code */
  if (idx == LUA_ENVIRONINDEX && L->ci == L->base_ci)
    luaG_runerror(L, "no calling environment");
  api_checknelems(L, 1);
  o = index2adr(L, idx);
  api_checkvalidindex(L, o);
  if (idx == LUA_ENVIRONINDEX) {
    Closure *func = curr_func(L);
    api_check(L, ttistable(L->top - 1)); 
    func->c.env = hvalue(L->top - 1);
    luaC_barrier(L, func, L->top - 1);
  }
  else {
    setobj(L, o, L->top - 1);
    if (idx < LUA_GLOBALSINDEX)  /* function upvalue? */
      luaC_barrier(L, curr_func(L), L->top - 1);
  }
  L->top--;
  lua_unlock(L);
}

/*
lua_pushvalue
[-0, +1, -] 
	void lua_pushvalue (lua_State *L, int index);
	Pushes a copy of the element at the given valid index onto the stack. 

lua_pushvalue
[-0, +1, -] 
	void lua_pushvalue (lua_State *L, int index);
	把所给合法index处的元素的拷贝压入栈内。
*/
LUA_API void lua_pushvalue (lua_State *L, int idx) {
  lua_lock(L);
  setobj2s(L, L->top, index2adr(L, idx));
  api_incr_top(L);
  lua_unlock(L);
}



/*
** access functions (stack -> C)
*/

/*
lua_type
[-0, +0, -] 
	int lua_type (lua_State *L, int index);
	Returns the type of the value in the given acceptable index, or LUA_TNONE for a non-valid index (that is, an index to an "empty" stack position). The types returned by lua_type are coded by the following constants defined in lua.h: LUA_TNIL, LUA_TNUMBER, LUA_TBOOLEAN, LUA_TSTRING, LUA_TTABLE, LUA_TFUNCTION, LUA_TUSERDATA, LUA_TTHREAD, and LUA_TLIGHTUSERDATA. 

lua_type
[-0, +0, -] 
	int lua_type (lua_State *L, int index);
	返回给定的认可的索引处的值的类型，或者对不合法的索引返回LUA_TNONE（即指向“空”栈位置的索引）。lua_type返回的类型在lua.h中定义，被编码为下面的常量： LUA_TNIL、LUA_TNUMBER、LUA_TBOOLEAN、LUA_TSTRING、LUA_TTABLE、LUA_TFUNCTION、LUA_TUSERDATA、LUA_TTHREAD和LUA_TLIGHTUSERDATA。 
*/
LUA_API int lua_type (lua_State *L, int idx) {
  StkId o = index2adr(L, idx);
  return (o == luaO_nilobject) ? LUA_TNONE : ttype(o);
}

/*
lua_typename
[-0, +0, -] 
	const char *lua_typename  (lua_State *L, int tp);
	Returns the name of the type encoded by the value tp, which must be one the values returned by lua_type. 

lua_typename
[-0, +0, -] 
	const char *lua_typename  (lua_State *L, int tp);
	返回由值tp编码的类型名，tp必须是lua_type的返回值的其中一个。 
*/
LUA_API const char *lua_typename (lua_State *L, int t) {
  UNUSED(L);
  return (t == LUA_TNONE) ? "no value" : luaT_typenames[t];
}

/*
lua_iscfunction
[-0, +0, -] 
	int lua_iscfunction (lua_State *L, int index);
	Returns 1 if the value at the given acceptable index is a C function, and 0 otherwise. 

lua_iscfunction
[-0, +0, -] 
	int lua_iscfunction (lua_State *L, int index);
	如果所给可接受索引处的值为C函数则返回1，否则返回0。
*/
LUA_API int lua_iscfunction (lua_State *L, int idx) {
  StkId o = index2adr(L, idx);
  return iscfunction(o);
}

/*
lua_isnumber
[-0, +0, -] 
	int lua_isnumber (lua_State *L, int index);
	Returns 1 if the value at the given acceptable index is a number or a string convertible to a number, and 0 otherwise. 

lua_isnumber
[-0, +0, -] 
	int lua_isnumber (lua_State *L, int index);
	如果所给可接受索引处的值是数或者可转换为数的字符串则返回1，否则返回0。
*/
LUA_API int lua_isnumber (lua_State *L, int idx) {
  TValue n;
  const TValue *o = index2adr(L, idx);
  return tonumber(o, &n);
}

/*
lua_isstring
[-0, +0, -] 
	int lua_isstring (lua_State *L, int index);
	Returns 1 if the value at the given acceptable index is a string or a number (which is always convertible to a string), and 0 otherwise. 

lua_isstring
[-0, +0, -] 
	int lua_isstring (lua_State *L, int index);
	如果所给可接受索引处的值是字符串或者数（总是可以转换为字符串）则返回1，否则返回0。
*/
LUA_API int lua_isstring (lua_State *L, int idx) {
  int t = lua_type(L, idx);
  return (t == LUA_TSTRING || t == LUA_TNUMBER);
}

/*
lua_isuserdata
[-0, +0, -] 
	int lua_isuserdata (lua_State *L, int index);
	Returns 1 if the value at the given acceptable index is a userdata (either full or light), and 0 otherwise. 

lua_isuserdata
[-0, +0, -] 
	int lua_isuserdata (lua_State *L, int index);
	如果所给可接受索引处的值是一个userdata（完全或者轻量级）则返回1，否则返回0。
*/
LUA_API int lua_isuserdata (lua_State *L, int idx) {
  const TValue *o = index2adr(L, idx);
  return (ttisuserdata(o) || ttislightuserdata(o));
}

/*
lua_rawequal
[-0, +0, -] 
	int lua_rawequal (lua_State *L, int index1, int index2);
	Returns 1 if the two values in acceptable indices index1 and index2 are primitively equal (that is, without calling metamethods). Otherwise returns 0. Also returns 0 if any of the indices are non valid. 

lua_rawequal
[-0, +0, -] 
	int lua_rawequal (lua_State *L, int index1, int index2);
	如果两个所接收的索引index1和index2处的值原生相等（即不调用元方法），则返回1。
	否则，返回0。
	如果任意索引不合法，也返回0。
*/
LUA_API int lua_rawequal (lua_State *L, int index1, int index2) {
  StkId o1 = index2adr(L, index1);
  StkId o2 = index2adr(L, index2);
  return (o1 == luaO_nilobject || o2 == luaO_nilobject) ? 0
         : luaO_rawequalObj(o1, o2);
}

/*
lua_equal
[-0, +0, e] 
int lua_equal (lua_State *L, int index1, int index2);
Returns 1 if the two values in acceptable indices index1 and index2 are equal, following the semantics of the Lua == operator (that is, may call metamethods). Otherwise returns 0. Also returns 0 if any of the indices is non valid. 

lua_equal
[-0, +0, e] 
int lua_equal (lua_State *L, int index1, int index2);
沿用Lua的==操作符的语义（即可能调用元方法），比较在可接受索引index1和index2中的两个值，如果相等则返回1。否则返回0。如果任何索引无效也返回0。 
*/
LUA_API int lua_equal (lua_State *L, int index1, int index2) {
  StkId o1, o2;
  int i;
  lua_lock(L);  /* may call tag method */
  o1 = index2adr(L, index1);
  o2 = index2adr(L, index2);
  i = (o1 == luaO_nilobject || o2 == luaO_nilobject) ? 0 : equalobj(L, o1, o2);
  lua_unlock(L);
  return i;
}

/*
lua_lessthan
[-0, +0, e] 
	int lua_lessthan (lua_State *L, int index1, int index2);
	Returns 1 if the value at acceptable index index1 is smaller than the value at acceptable index index2, following the semantics of the Lua < operator (that is, may call metamethods). Otherwise returns 0. Also returns 0 if any of the indices is non valid. 

lua_lessthan
[-0, +0, e] 
	int lua_lessthan (lua_State *L, int index1, int index2);
	如果所给可接受索引index1处的值小于索引index2处的值则返回1，遵循Lua的<运算符的语义（即，可能调用元方法）。
	否则返回0。
	如果任意索引不合法也会返回0。
*/
LUA_API int lua_lessthan (lua_State *L, int index1, int index2) {
  StkId o1, o2;
  int i;
  lua_lock(L);  /* may call tag method */
  o1 = index2adr(L, index1);
  o2 = index2adr(L, index2);
  i = (o1 == luaO_nilobject || o2 == luaO_nilobject) ? 0
       : luaV_lessthan(L, o1, o2);
  lua_unlock(L);
  return i;
}

/*
lua_tonumber
[-0, +0, -] 
	lua_Number lua_tonumber (lua_State *L, int index);
	Converts the Lua value at the given acceptable index to the C type lua_Number (see lua_Number). The Lua value must be a number or a string convertible to a number (see §2.2.1); otherwise, lua_tonumber returns 0. 

lua_tonumber
[-0, +0, -] 
	lua_Number lua_tonumber (lua_State *L, int index);
	把所给可接受索引处的值转换为C类型lua_Number（参考lua_Number）。
	Lua值必须为一个数或可以转换为数的字符串（参考§2.2.1）；否则，lua_tonumber返回0。
*/
LUA_API lua_Number lua_tonumber (lua_State *L, int idx) {
  TValue n;
  const TValue *o = index2adr(L, idx);
  if (tonumber(o, &n))
    return nvalue(o);
  else
    return 0;
}

/*
lua_tointeger
[-0, +0, -] 
	lua_Integer lua_tointeger (lua_State *L, int index);
	Converts the Lua value at the given acceptable index to the signed integral type lua_Integer. The Lua value must be a number or a string convertible to a number (see §2.2.1); otherwise, lua_tointeger returns 0. 
	If the number is not an integer, it is truncated in some non-specified way. 

lua_tointeger
[-0, +0, -] 
	lua_Integer lua_tointeger (lua_State *L, int index);
	把所给可接受索引处的值转换为带符号的整型lua_Integer。
	Lua值必须是一个数或者可转为数的字符串（参考§2.2.1），否则，lua_tointeger返回0。
	如果数不是整数，会以不确定的方式被剪切。
*/
LUA_API lua_Integer lua_tointeger (lua_State *L, int idx) {
  TValue n;
  const TValue *o = index2adr(L, idx);
  if (tonumber(o, &n)) {
    lua_Integer res;
    lua_Number num = nvalue(o);
    lua_number2integer(res, num);
    return res;
  }
  else
    return 0;
}

/*
lua_toboolean
[-0, +0, -] 
	int lua_toboolean (lua_State *L, int index);
	Converts the Lua value at the given acceptable index to a C boolean value (0 or 1). Like all tests in Lua, lua_toboolean returns 1 for any Lua value different from false and nil; otherwise it returns 0. It also returns 0 when called with a non-valid index. (If you want to accept only actual boolean values, use lua_isboolean to test the value's type.) 

lua_toboolean
[-0, +0, -] 
	int lua_toboolean (lua_State *L, int index);
	把所给可接受的索引处的Lua值转换为C的布尔值(0或1)。
	好像Lua的所有测试那样，对于任何不是false和nil的值lua_toboolean返回1；否则返回0。
	它在用非法索引调用时也返回0。
	（如果你只想接收实际的布尔值，使用lua_isboolean来检查值类型）
*/
LUA_API int lua_toboolean (lua_State *L, int idx) {
  const TValue *o = index2adr(L, idx);
  return !l_isfalse(o);
}

/*
lua_tolstring
[-0, +0, m] 
	const char *lua_tolstring (lua_State *L, int index, size_t *len);
	Converts the Lua value at the given acceptable index to a C string. If len is not NULL, it also sets *len with the string length. The Lua value must be a string or a number; otherwise, the function returns NULL. If the value is a number, then lua_tolstring also changes the actual value in the stack to a string. (This change confuses lua_next when lua_tolstring is applied to keys during a table traversal.) 
	lua_tolstring returns a fully aligned pointer to a string inside the Lua state. This string always has a zero ('\0') after its last character (as in C), but can contain other zeros in its body. Because Lua has garbage collection, there is no guarantee that the pointer returned by lua_tolstring will be valid after the corresponding value is removed from the stack. 

lua_tolstring
[-0, +0, m] 
	const char *lua_tolstring (lua_State *L, int index, size_t *len);
	把所给可接受索引处的值转换为C的字符串。
	如果len不是NULL，也可以设置*len为字符串长度。
	Lua值不许是一个字符串或数。否则，这个函数返回NULL。
	如果值是数，那么lua_tolstring还会改变堆栈的实际值为字符串。
	（当lua_tolstring应用到表遍历的键中，这种改变会混淆lua_next。）
	lua_tolstring返回一个完全对齐的指向Lua状态内部的字符串的指针。
	因为Lua有垃圾回收，所以不保证lua_tolstring所返回的指针在相应值从堆栈中删除后仍合法。
*/
LUA_API const char *lua_tolstring (lua_State *L, int idx, size_t *len) {
  StkId o = index2adr(L, idx);
  if (!ttisstring(o)) {
    lua_lock(L);  /* `luaV_tostring' may create a new string */
    if (!luaV_tostring(L, o)) {  /* conversion failed? */
      if (len != NULL) *len = 0;
      lua_unlock(L);
      return NULL;
    }
    luaC_checkGC(L);
    o = index2adr(L, idx);  /* previous call may reallocate the stack */
    lua_unlock(L);
  }
  if (len != NULL) *len = tsvalue(o)->len;
  return svalue(o);
}

/*
lua_objlen
[-0, +0, -] 
	size_t lua_objlen (lua_State *L, int index);
	Returns the "length" of the value at the given acceptable index: for strings, this is the string length; for tables, this is the result of the length operator ('#'); for userdata, this is the size of the block of memory allocated for the userdata; for other values, it is 0. 

lua_objlen
[-0, +0, -] 
	size_t lua_objlen (lua_State *L, int index);
	返回给定的认可的索引处的值的“长度”：对于字符串，这是其长度；对于表，这是取长操作符（‘#’）的结果；对于用户数据，这是为其分配的内存块的尺寸；对于其他类型是0。 
*/
LUA_API size_t lua_objlen (lua_State *L, int idx) {
  StkId o = index2adr(L, idx);
  switch (ttype(o)) {
    case LUA_TSTRING: return tsvalue(o)->len;
    case LUA_TUSERDATA: return uvalue(o)->len;
    case LUA_TTABLE: return luaH_getn(hvalue(o));
    case LUA_TNUMBER: {
      size_t l;
      lua_lock(L);  /* `luaV_tostring' may create a new string */
      l = (luaV_tostring(L, o) ? tsvalue(o)->len : 0);
      lua_unlock(L);
      return l;
    }
    default: return 0;
  }
}

/*
lua_tocfunction
[-0, +0, -] 
	lua_CFunction lua_tocfunction (lua_State *L, int index);
	Converts a value at the given acceptable index to a C function. That value must be a C function; otherwise, returns NULL. 

lua_tocfunction
[-0, +0, -] 
	lua_CFunction lua_tocfunction (lua_State *L, int index);
	把所给可接受索引处的值转换为C函数。
	那个值必须是C函数，否则返回NULL。
*/
LUA_API lua_CFunction lua_tocfunction (lua_State *L, int idx) {
  StkId o = index2adr(L, idx);
  return (!iscfunction(o)) ? NULL : clvalue(o)->c.f;
}

/*
lua_touserdata
[-0, +0, -] 
	void *lua_touserdata (lua_State *L, int index);
	If the value at the given acceptable index is a full userdata, returns its block address. If the value is a light userdata, returns its pointer. Otherwise, returns NULL. 

lua_touserdata[-0, +0, -]
	如果给定的认可的索引处的值是完整的用户数据，则返回其块地址。
	如果是轻型用户数据，返回其指针。否则返回NULL。 
*/
LUA_API void *lua_touserdata (lua_State *L, int idx) {
  StkId o = index2adr(L, idx);
  switch (ttype(o)) {
    case LUA_TUSERDATA: return (rawuvalue(o) + 1);
    case LUA_TLIGHTUSERDATA: return pvalue(o);
    default: return NULL;
  }
}

/*
lua_tothread
[-0, +0, -] 
	lua_State *lua_tothread (lua_State *L, int index);
	Converts the value at the given acceptable index to a Lua thread (represented as lua_State*). This value must be a thread; otherwise, the function returns NULL. 

lua_tothread
[-0, +0, -] 
	lua_State *lua_tothread (lua_State *L, int index);
	把给定的认可的索引处的值转换为Lua线程（用lua_State*表示）。该值必须为线程；否则，本函数返回NULL。 
*/
LUA_API lua_State *lua_tothread (lua_State *L, int idx) {
  StkId o = index2adr(L, idx);
  return (!ttisthread(o)) ? NULL : thvalue(o);
}

/*
lua_topointer
[-0, +0, -] 
	const void *lua_topointer (lua_State *L, int index);
	Converts the value at the given acceptable index to a generic C pointer (void*). The value can be a userdata, a table, a thread, or a function; otherwise, lua_topointer returns NULL. Different objects will give different pointers. There is no way to convert the pointer back to its original value. 
	Typically this function is used only for debug information. 

lua_topointer
[-0, +0, -] 
	const void *lua_topointer (lua_State *L, int index);
	把所给可接受索引处的值转换为泛型的C指针（void*）。
	这个值可以是用户定义数据，表，线程，或者是函数；否则，lua_topointer返回NULL。
	不同的对象会给出不同的指针。
	没有方法可以把指针转换回它原有值。
	特别地这个函数仅用于调试信息。
*/
LUA_API const void *lua_topointer (lua_State *L, int idx) {
  StkId o = index2adr(L, idx);
  switch (ttype(o)) {
    case LUA_TTABLE: return hvalue(o);
    case LUA_TFUNCTION: return clvalue(o);
    case LUA_TTHREAD: return thvalue(o);
    case LUA_TUSERDATA:
    case LUA_TLIGHTUSERDATA:
      return lua_touserdata(L, idx);
    default: return NULL;
  }
}



/*
** push functions (C -> stack)
*/

/*
lua_pushnil
[-0, +1, -] 
	void lua_pushnil (lua_State *L);
	Pushes a nil value onto the stack. 

lua_pushnil
[-0, +1, -] 
	void lua_pushnil (lua_State *L);
	压入一个空值进堆栈。 
*/
LUA_API void lua_pushnil (lua_State *L) {
  lua_lock(L);
  setnilvalue(L->top);
  api_incr_top(L);
  lua_unlock(L);
}

/*
lua_pushnumber
[-0, +1, -] 
	void lua_pushnumber (lua_State *L, lua_Number n);
	Pushes a number with value n onto the stack. 

lua_pushnumber
[-0, +1, -] 
	void lua_pushnumber (lua_State *L, lua_Number n);
	压入一个拥有值n的数字进堆栈。
*/
LUA_API void lua_pushnumber (lua_State *L, lua_Number n) {
  lua_lock(L);
  setnvalue(L->top, n);
  api_incr_top(L);
  lua_unlock(L);
}

/*
lua_pushinteger
[-0, +1, -] 
	void lua_pushinteger (lua_State *L, lua_Integer n);
	Pushes a number with value n onto the stack. 

lua_pushinteger
[-0, +1, -] 
	void lua_pushinteger (lua_State *L, lua_Integer n);
	将一个值为n的数字压栈。 
*/
LUA_API void lua_pushinteger (lua_State *L, lua_Integer n) {
  lua_lock(L);
  setnvalue(L->top, cast_num(n));
  api_incr_top(L);
  lua_unlock(L);
}

/*
lua_pushlstring
[-0, +1, m] 
	void lua_pushlstring (lua_State *L, const char *s, size_t len);
	Pushes the string pointed to by s with size len onto the stack. Lua makes (or reuses) an internal copy of the given string, so the memory at s can be freed or reused immediately after the function returns. The string can contain embedded zeros. 

lua_pushlstring
[-0, +1, m] 
	void lua_pushlstring (lua_State *L, const char *s, size_t len);
	将s指向的尺寸为len的字符串压栈。Lua制造（或重用）给定字符串的内部拷贝，所以函数返回后s的内存立刻可被释放或重用。字符串可含有内嵌的0。 
*/
LUA_API void lua_pushlstring (lua_State *L, const char *s, size_t len) {
  lua_lock(L);
  luaC_checkGC(L);
  setsvalue2s(L, L->top, luaS_newlstr(L, s, len));
  api_incr_top(L);
  lua_unlock(L);
}

/*
lua_pushstring
[-0, +1, m] 
	void lua_pushstring (lua_State *L, const char *s);
	Pushes the zero-terminated string pointed to by s onto the stack. Lua makes (or reuses) an internal copy of the given string, so the memory at s can be freed or reused immediately after the function returns. The string cannot contain embedded zeros; it is assumed to end at the first zero. 

lua_pushstring
[-0, +1, m]
void lua_pushstring (lua_State *L, const char *s);
	将s指向的以0结尾的字符串压栈。Lua制造（或重用）给定字符串的内部拷贝，
	所以函数返回后s的内存立刻可被释放或重用。字符串不可含有内嵌的0；
	假定它在首个0处结束。 
*/
LUA_API void lua_pushstring (lua_State *L, const char *s) {
  if (s == NULL)
    lua_pushnil(L);
  else
    lua_pushlstring(L, s, strlen(s));
}

/*
lua_pushvfstring
[-0, +1, m] 
	const char *lua_pushvfstring (lua_State *L,
								  const char *fmt,
								  va_list argp);
	Equivalent to lua_pushfstring, except that it receives a va_list instead of a variable number of arguments. 

lua_pushvfstring
[-0, +1, m] 
	const char *lua_pushvfstring (lua_State *L,
								  const char *fmt,
								  va_list argp);
	等效于lua_pushfstring，除了接收一个va_list以代替参数个数。
*/
LUA_API const char *lua_pushvfstring (lua_State *L, const char *fmt,
                                      va_list argp) {
  const char *ret;
  lua_lock(L);
  luaC_checkGC(L);
  ret = luaO_pushvfstring(L, fmt, argp);
  lua_unlock(L);
  return ret;
}

/*
lua_pushfstring
[-0, +1, m] 
	const char *lua_pushfstring (lua_State *L, const char *fmt, ...);
	Pushes onto the stack a formatted string and returns a pointer to this string. It is similar to the C function sprintf, but has some important differences: 
	You do not have to allocate space for the result: the result is a Lua string and Lua takes care of memory allocation (and deallocation, through garbage collection). 
	The conversion specifiers are quite restricted. There are no flags, widths, or precisions. The conversion specifiers can only be '%%' (inserts a '%' in the string), '%s' (inserts a zero-terminated string, with no size restrictions), '%f' (inserts a lua_Number), '%p' (inserts a pointer as a hexadecimal numeral), '%d' (inserts an int), and '%c' (inserts an int as a character). 

lua_pushfstring
[-0, +1, m] 
	const char *lua_pushfstring (lua_State *L, const char *fmt, ...);
	将格式化的字符串压栈并返回指向它的指针。它与C函数sprintf类似，但也有一些重要的区别： 
	你不需要为结果分配空间：结果是Lua字符串且Lua会照看内存分配（以及通过垃圾收集解除分配）。 
	转换说明符非常的有限。没有标记、宽度或精度。转换说明符只能是'%%' （在字符串中插入一个'%'），'%s' （插入一个以0结尾的字符串，没有尺寸限制），'%f' （插入一个lua_Number），'%p' （插入一个指针作为十六进制数），'%d' （插入一个int），以及'%c' （插入一个int作为字符）。 
*/
LUA_API const char *lua_pushfstring (lua_State *L, const char *fmt, ...) {
  const char *ret;
  va_list argp;
  lua_lock(L);
  luaC_checkGC(L);
  va_start(argp, fmt);
  ret = luaO_pushvfstring(L, fmt, argp);
  va_end(argp);
  lua_unlock(L);
  return ret;
}

/*
lua_pushcclosure
[-n, +1, m] 
	void lua_pushcclosure (lua_State *L, lua_CFunction fn, int n);
	Pushes a new C closure onto the stack. 
	When a C function is created, it is possible to associate some values with it, thus creating a C closure (see §3.4); these values are then accessible to the function whenever it is called. To associate values with a C function, first these values should be pushed onto the stack (when there are multiple values, the first value is pushed first). Then lua_pushcclosure is called to create and push the C function onto the stack, with the argument n telling how many values should be associated with the function. lua_pushcclosure also pops these values from the stack. 
	The maximum value for n is 255. 

lua_pushcclosure
[-n, +1, m] 
	void lua_pushcclosure (lua_State *L, lua_CFunction fn, int n);
	把新的C闭包压栈。 
	当C函数被创建时，它可以把一些值与自己关联，这样就创建了C闭包（见§3.4）；接下来无论何时它被调用，这些值对该函数都是可访问的。要将值与C函数关联，首先这些值应当被压栈（当有多个值时第一个值首先压栈）。然后用参数n调用lua_pushcclosure来创建C函数并将其压栈，n表明应当把多少值关联到该函数。lua_pushcclosure也会将这些值从栈中弹出。 
	n的最大值是255。 
*/
LUA_API void lua_pushcclosure (lua_State *L, lua_CFunction fn, int n) {
  Closure *cl;
  lua_lock(L);
  luaC_checkGC(L);
  api_checknelems(L, n);
  cl = luaF_newCclosure(L, n, getcurrenv(L));
  cl->c.f = fn;
  L->top -= n;
  while (n--)
    setobj2n(L, &cl->c.upvalue[n], L->top+n);
  setclvalue(L, L->top, cl);
  lua_assert(iswhite(obj2gco(cl)));
  api_incr_top(L);
  lua_unlock(L);
}

/*
lua_pushboolean
[-0, +1, -] 
	void lua_pushboolean (lua_State *L, int b);
	Pushes a boolean value with value b onto the stack. 

lua_pushboolean
[-0, +1, -] 
	void lua_pushboolean (lua_State *L, int b);
	把值b作为布尔值压栈。 
*/
LUA_API void lua_pushboolean (lua_State *L, int b) {
  lua_lock(L);
  setbvalue(L->top, (b != 0));  /* ensure that true is 1 */
  api_incr_top(L);
  lua_unlock(L);
}

/*
lua_pushlightuserdata
[-0, +1, -] 
	void lua_pushlightuserdata (lua_State *L, void *p);
	Pushes a light userdata onto the stack. 
	Userdata represent C values in Lua. A light userdata represents a pointer. It is a value (like a number): you do not create it, it has no individual metatable, and it is not collected (as it was never created). A light userdata is equal to "any" light userdata with the same C address. 

lua_pushlightuserdata
[-0, +1, -] 
	void lua_pushlightuserdata (lua_State *L, void *p);
	将轻型用户数据压栈。 
	在Lua中用户数据表示C值。轻型用户数据表示一个指针。它是个值（就像数字）：你不用创建它，它没有单独的元表，而且它不会被回收（如同从不被创建）。带有相同的C地址的轻型用户数据相等。 
*/
LUA_API void lua_pushlightuserdata (lua_State *L, void *p) {
  lua_lock(L);
  setpvalue(L->top, p);
  api_incr_top(L);
  lua_unlock(L);
}

/*
lua_pushthread
[-0, +1, -] 
	int lua_pushthread (lua_State *L);
	Pushes the thread represented by L onto the stack. Returns 1 if this thread is the main thread of its state. 

lua_pushthread
[-0, +1, -] 
	int lua_pushthread (lua_State *L);
	把L代表的线程压入栈中。
	如果这个线程是状态的主线程则返回1。
*/
LUA_API int lua_pushthread (lua_State *L) {
  lua_lock(L);
  setthvalue(L, L->top, L);
  api_incr_top(L);
  lua_unlock(L);
  return (G(L)->mainthread == L);
}



/*
** get functions (Lua -> stack)
*/

/*
lua_gettable
[-1, +1, e] 
void lua_gettable (lua_State *L, int index);
Pushes onto the stack the value t[k], where t is the value at the given valid index and k is the value at the top of the stack. 
This function pops the key from the stack (putting the resulting value in its place). As in Lua, this function may trigger a metamethod for the "index" event (see §2.8). 

lua_gettable
[-1, +1, e] 
void lua_gettable (lua_State *L, int index);
将值t[k]压栈，其中t是指定的有效索引处的值，并且k是栈顶的值。 
本函数将键出栈（将结果值放在它的位置）。同Lua中一样，本函数可能触发用于“index”事件的元方法（见§2.8）。 
*/
LUA_API void lua_gettable (lua_State *L, int idx) {
  StkId t;
  lua_lock(L);
  t = index2adr(L, idx);
  api_checkvalidindex(L, t);
  luaV_gettable(L, t, L->top - 1, L->top - 1);
  lua_unlock(L);
}

/*
lua_getfield
[-0, +1, e] 
void lua_getfield (lua_State *L, int index, const char *k);
Pushes onto the stack the value t[k], where t is the value at the given valid index. As in Lua, this function may trigger a metamethod for the "index" event (see §2.8). 

lua_getfield
[-0, +1, e] 
void lua_getfield (lua_State *L, int index, const char *k);
将t[k]的值压栈，其中t是给定的有效索引处的值。同Lua中一样，本函数可能触发用于“index”事件的元方法（见§2.8）。 
*/
LUA_API void lua_getfield (lua_State *L, int idx, const char *k) {
  StkId t;
  TValue key;
  lua_lock(L);
  t = index2adr(L, idx);
  api_checkvalidindex(L, t);
  setsvalue(L, &key, luaS_new(L, k));
  luaV_gettable(L, t, &key, L->top);
  api_incr_top(L);
  lua_unlock(L);
}

/*
lua_rawget
[-1, +1, -] 
	void lua_rawget (lua_State *L, int index);
	Similar to lua_gettable, but does a raw access (i.e., without metamethods). 

lua_rawget
[-1, +1, -] 
	void lua_rawget (lua_State *L, int index);
	类似于lua_gettable，但执行原生访问(例如，没有元方法） 
*/
LUA_API void lua_rawget (lua_State *L, int idx) {
  StkId t;
  lua_lock(L);
  t = index2adr(L, idx);
  api_check(L, ttistable(t));
  setobj2s(L, L->top - 1, luaH_get(hvalue(t), L->top - 1));
  lua_unlock(L);
}

/*
lua_rawgeti
[-0, +1, -] 
	void lua_rawgeti (lua_State *L, int index, int n);
	Pushes onto the stack the value t[n], where t is the value at the given valid index. The access is raw; that is, it does not invoke metamethods. 

lua_rawgeti
[-0, +1, -] 
	void lua_rawgeti (lua_State *L, int index, int n);
	把t[n]的值压入栈中，其中t是所给合法Index处的值。
	访问是原生的，不执行元方法
*/
LUA_API void lua_rawgeti (lua_State *L, int idx, int n) {
  StkId o;
  lua_lock(L);
  o = index2adr(L, idx);
  api_check(L, ttistable(o));
  setobj2s(L, L->top, luaH_getnum(hvalue(o), n));
  api_incr_top(L);
  lua_unlock(L);
}

/*
lua_createtable
[-0, +1, m] 
void lua_createtable (lua_State *L, int narr, int nrec);
Creates a new empty table and pushes it onto the stack. The new table has space pre-allocated for narr array elements and nrec non-array elements. This pre-allocation is useful when you know exactly how many elements the table will have. Otherwise you can use the function lua_newtable. 

lua_createtable
[-0, +1, m] 
void lua_createtable (lua_State *L, int narr, int nrec);
创建新的空表并将其压栈。新表预分配narr个数组元素和nrec个非数组元素的空闲空间。当你确切地知道表将由多少个元素时，预分配是非常有用的。否则，你可用函数lua_newtable。 
*/
LUA_API void lua_createtable (lua_State *L, int narray, int nrec) {
  lua_lock(L);
  luaC_checkGC(L);
  sethvalue(L, L->top, luaH_new(L, narray, nrec));
  api_incr_top(L);
  lua_unlock(L);
}

/*
lua_getmetatable
[-0, +(0|1), -] 
int lua_getmetatable (lua_State *L, int index);
Pushes onto the stack the metatable of the value at the given acceptable index. If the index is not valid, or if the value does not have a metatable, the function returns 0 and pushes nothing on the stack. 

lua_getmetatable
[-0, +(0|1), -] 
int lua_getmetatable (lua_State *L, int index);
将给定的认可的索引处的值的元表压栈。如果索引无效，或者如果该值没有元表，本函数返回0且不会压栈任何东西。 
*/
LUA_API int lua_getmetatable (lua_State *L, int objindex) {
  const TValue *obj;
  Table *mt = NULL;
  int res;
  lua_lock(L);
  obj = index2adr(L, objindex);
  switch (ttype(obj)) {
    case LUA_TTABLE:
      mt = hvalue(obj)->metatable;
      break;
    case LUA_TUSERDATA:
      mt = uvalue(obj)->metatable;
      break;
    default:
      mt = G(L)->mt[ttype(obj)];
      break;
  }
  if (mt == NULL)
    res = 0;
  else {
    sethvalue(L, L->top, mt);
    api_incr_top(L);
    res = 1;
  }
  lua_unlock(L);
  return res;
}

/*
lua_getfenv
[-0, +1, -] 
void lua_getfenv (lua_State *L, int index);
Pushes onto the stack the environment table of the value at the given index. 

lua_getfenv
[-0, +1, -] 
void lua_getfenv (lua_State *L, int index);
将给定索引处的值的环境表压栈。 
*/
LUA_API void lua_getfenv (lua_State *L, int idx) {
  StkId o;
  lua_lock(L);
  o = index2adr(L, idx);
  api_checkvalidindex(L, o);
  switch (ttype(o)) {
    case LUA_TFUNCTION:
      sethvalue(L, L->top, clvalue(o)->c.env);
      break;
    case LUA_TUSERDATA:
      sethvalue(L, L->top, uvalue(o)->env);
      break;
    case LUA_TTHREAD:
      setobj2s(L, L->top,  gt(thvalue(o)));
      break;
    default:
      setnilvalue(L->top);
      break;
  }
  api_incr_top(L);
  lua_unlock(L);
}


/*
** set functions (stack -> Lua)
*/

/*
lua_settable
[-2, +0, e] 
	void lua_settable (lua_State *L, int index);
	Does the equivalent to t[k] = v, where t is the value at the given valid index, v is the value at the top of the stack, and k is the value just below the top. 
	This function pops both the key and the value from the stack. As in Lua, this function may trigger a metamethod for the "newindex" event (see §2.8). 

lua_settable
[-2, +0, e] 
	void lua_settable (lua_State *L, int index);
	执行t[k] = v的等价操作，其中t是给定的有效索引处的值，v是栈顶的值，k正好是栈顶下面的值。 
	本函数将键和值都弹出栈。同Lua中一样，本函数可能触发“newindex”事件的元方法（见§2.8）。 
*/
LUA_API void lua_settable (lua_State *L, int idx) {
  StkId t;
  lua_lock(L);
  api_checknelems(L, 2);
  t = index2adr(L, idx);
  api_checkvalidindex(L, t);
  luaV_settable(L, t, L->top - 2, L->top - 1);
  L->top -= 2;  /* pop index and value */
  lua_unlock(L);
}

/*
lua_setfield
[-1, +0, e] 
	void lua_setfield (lua_State *L, int index, const char *k);
	Does the equivalent to t[k] = v, where t is the value at the given valid index and v is the value at the top of the stack. 
	This function pops the value from the stack. As in Lua, this function may trigger a metamethod for the "newindex" event (see §2.8). 

lua_setfield
[-1, +0, e] 
	void lua_setfield (lua_State *L, int index, const char *k);
	执行t[k] = v的等价操作，其中t是给定的有效索引处的值，v是栈顶的值。 
	本函数从栈中弹出值。同Lua中一样，本函数可能触发“newindex”事件的元方法（见§2.8）。 
*/
LUA_API void lua_setfield (lua_State *L, int idx, const char *k) {
  StkId t;
  TValue key;
  lua_lock(L);
  api_checknelems(L, 1);
  t = index2adr(L, idx);
  api_checkvalidindex(L, t);
  setsvalue(L, &key, luaS_new(L, k));
  luaV_settable(L, t, &key, L->top - 1);
  L->top--;  /* pop value */
  lua_unlock(L);
}

/*
lua_rawset
[-2, +0, m] 
	void lua_rawset (lua_State *L, int index);
	Similar to lua_settable, but does a raw assignment (i.e., without metamethods). 

lua_rawset
[-2, +0, m] 
	void lua_rawset (lua_State *L, int index);
	类似lua_settable，但是执行一次原生赋值（也就是不用元方法）。 
*/
LUA_API void lua_rawset (lua_State *L, int idx) {
  StkId t;
  lua_lock(L);
  api_checknelems(L, 2);
  t = index2adr(L, idx);
  api_check(L, ttistable(t));
  setobj2t(L, luaH_set(L, hvalue(t), L->top-2), L->top-1);
  luaC_barriert(L, hvalue(t), L->top-1);
  L->top -= 2;
  lua_unlock(L);
}

/*
lua_rawseti
[-1, +0, m] 
	void lua_rawseti (lua_State *L, int index, int n);
	Does the equivalent of t[n] = v, where t is the value at the given valid index and v is the value at the top of the stack. 
	This function pops the value from the stack. The assignment is raw; that is, it does not invoke metamethods. 

lua_rawseti
[-1, +0, m] 
	void lua_rawseti (lua_State *L, int index, int n);
	执行t[n] = v的等价操作，其中t是给定的有效索引处的值，v是栈顶的值。 
	本函数将值出栈。赋值是原生的；即不调用元方法。 
*/
LUA_API void lua_rawseti (lua_State *L, int idx, int n) {
  StkId o;
  lua_lock(L);
  api_checknelems(L, 1);
  o = index2adr(L, idx);
  api_check(L, ttistable(o));
  setobj2t(L, luaH_setnum(L, hvalue(o), n), L->top-1);
  luaC_barriert(L, hvalue(o), L->top-1);
  L->top--;
  lua_unlock(L);
}

/*
lua_setmetatable
[-1, +0, -] 
	int lua_setmetatable (lua_State *L, int index);
	Pops a table from the stack and sets it as the new metatable for the value at the given acceptable index. 

lua_setmetatable
[-1, +0, -] 
	int lua_setmetatable (lua_State *L, int index);
	从栈中弹出一个表并将其设为给定的认可的索引处的值的新元表。 
*/
LUA_API int lua_setmetatable (lua_State *L, int objindex) {
  TValue *obj;
  Table *mt;
  lua_lock(L);
  api_checknelems(L, 1);
  obj = index2adr(L, objindex);
  api_checkvalidindex(L, obj);
  if (ttisnil(L->top - 1))
    mt = NULL;
  else {
    api_check(L, ttistable(L->top - 1));
    mt = hvalue(L->top - 1);
  }
  switch (ttype(obj)) {
    case LUA_TTABLE: {
      hvalue(obj)->metatable = mt;
      if (mt)
        luaC_objbarriert(L, hvalue(obj), mt);
      break;
    }
    case LUA_TUSERDATA: {
      uvalue(obj)->metatable = mt;
      if (mt)
        luaC_objbarrier(L, rawuvalue(obj), mt);
      break;
    }
    default: {
      G(L)->mt[ttype(obj)] = mt;
      break;
    }
  }
  L->top--;
  lua_unlock(L);
  return 1;
}

/*
lua_setfenv
[-1, +0, -] 
	int lua_setfenv (lua_State *L, int index);
	Pops a table from the stack and sets it as the new environment for the value at the given index. If the value at the given index is neither a function nor a thread nor a userdata, lua_setfenv returns 0. Otherwise it returns 1. 

lua_setfenv
[-1, +0, -] 
	int lua_setfenv (lua_State *L, int index);
	从栈中弹出一个表并把它设为给定索引处的值的新环境。如果给定索引处的值既不是函数又不是线程也不是用户数据，lua_setfenv返回0。否则返回1。 
*/
LUA_API int lua_setfenv (lua_State *L, int idx) {
  StkId o;
  int res = 1;
  lua_lock(L);
  api_checknelems(L, 1);
  o = index2adr(L, idx);
  api_checkvalidindex(L, o);
  api_check(L, ttistable(L->top - 1));
  switch (ttype(o)) {
    case LUA_TFUNCTION:
      clvalue(o)->c.env = hvalue(L->top - 1);
      break;
    case LUA_TUSERDATA:
      uvalue(o)->env = hvalue(L->top - 1);
      break;
    case LUA_TTHREAD:
      sethvalue(L, gt(thvalue(o)), hvalue(L->top - 1));
      break;
    default:
      res = 0;
      break;
  }
  if (res) luaC_objbarrier(L, gcvalue(o), hvalue(L->top - 1));
  L->top--;
  lua_unlock(L);
  return res;
}


/*
** `load' and `call' functions (run Lua code)
*/


#define adjustresults(L,nres) \
    { if (nres == LUA_MULTRET && L->top >= L->ci->top) L->ci->top = L->top; }


#define checkresults(L,na,nr) \
     api_check(L, (nr) == LUA_MULTRET || (L->ci->top - L->top >= (nr) - (na)))
	
/*
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

lua_call
[-(nargs + 1), +nresults, e] 
void lua_call (lua_State *L, int nargs, int nresults);
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
LUA_API void lua_call (lua_State *L, int nargs, int nresults) {
  StkId func;
  lua_lock(L);
  api_checknelems(L, nargs+1);
  checkresults(L, nargs, nresults);
  func = L->top - (nargs+1);
  luaD_call(L, func, nresults);
  adjustresults(L, nresults);
  lua_unlock(L);
}



/*
** Execute a protected call.
*/
struct CallS {  /* data to `f_call' */
  StkId func;
  int nresults;
};


static void f_call (lua_State *L, void *ud) {
  struct CallS *c = cast(struct CallS *, ud);
  luaD_call(L, c->func, c->nresults);
}

/*
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

lua_pcall
[-(nargs + 1), +(nresults|1), -]
int lua_pcall (lua_State *L, int nargs, int nresults, int errfunc);
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
LUA_API int lua_pcall (lua_State *L, int nargs, int nresults, int errfunc) {
  struct CallS c;
  int status;
  ptrdiff_t func;
  lua_lock(L);
  api_checknelems(L, nargs+1);
  checkresults(L, nargs, nresults);
  if (errfunc == 0)
    func = 0;
  else {
    StkId o = index2adr(L, errfunc);
    api_checkvalidindex(L, o);
    func = savestack(L, o);
  }
  c.func = L->top - (nargs+1);  /* function to be called */
  c.nresults = nresults;
  status = luaD_pcall(L, f_call, &c, savestack(L, c.func), func);
  adjustresults(L, nresults);
  lua_unlock(L);
  return status;
}


/*
** Execute a protected C call.
*/
struct CCallS {  /* data to `f_Ccall' */
  lua_CFunction func;
  void *ud;
};


static void f_Ccall (lua_State *L, void *ud) {
  struct CCallS *c = cast(struct CCallS *, ud);
  Closure *cl;
  cl = luaF_newCclosure(L, 0, getcurrenv(L));
  cl->c.f = c->func;
  setclvalue(L, L->top, cl);  /* push function */
  api_incr_top(L);
  setpvalue(L->top, c->ud);  /* push only argument */
  api_incr_top(L);
  luaD_call(L, L->top - 2, 0);
}

/*
lua_cpcall
[-0, +(0|1), -] 
	int lua_cpcall (lua_State *L, lua_CFunction func, void *ud);
	Calls the C function func in protected mode. func starts with only one element in its stack, a light userdata containing ud. In case of errors, lua_cpcall returns the same error codes as lua_pcall, plus the error object on the top of the stack; otherwise, it returns zero, and does not change the stack. All values returned by func are discarded. 

lua_cpcall [-0, +(0|1), -]
	以保护模式调用C函数func。
	func的栈中只有一个元素，是个包含ud的轻量级用户数据。
	发生错误时，lua_cpcall返回同lua_pcall一样的错误代码，以及栈顶的错误对象；
	否则返回0且不改变栈。func返回的所有值被丢弃。

注：第三个参数ud是userdata，可以在C函数中用lua_touserdata()取出。
*/
LUA_API int lua_cpcall (lua_State *L, lua_CFunction func, void *ud) {
  struct CCallS c;
  int status;
  lua_lock(L);
  c.func = func;
  c.ud = ud;
  status = luaD_pcall(L, f_Ccall, &c, savestack(L, L->top), 0);
  lua_unlock(L);
  return status;
}

/*
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

lua_load
[-0, +1, -] 
int lua_load (lua_State *L,
              lua_Reader reader,
              void *data,
              const char *chunkname);
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
LUA_API int lua_load (lua_State *L, lua_Reader reader, void *data,
                      const char *chunkname) {
  ZIO z;
  int status;
  lua_lock(L);
  if (!chunkname) chunkname = "?";
  luaZ_init(L, &z, reader, data);
  status = luaD_protectedparser(L, &z, chunkname);
  lua_unlock(L);
  return status;
}

/*
lua_dump
[-0, +0, m] 
int lua_dump (lua_State *L, lua_Writer writer, void *data);
Dumps a function as a binary chunk. Receives a Lua function on the top of the stack and produces a binary chunk that, if loaded again, results in a function equivalent to the one dumped. As it produces parts of the chunk, lua_dump calls function writer (see lua_Writer) with the given data to write them. 
The value returned is the error code returned by the last call to the writer; 0 means no errors. 
This function does not pop the Lua function from the stack. 

lua_dump
[-0, +0, m] 
int lua_dump (lua_State *L, lua_Writer writer, void *data);
将函数转储为二进制代码单元。接收栈顶的Lua函数并产生二进制单元，如果后者被再次加载，得到与被转储的等价的函数。当产生单元的各部分时，lua_dump用给定的data调用函数writer（见lua_Writer）来写出它们。 
返回值是最后一次调用记录器（writer）返回的错误代码；0表示没有错误。 
本函数不会将Lua函数从栈中弹出。 
*/
LUA_API int lua_dump (lua_State *L, lua_Writer writer, void *data) {
  int status;
  TValue *o;
  lua_lock(L);
  api_checknelems(L, 1);
  o = L->top - 1;
  if (isLfunction(o))
    status = luaU_dump(L, clvalue(o)->l.p, writer, data, 0);
  else
    status = 1;
  lua_unlock(L);
  return status;
}

/*
lua_status
[-0, +0, -] 
	int lua_status (lua_State *L);
	Returns the status of the thread L. 
	The status can be 0 for a normal thread, an error code if the thread finished its execution with an error, or LUA_YIELD if the thread is suspended. 

lua_status
[-0, +0, -] 
	int lua_status (lua_State *L);
	返回线程L的状态值。
	线程正常时状态值为0，线程遇到错误而结束它的执行则返回一个错误码，线程挂起（暂停）则返回LUA_YIELD。
*/
LUA_API int  lua_status (lua_State *L) {
  return L->status;
}


/*
** Garbage-collection function
*/

/*
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

lua_gc[-0, +0, e]
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
LUA_API int lua_gc (lua_State *L, int what, int data) {
  int res = 0;
  global_State *g;
  lua_lock(L);
  g = G(L);
  switch (what) {
    case LUA_GCSTOP: {
      g->GCthreshold = MAX_LUMEM;
      break;
    }
    case LUA_GCRESTART: {
      g->GCthreshold = g->totalbytes;
      break;
    }
    case LUA_GCCOLLECT: {
      luaC_fullgc(L);
      break;
    }
    case LUA_GCCOUNT: {
      /* GC values are expressed in Kbytes: #bytes/2^10 */
      res = cast_int(g->totalbytes >> 10);
      break;
    }
    case LUA_GCCOUNTB: {
      res = cast_int(g->totalbytes & 0x3ff);
      break;
    }
    case LUA_GCSTEP: {
      lu_mem a = (cast(lu_mem, data) << 10);
      if (a <= g->totalbytes)
        g->GCthreshold = g->totalbytes - a;
      else
        g->GCthreshold = 0;
      while (g->GCthreshold <= g->totalbytes) {
        luaC_step(L);
        if (g->gcstate == GCSpause) {  /* end of cycle? */
          res = 1;  /* signal it */
          break;
        }
      }
      break;
    }
    case LUA_GCSETPAUSE: {
      res = g->gcpause;
      g->gcpause = data;
      break;
    }
    case LUA_GCSETSTEPMUL: {
      res = g->gcstepmul;
      g->gcstepmul = data;
      break;
    }
    default: res = -1;  /* invalid option */
  }
  lua_unlock(L);
  return res;
}



/*
** miscellaneous functions
*/

/*
lua_error
[-1, +0, v] 
int lua_error (lua_State *L);
Generates a Lua error. The error message (which can actually be a Lua value of any type) must be on the stack top. This function does a long jump, and therefore never returns. (see luaL_error). 

lua_error
[-1, +0, v] 
int lua_error (lua_State *L);
产生一个Lua错误。错误消息（实际上可为任何类型的Lua值）必须在栈顶。本函数执行长跳转，因此从不返回。（见luaL_error）。 
*/
LUA_API int lua_error (lua_State *L) {
  lua_lock(L);
  api_checknelems(L, 1);
  luaG_errormsg(L);
  lua_unlock(L);
  return 0;  /* to avoid warnings */
}

/*
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

lua_next
[-1, +(2|0), e] 
int lua_next (lua_State *L, int index);
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
LUA_API int lua_next (lua_State *L, int idx) {
  StkId t;
  int more;
  lua_lock(L);
  t = index2adr(L, idx);
  api_check(L, ttistable(t));
  more = luaH_next(L, hvalue(t), L->top - 1);
  if (more) {
    api_incr_top(L);
  }
  else  /* no more elements */
    L->top -= 1;  /* remove key */
  lua_unlock(L);
  return more;
}

/*
lua_concat
[-n, +1, e] 
	void lua_concat (lua_State *L, int n);
	Concatenates the n values at the top of the stack, pops them, and leaves the result at the top. If n is 1, the result is the single value on the stack (that is, the function does nothing); if n is 0, the result is the empty string. Concatenation is performed following the usual semantics of Lua (see §2.5.4). 

lua_concat
[-n, +1, e] 
	void lua_concat (lua_State *L, int n);
	连接栈顶的n个值，弹出它们并将结果留在栈顶。如果n是1，
	结果就是栈上的单个值（即函数什么也不做）；如果n是0，结果是空字符串。
	连接操作依照Lua的常规语义执行（见§2.5.4）。 
*/
LUA_API void lua_concat (lua_State *L, int n) {
  lua_lock(L);
  api_checknelems(L, n);
  if (n >= 2) {
    luaC_checkGC(L);
    luaV_concat(L, n, cast_int(L->top - L->base) - 1);
    L->top -= (n-1);
  }
  else if (n == 0) {  /* push empty string */
    setsvalue2s(L, L->top, luaS_newlstr(L, "", 0));
    api_incr_top(L);
  }
  /* else n == 1; nothing to do */
  lua_unlock(L);
}

/*
lua_getallocf
[-0, +0, -] 
lua_Alloc lua_getallocf (lua_State *L, void **ud);
Returns the memory-allocation function of a given state. If ud is not NULL, Lua stores in *ud the opaque pointer passed to lua_newstate. 

lua_getallocf
[-0, +0, -] 
lua_Alloc lua_getallocf (lua_State *L, void **ud);
返回给定状态机的内存分配函数。如果ud不为NULL，Lua将lua_newstate传入的不透明指针存入*ud。 
*/
LUA_API lua_Alloc lua_getallocf (lua_State *L, void **ud) {
  lua_Alloc f;
  lua_lock(L);
  if (ud) *ud = G(L)->ud;
  f = G(L)->frealloc;
  lua_unlock(L);
  return f;
}

/*
lua_setallocf
[-0, +0, -] 
	void lua_setallocf (lua_State *L, lua_Alloc f, void *ud);
	Changes the allocator function of a given state to f with user data ud. 

lua_setallocf
[-0, +0, -] 
	void lua_setallocf (lua_State *L, lua_Alloc f, void *ud);
	把给定状态机的分配器函数换成带用户数据ud的f。 
*/
LUA_API void lua_setallocf (lua_State *L, lua_Alloc f, void *ud) {
  lua_lock(L);
  G(L)->ud = ud;
  G(L)->frealloc = f;
  lua_unlock(L);
}

/*
lua_newuserdata
[-0, +1, m] 
	void *lua_newuserdata (lua_State *L, size_t size);
	This function allocates a new block of memory with the given size, pushes onto the stack a new full userdata with the block address, and returns this address. 
	Userdata represent C values in Lua. A full userdata represents a block of memory. It is an object (like a table): you must create it, it can have its own metatable, and you can detect when it is being collected. A full userdata is only equal to itself (under raw equality). 
	When Lua collects a full userdata with a gc metamethod, Lua calls the metamethod and marks the userdata as finalized. When this userdata is collected again then Lua frees its corresponding memory. 

lua_newuserdata
[-0, +1, m] 
	void *lua_newuserdata (lua_State *L, size_t size);
	本函数分配新的给定尺寸的内存块，以块地址的方式将完整的用户数据压栈，并返回该地址。 
	Lua中的用户数据表示C值。完整的用户数据表示一块内存。它是个对象（如同表）：你必须创建它，它可以有自己的元表，而且当被收集时能被检测到。完整的用户数据只等于自己（依照原始的相等比较）。 
	当Lua用gc元方法收集完整的用户数据时，Lua调用该元方法并把用户数据标记为完成的。当该用户数据再次被收集时，Lua释放其对应的内存。 
*/
LUA_API void *lua_newuserdata (lua_State *L, size_t size) {
  Udata *u;
  lua_lock(L);
  luaC_checkGC(L);
  u = luaS_newudata(L, size, getcurrenv(L));
  setuvalue(L, L->top, u);
  api_incr_top(L);
  lua_unlock(L);
  return u + 1;
}




static const char *aux_upvalue (StkId fi, int n, TValue **val) {
  Closure *f;
  if (!ttisfunction(fi)) return NULL;
  f = clvalue(fi);
  if (f->c.isC) {
    if (!(1 <= n && n <= f->c.nupvalues)) return NULL;
    *val = &f->c.upvalue[n-1];
    return "";
  }
  else {
    Proto *p = f->l.p;
    if (!(1 <= n && n <= p->sizeupvalues)) return NULL;
    *val = f->l.upvals[n-1]->v;
    return getstr(p->upvalues[n-1]);
  }
}

/*
lua_getupvalue
[-0, +(0|1), -] 
	const char *lua_getupvalue (lua_State *L, int funcindex, int n);
	Gets information about a closure's upvalue. (For Lua functions, upvalues are the external local variables that the function uses, and that are consequently included in its closure.) lua_getupvalue gets the index n of an upvalue, pushes the upvalue's value onto the stack, and returns its name. funcindex points to the closure in the stack. (Upvalues have no particular order, as they are active through the whole function. So, they are numbered in an arbitrary order.) 
	Returns NULL (and pushes nothing) when the index is greater than the number of upvalues. For C functions, this function uses the empty string "" as a name for all upvalues. 

lua_getupvalue
[-0, +(0|1), -] 
	const char *lua_getupvalue (lua_State *L, int funcindex, int n);
	得到闭包的一个upvalue的信息。（对于Lua函数，upvalue是函数用到因而被包含在其闭包内的外部局部变量。）lua_getupvalue得到索引n处的upvalue，将其值压栈并返回其名。funcindex指向栈中的闭包。（upvalue没有特定的顺序，因为它们在整个函数中从头至尾都是活动的，所以它们以任意顺序被编号。） 
	当索引比upvalue的数量大时返回NULL（且不压栈任何东西）。对于C函数，本函数用空串""作为所有upvalue的名字。 
*/
LUA_API const char *lua_getupvalue (lua_State *L, int funcindex, int n) {
  const char *name;
  TValue *val;
  lua_lock(L);
  name = aux_upvalue(index2adr(L, funcindex), n, &val);
  if (name) {
    setobj2s(L, L->top, val);
    api_incr_top(L);
  }
  lua_unlock(L);
  return name;
}

/*
lua_setupvalue
[-(0|1), +0, -] 
	const char *lua_setupvalue (lua_State *L, int funcindex, int n);
	Sets the value of a closure's upvalue. It assigns the value at the top of the stack to the upvalue and returns its name. It also pops the value from the stack. Parameters funcindex and n are as in the lua_getupvalue (see lua_getupvalue). 
	Returns NULL (and pops nothing) when the index is greater than the number of upvalues. 

lua_setupvalue
[-(0|1), +0, -] 
	const char *lua_setupvalue (lua_State *L, int funcindex, int n);
	设置闭包的upvalue的值。它把栈顶的值赋给upvalue并返回其名字。它也把值从栈中弹出。参数funcindex和n同lua_getupvalue中一样（见lua_getupvalue）。 
	当索引超过upvalue的数量时返回NULL（且不出栈任何东西）。 
*/
LUA_API const char *lua_setupvalue (lua_State *L, int funcindex, int n) {
  const char *name;
  TValue *val;
  StkId fi;
  lua_lock(L);
  fi = index2adr(L, funcindex);
  api_checknelems(L, 1);
  name = aux_upvalue(fi, n, &val);
  if (name) {
    L->top--;
    setobj(L, val, L->top);
    luaC_barrier(L, clvalue(fi), L->top);
  }
  lua_unlock(L);
  return name;
}

